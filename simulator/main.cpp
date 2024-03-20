#include <spdlog/spdlog.h>

#include <filesystem>  // for std::filesystem
#include <iostream>
#include <random>
#include <string>

#include "batch_test.h"
#include "threadpool.h"

using namespace simulator;

std::string generateRunID() {
    int runID = 0;
    std::string outputDir =
        simulator::OUTPUT_DIR_PREFIX + std::to_string(runID);
    /* First, check whether the outputDir already exists. */
    while (std::filesystem::exists(outputDir)) {
        /* Find the last underscore in the outputDir's name so that we can
         * insert the new runID just after it. */
        auto const lastUnderscorePos = outputDir.find_last_of('_');
        if (lastUnderscorePos != std::string::npos) {
            /* If there is an underscore, try to parse the run ID from the
             * string that follows it. */
            try {
                runID = std::stoi(outputDir.substr(lastUnderscorePos + 1));
            } catch (std::exception const& e) {
                // Unable to parse previous Run ID. Set as default.
                runID = 1;
            }
            /* Increment the run ID. */
            ++runID;
            /* Remove the old run ID from the string. */
            outputDir.erase(lastUnderscorePos + 1);
        }
        /* Add the new run ID to the string. */
        outputDir += std::to_string(runID);
    }
    return std::to_string(runID);
}

void debugRun(const std::string& outputDir) {
    // double runningTime = instTimeout[getNurseNum( instIndex )];
    // int randSeed = 3434;
    // double runningTime = instTimeout[getNurseNum( instIndex )];
    // int randSeed = static_cast<int>(time( NULL ));
    std::string instanceName = "n100w4";
    int initHis = 1;
    std::vector<int> weekdata = {4, 4, 2, 8};
    std::vector<unsigned int> seeds;
    for (int i = 0; i < weekdata.size(); i++) {
        seeds.push_back(std::random_device()());
    }

    double timeout = 35;
    int runCount = 1;

    SingleCase singleCase =
        SingleCase(instanceName, initHis, weekdata, seeds, timeout, runCount);

    testSingleCase(singleCase, outputDir);
}

void benchmarkRun(const std::string& outputDir) {
    std::vector<simulator::SingleCase> testCases =
        loadInstSeq("config/seq.txt");
    if (testCases.empty()) {
        spdlog::error("No test cases loaded.");
        exit(1);
    }
    testAllInstancesWithPreloadedInstSeq(testCases, outputDir);
}

int main(int argc, char* argv[]) {
    // Increment the run ID.
    const std::string runID = generateRunID();
    std::string outputDir = OUTPUT_DIR_PREFIX + runID;
    makeSureDirExist(outputDir);

    auto logger = setting_logger(outputDir + "/run.log", true);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Output directory: {}", outputDir);
    spdlog::info("Run ID: {}", runID);

    loadConfig();
    loadInstTimeOut();

    debugRun(outputDir);
    //benchmarkRun(outputDir);
}