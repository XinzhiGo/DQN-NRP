#include "batch_test.h"

#include <spdlog/async.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <random>

#include "INRC2.h"
#include "threadpool.h"

std::string simulator::loadConfig(const std::string &filename) {
    std::string configString;
    std::ifstream ifs(filename);
    std::string s;

    configString = std::string();
    while (ifs >> s) {
        configString += s;
    }

    ifs.close();
    return configString;
}

std::map<int, double> simulator::loadInstTimeOut(const std::string &filename) {
    std::map<int, double> instTimeout;
    std::ifstream timoutFile(filename);

    int nurseNum;
    double runningTime;
    while (timoutFile >> nurseNum >> runningTime) {
        instTimeout[nurseNum] = runningTime;
    }

    timoutFile.close();
    return instTimeout;
}

std::vector<simulator::SingleCase> simulator::loadInstSeq(
    const std::string &filename) {
    std::ifstream file(filename);
    std::vector<SingleCase> testCases;
    if (!file.is_open()) {
        spdlog::error("Error: Unable to open file!");
        return testCases;
    }

    std::string line;
    // skip the first line
    std::getline(file, line);

    // Read each line of the file
    while (std::getline(file, line)) {
        std::string instanceName;
        int initHis;
        std::vector<int> weekData;
        std::vector<unsigned int> seeds;
        double timeout;
        int runCount;

        std::string caseStr;
        // Parse the line using comma as delimiter
        std::stringstream ss(line);
        std::getline(ss, caseStr, ',');
        ss >> timeout;
        ss.ignore();  // Ignore the comma after timeout
        ss >> runCount;

        // parse caseStr
        std::stringstream ss1(caseStr);
        std::string token;
        char his;
        std::string seq;

        // Get the instance name
        getline(ss1, token, '_');
        instanceName = token;

        // Get the initial history character
        getline(ss1, token, '_');
        initHis = token[0] - '0';

        // Get the week data sequence
        getline(ss1, seq);
        // seq = "6-7-5-3"
        // split seq by - and convert it to int
        std::stringstream ss2(seq);
        while (getline(ss2, token, '-')) {
            weekData.push_back(std::stoi(token));
        }

        // TODO seeds
        int weekNum = weekData.size();
        for (int i = 0; i < weekNum; ++i) {
            seeds.push_back(std::random_device()());
        }
        SingleCase singleCase(instanceName, initHis, weekData, seeds, timeout,
                              runCount);
        testCases.push_back(singleCase);
    }
    // Close the file
    file.close();

    return testCases;
}

bool simulator::makeSureDirExist(const std::string &dir) {
    std::filesystem::path path(dir);
    if (std::filesystem::create_directory(path)) {
        spdlog::info("Directory {} created.", dir);
        return true;  // created successfully
    } else if (std::filesystem::exists(path)) {
        spdlog::warn("Directory {} already existed.", dir);
        return true;  // already existed
    } else {
        spdlog::error("Failed to create directory {}.", dir);
        return false;  // failed to create
    }
}

void simulator::testAllInstancesWithPreloadedInstSeq(
    std::vector<SingleCase> testCases, const std::string &outputDir) {
    // Create a thread pool with four threads.
    const int threadPoolSize = std::thread::hardware_concurrency() - 2;
    //const int threadPoolSize = 1;
    spdlog::info("Thread pool size: {}", threadPoolSize);
    ThreadPool threadPool(threadPoolSize);

    for (int caseIdx = 0; caseIdx < testCases.size(); ++caseIdx) {
        spdlog::info("Start case {}.", caseIdx);
        spdlog::info("==============================");
        SingleCase singleCase = testCases[caseIdx];
        std::string subDir =
            outputDir + "/" + "case_" + std::to_string(caseIdx);
        makeSureDirExist(subDir);

        // Submit the tasks to the thread pool and store the futures in a
        // vector.
        std::vector<std::future<void>> futures;
        for (int i = 0; i < singleCase.runCount; ++i) {
            std::string outputDir = subDir + "/" + std::to_string(i);

            SingleCase newSingleCase(singleCase);
            // prepare seeds
            newSingleCase.seeds.clear();
            size_t weekNum = newSingleCase.weekData.size();
            for (int j = 0; j < weekNum; ++j) {
                newSingleCase.seeds.push_back(std::random_device()());
            }
            auto future =
                threadPool.submit(singleRunSingleCase, newSingleCase, outputDir);
            futures.push_back(std::move(future));
        }
    }
}

void simulator::testSingleCase(SingleCase singleCase,
                               const std::string &outputDir) {
    for (int i = 0; i < singleCase.runCount; ++i) {
        singleRunSingleCase(singleCase, outputDir);
    }
}

void simulator::singleRunSingleCase(SingleCase singleCase,
                                    const std::string &outputDir) {
    makeSureDirExist(outputDir);
    const std::string instanceDir = "../Instance/";
    const std::string &instanceName = singleCase.instanceName;

    // create sub directory strings
    std::string weekstr;
    for (int j = 0; j < singleCase.weekData.size(); ++j) {
        weekstr += std::to_string(singleCase.weekData[j]);
        if (j != singleCase.weekData.size() - 1) {
            weekstr += "-";
        }
    }
    std::string subDir =
        instanceName + "_" + std::to_string(singleCase.initHis) + "_" + weekstr;
    std::string saveDir = outputDir + "/" + subDir;
    makeSureDirExist(saveDir);

    // prepare first week
    std::map<std::string, std::string> argvMaps;
    std::string sce =
        instanceDir + instanceName + "/Sc-" + instanceName + ".txt";
    std::string his = instanceDir + instanceName + "/H0-" + instanceName + '-' +
                      std::to_string(singleCase.initHis) + ".txt";
    std::string week = instanceDir + instanceName + "/WD-" + instanceName +
                       '-' + std::to_string(singleCase.weekData[0]) + ".txt";
    std::string sol = saveDir + "/sol-week0.txt";
    std::string cusOut = saveDir + "/custom-week0";

    std::map<std::string, std::string> argvMap = {
        {ARGV::ID, std::to_string(time(NULL))},
        {ARGV::SCE, sce},
        {ARGV::HIS, his},
        {ARGV::WEEK, week},
        {ARGV::SOL, sol},
        {ARGV::TIMEOUT, std::to_string(singleCase.timeout)},
        {ARGV::RAND, std::to_string(singleCase.seeds[0])},
        {ARGV::CUS_OUT, cusOut},
        {ARGV::SAVE_DIR, saveDir + "/"}};

    argvMaps = argvMap;
    saveArgvMap(argvMap, saveDir + "/argv0.txt");
    spdlog::info("=============== run week 0 ===============");
    // log argvMaps
    for (auto &kv : argvMaps) {
        spdlog::info("{}: {}", kv.first, kv.second);
    }
    INRC2::run(argvMaps);

    // prepare other weeks
    for (char w = '1'; w < instanceName[5]; ++w) {
        std::string sce =
            instanceDir + instanceName + "/Sc-" + instanceName + ".txt";
        std::string week = instanceDir + instanceName + "/WD-" + instanceName +
                           '-' + std::to_string(singleCase.weekData[w - '0']) +
                           ".txt";
        std::string sol = saveDir + "/sol-week" + w + ".txt";
        std::string cusOut = saveDir + "/custom-week" + w;
        std::string cusIn = saveDir + "/custom-week" + char(w - 1);

        std::map<std::string, std::string> argvMap{
            {ARGV::ID, std::to_string(time(NULL))},
            {ARGV::SCE, sce},
            {ARGV::WEEK, week},
            {ARGV::SOL, sol},
            {ARGV::TIMEOUT, std::to_string(singleCase.timeout)},
            {ARGV::RAND, std::to_string(singleCase.seeds[w - '0'])},
            {ARGV::CUS_OUT, cusOut},
            {ARGV::CUS_IN, cusIn},
            {ARGV::SAVE_DIR, saveDir + "/"}};
        argvMaps = argvMap;
        saveArgvMap(argvMap, saveDir + "/argv" + w + ".txt");
        spdlog::info("=============== run week {} ===============", w);
        // log argvMaps
        for (auto &kv : argvMaps) {
            spdlog::info("{}: {}", kv.first, kv.second);
        }
        INRC2::run(argvMaps);
    }
    spdlog::info("Call validator for {}", saveDir);
    callValidator(saveDir, false);
}

bool simulator::callValidator(const std::string &outputDir, bool verbose) {
    std::string cmd = VALIDATOR_PATH + " " + outputDir + " -y";
    if (verbose) {
        cmd += " -v";
    }
    int ret = system(cmd.c_str());
    return ret == 0;
}

void simulator::saveArgvMap(const std::map<std::string, std::string> argvMap,
                            const std::string &filename) {
    std::ofstream fout(filename);
    for (auto &kv : argvMap) {
        fout << kv.first << ": " << kv.second << std::endl;
    }
    fout.close();
}

std::shared_ptr<spdlog::logger> simulator::setting_logger(
    const std::string &log_file_path, bool use_console) {
    const std::string pattern =
        "[thread %t] [%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v";
    std::vector<spdlog::sink_ptr> sinks;
    // Create a console logger that outputs to stdout
    if (use_console) {
        auto console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern(pattern);
        sinks.push_back(console_sink);
    }

    // Create a file logger that outputs to the specified log file
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        log_file_path, true);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern(pattern);
    sinks.push_back(file_sink);

    // create default thread pool
    spdlog::init_thread_pool(8192, 1);

    // Create a thread safe logger logger that uses both the console and
    // file sinks
    auto m_logger = std::make_shared<spdlog::async_logger>(
        "multi_sink", sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    m_logger->set_level(spdlog::level::trace);

    // Check that the file logger was created successfully
    if (file_sink->filename() != log_file_path) {
        // The file logger was not created successfully
        std::string message = "The file logger was not created successfully.";
        throw std::system_error(1, std::generic_category(), message);
    }

    return m_logger;
}
