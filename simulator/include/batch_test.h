#ifndef BATCH_TEST_H_
#define BATCH_TEST_H_

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <map>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace simulator {

static const int INIT_HIS_NUM = 3;
static const int WEEKDATA_NUM = 10;
static const int MAX_WEEK_NUM = 8;
static const int WEEKDATA_SEQ_SIZE = (MAX_WEEK_NUM + 1);

const std::string OUTPUT_DIR_PREFIX = "output_";

const std::vector<std::string> INSTANCES = {
    "n005w4", "n012w8", "n021w4",            // 0 1 2
    "n030w4", "n030w8",                      // 3 4
    "n035w4", "n035w8", "n040w4", "n040w8",  // 5 6
    "n050w4", "n050w8",                      // 7 8
    "n060w4", "n060w8",                      // 9 10
    "n070w4", "n070w8", "n080w4", "n080w8",  // 11 12
    "n100w4", "n100w8",                      // 13 14
    "n110w4", "n110w8", "n120w4", "n120w8"   // 15 16
};

const std::string VALIDATOR_PATH = "../Instance/validator.sh";

class ARGV final {
   public:
    static inline const std::string ID = "id";
    static inline const std::string CONFIG = "config";
    static inline const std::string SCE = "sce";
    static inline const std::string HIS = "his";
    static inline const std::string WEEK = "week";
    static inline const std::string SOL = "sol";
    static inline const std::string TIMEOUT = "timeout";
    static inline const std::string RAND = "rand";
    static inline const std::string CUS_IN = "cusIn";
    static inline const std::string CUS_OUT = "cusOut";
    static inline const std::string SAVE_DIR = "saveDir";
};

class SingleCase {
   public:
    SingleCase() = default;
    SingleCase(const std::string &instanceName, int initHis,
               std::vector<int> weekData, std::vector<unsigned int> seeds,
               double timeout, int runCount)
        : instanceName(instanceName),
          initHis(initHis),
          weekData(weekData),
          seeds(seeds),
          timeout(timeout),
          runCount(runCount) {}

    std::string instanceName;
    int initHis;
    std::vector<int> weekData;
    std::vector<unsigned int> seeds;

    double timeout;  // in seconds
    int runCount;

};

// load config from configFile
std::string loadConfig(const std::string &filename = "config.txt");
// load timeout from timoutFile
std::map<int, double> loadInstTimeOut(
    const std::string &filename = "timeout.txt");
// load instance sequence from instSeqFile
std::vector<SingleCase> loadInstSeq(const std::string &filename = "seq.txt");

bool makeSureDirExist(const std::string &dir);

void testAllInstancesWithPreloadedInstSeq(std::vector<SingleCase> testCases,
                                          const std::string &outputDir);
void testSingleCase(SingleCase singleCase, const std::string &outputDir);
void singleRunSingleCase(SingleCase singleCase, const std::string &outputDir);

bool callValidator(const std::string &outputDir, bool verbose = false);
void saveArgvMap(const std::map<std::string, std::string> argvMap,
                 const std::string &filename);
std::shared_ptr<spdlog::logger> setting_logger(
    const std::string &log_file_path = "run.log", bool use_console = true);

}  // namespace simulator

#endif