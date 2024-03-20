#include "INRC2.h"

using namespace std;
using namespace INRC2;

int main(int argc, char *argv[]) {
    std::map<std::string, std::string> argvMap;
    // handle command line arguments
    int j;
    for (int i = 1; i < argc; i = j) {
        if (argv[i][0] == '-') {
            argvMap[argv[i] + 2];
        }
        for (j = i + 1; (j < argc) && (argv[j][0] != '-'); ++j) {
            string argValue = argv[j];

            // Remove the quotes around the argument value, if present
            if (argValue.front() == '"' && argValue.back() == '"') {
                argValue = argValue.substr(1, argValue.length() - 2);
            }

            // (argv[i] + 2) to skip "--" before argument
            argvMap[argv[i] + 2] += argValue;
        }
    }

    return run(argvMap);
}