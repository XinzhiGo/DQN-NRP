#include "Solution.h"
#include <spdlog/spdlog.h>

#include "Solver.h"

using namespace std;

const NurseRostering::Solution::TryMoveTable NurseRostering::Solution::tryMove =
    {&NurseRostering::Solution::tryAddAssign,
     &NurseRostering::Solution::tryRemoveAssign,
     &NurseRostering::Solution::tryChangeAssign,
     &NurseRostering::Solution::tryExchangeDay,
     &NurseRostering::Solution::trySwapNurse,
     &NurseRostering::Solution::trySwapBlock};
const NurseRostering::Solution::FindBestMoveTable
    NurseRostering::Solution::findBestMove = {
        &NurseRostering::Solution::findBestAdd,
        &NurseRostering::Solution::findBestRemove,
        &NurseRostering::Solution::findBestChange,
        &NurseRostering::Solution::findBestExchange,
        &NurseRostering::Solution::findBestSwap,
#if INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_ORGN
        &NurseRostering::Solution::findBestBlockSwap,
#elif INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_CACHED
        &NurseRostering::Solution::findBestBlockSwap_cached,
#elif INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_FAST
        &NurseRostering::Solution::findBestBlockSwap_fast,
#elif INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_PART
        &NurseRostering::Solution::findBestBlockSwap_part,
#elif INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_RAND
        &NurseRostering::Solution::findBestBlockSwap_rand,
#endif
        &NurseRostering::Solution::findBestBlockShift,
        &NurseRostering::Solution::findBestARLoop,
        &NurseRostering::Solution::findBestARRand,
        &NurseRostering::Solution::findBestARBoth};
const NurseRostering::Solution::FindBestMoveTable
    NurseRostering::Solution::findBestMoveOnBlockBorder = {
        &NurseRostering::Solution::findBestAddOnBlockBorder,
        &NurseRostering::Solution::findBestRemoveOnBlockBorder,
        &NurseRostering::Solution::findBestChangeOnBlockBorder,
        &NurseRostering::Solution::findBestExchangeOnBlockBorder,
        &NurseRostering::Solution::findBestSwapOnBlockBorder,
#if INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_ORGN
        &NurseRostering::Solution::findBestBlockSwap,
#elif INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_CACHED
        &NurseRostering::Solution::findBestBlockSwap_cached,
#elif INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_FAST
        &NurseRostering::Solution::findBestBlockSwap_fast,
#elif INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_PART
        &NurseRostering::Solution::findBestBlockSwap_part,
#elif INRC2_BLOCK_SWAP_FIND_BEST == INRC2_BLOCK_SWAP_RAND
        &NurseRostering::Solution::findBestBlockSwap_rand,
#endif
        &NurseRostering::Solution::findBestBlockShift,
        &NurseRostering::Solution::findBestARLoopOnBlockBorder,
        &NurseRostering::Solution::findBestARRandOnBlockBorder,
        &NurseRostering::Solution::findBestARBothOnBlockBorder};
const NurseRostering::Solution::ApplyMoveTable
    NurseRostering::Solution::applyMove = {
        &NurseRostering::Solution::addAssign,
        &NurseRostering::Solution::removeAssign,
        &NurseRostering::Solution::changeAssign,
        &NurseRostering::Solution::exchangeDay,
        &NurseRostering::Solution::swapNurse,
        &NurseRostering::Solution::swapBlock};
#ifdef INRC2_USE_TABU
const NurseRostering::Solution::UpdateTabuTable
    NurseRostering::Solution::updateTabu = {
        &NurseRostering::Solution::updateAddTabu,
        &NurseRostering::Solution::updateRemoveTabu,
        &NurseRostering::Solution::updateChangeTabu,
        &NurseRostering::Solution::updateExchangeTabu,
        &NurseRostering::Solution::updateSwapTabu,
        &NurseRostering::Solution::updateBlockSwapTabu};
#endif

const vector<string> NurseRostering::Solution::modeSeqNames = {
    "[ARlCS]",   "[ARrCS]",   "[ARbCS]",   "[ACSR]",   "[ARlSCB]", "[ARrSCB]",
    "[ARbSCB]",  "[ASCBR]",   "[ARlCSE]",  "[ARrCSE]", "[ARbCSE]", "[ACSER]",
    "[ARlCSEB]", "[ARrCSEB]", "[ARbCSEB]", "[ACSEBR]", "[ARlCB]",  "[ARrCB]",
    "[ARbCB]",   "[ACBR]",    "[ARlCEB]",  "[ARrCEB]", "[ARbCEB]", "[ACEBR]"};
const vector<vector<int> > NurseRostering::Solution::modeSeqPatterns = {
    {Solution::Move::Mode::Add, Solution::Move::Mode::Remove, Solution::Move::Mode::ARBoth,
     Solution::Move::Mode::Change,Solution::Move::Mode::Swap,Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::ARRand, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap},
    {Solution::Move::Mode::ARBoth, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap},
    {Solution::Move::Mode::Add, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::Remove},

    {Solution::Move::Mode::ARLoop, Solution::Move::Mode::Swap,
     Solution::Move::Mode::Change, Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::ARRand, Solution::Move::Mode::Swap,
     Solution::Move::Mode::Change, Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::ARBoth, Solution::Move::Mode::Swap,
     Solution::Move::Mode::Change, Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::Add, Solution::Move::Mode::Swap,
     Solution::Move::Mode::Change, Solution::Move::Mode::BlockSwap,
     Solution::Move::Mode::Remove},

    {Solution::Move::Mode::ARLoop, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::BlockShift},
    {Solution::Move::Mode::ARRand, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::BlockShift},
    {Solution::Move::Mode::ARBoth, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::BlockShift},
    {Solution::Move::Mode::Add, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::BlockShift,
     Solution::Move::Mode::Remove},

    {Solution::Move::Mode::ARLoop, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::BlockShift,
     Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::ARRand, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::BlockShift,
     Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::ARBoth, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::BlockShift,
     Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::Add, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::BlockShift,
     Solution::Move::Mode::BlockSwap, Solution::Move::Mode::Remove},

    {Solution::Move::Mode::ARLoop, Solution::Move::Mode::Change,
     Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::ARRand, Solution::Move::Mode::Change,
     Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::ARBoth, Solution::Move::Mode::Change,
     Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::Add, Solution::Move::Mode::Change,
     Solution::Move::Mode::Swap, Solution::Move::Mode::Remove},

    {Solution::Move::Mode::ARLoop, Solution::Move::Mode::Change,
     Solution::Move::Mode::BlockShift, Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::ARRand, Solution::Move::Mode::Change,
     Solution::Move::Mode::BlockShift, Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::ARBoth, Solution::Move::Mode::Change,
     Solution::Move::Mode::BlockShift, Solution::Move::Mode::BlockSwap},
    {Solution::Move::Mode::Add, Solution::Move::Mode::Change,
     Solution::Move::Mode::BlockShift, Solution::Move::Mode::BlockSwap,
     Solution::Move::Mode::Remove}};

const double NurseRostering::Solution::NO_DIFF = -1;

NurseRostering::Solution::Solution(const TabuSolver &s)
    : solver(s), problem(s.problem), iterCount(1) {}

NurseRostering::Solution::Solution(const TabuSolver &s, const Output &output)
    : solver(s), problem(s.problem), iterCount(1) {
    rebuild(output);
}

void NurseRostering::Solution::rebuild(const Output &output, double diff) {
    if (diff < 1) {  // greater than 1 means totally change
        unsigned selectBound = static_cast<unsigned>(
            diff * (solver.randGen.max() - solver.randGen.min()) +
            solver.randGen.min());

        const AssignTable &assignTable(
            (&output.getAssignTable() != &assign)
                ? output.getAssignTable()
                : AssignTable(output.getAssignTable()));

        resetAssign();
        resetAssistData();

        for (NurseID nurse = 0; nurse < problem.scenario.nurseNum; ++nurse) {
            for (int weekday = Weekday::Mon; weekday <= Weekday::Sun;
                 ++weekday) {
                if (assignTable[nurse][weekday].isWorking()) {
                    if (solver.randGen() >= selectBound) {
                        addAssign(weekday, nurse, assignTable[nurse][weekday]);
                    }
                }
            }
        }
    }

    if (diff > 0) {  // there may be difference
        repair(solver.timer);
    }

    evaluateObjValue();
}

void NurseRostering::Solution::rebuild(const Output &output) {
    const AssignTable &assignTable((&output.getAssignTable() != &assign)
                                       ? output.getAssignTable()
                                       : AssignTable(output.getAssignTable()));

    resetAssign();
    resetAssistData();

    for (NurseID nurse = 0; nurse < problem.scenario.nurseNum; ++nurse) {
        for (int weekday = Weekday::Mon; weekday <= Weekday::Sun; ++weekday) {
            if (assignTable[nurse][weekday].isWorking()) {
                addAssign(weekday, nurse, assignTable[nurse][weekday]);
            }
        }
    }

    objValue = output.getObjValue();
}

void NurseRostering::Solution::rebuild() {
    rebuild(*this);
    evaluateObjValue();
}

bool NurseRostering::Solution::genInitAssign(int greedyRetryCount) {
    bool feasible;
    Timer::Duration timeForBranchAndCut = solver.timer.restTime() * 3 / 4;
    do {
        feasible = genInitAssign_Greedy();
        //feasible = genInitAssign_Dynamic();
        //if (!feasible) {
        //    Timer timer((solver.timer.restTime() - timeForBranchAndCut) /
        //                greedyRetryCount);
        //    feasible = repair(timer);
        //}
    } while (!feasible && (--greedyRetryCount > 0));

    return (feasible || genInitAssign_BranchAndCut());
}


bool NurseRostering::Solution::genInitAssign_Greedy() {
    resetAssign();
    resetAssistData();

    AvailableNurses availableNurse(*this);
    const NurseNumOfSkill &nurseNumOfSkill(solver.getNurseNumOfSkill());

    for (int weekday = Weekday::Mon; weekday <= Weekday::Sun; ++weekday) {
        // decide assign sequence of skill
        // the greater requiredNurseNum/nurseNumOfSkill[skill] is, the smaller
        // index in skillRank a skill will get
        vector<SkillID> skillRank(problem.scenario.skillTypeNum);
        vector<double> dailyRequire(problem.scenario.skillSize, 0);
        for (int rank = 0; rank < problem.scenario.skillTypeNum; ++rank) {
            SkillID skill = rank + NurseRostering::Scenario::Skill::ID_BEGIN;
            skillRank[rank] = skill;
            for (ShiftID shift = NurseRostering::Scenario::Shift::ID_BEGIN;
                 shift < problem.scenario.shiftSize; ++shift) {
                dailyRequire[skill] +=
                    problem.weekData.minNurseNums[weekday][shift][skill];
            }
            dailyRequire[skill] /= nurseNumOfSkill[skill];
        }

        class CmpDailyRequire {
           public:
            CmpDailyRequire(vector<double> &dr) : dailyRequire(dr) {}

            bool operator()(const int &l, const int &r) {
                return (dailyRequire[l] > dailyRequire[r]);
            }

           private:
            const vector<double> &dailyRequire;

           private:  // forbidden operators
            CmpDailyRequire &operator=(const CmpDailyRequire &) {
                return *this;
            }
        };
        sort(skillRank.begin(), skillRank.end(), CmpDailyRequire(dailyRequire));

        // start assigning nurses
        for (int rank = 0; rank < problem.scenario.skillTypeNum; ++rank) {
            SkillID skill = skillRank[rank];
            availableNurse.setEnvironment(weekday, skill);
            for (ShiftID shift = NurseRostering::Scenario::Shift::ID_BEGIN;
                 shift < problem.scenario.shiftSize; ++shift) {
                availableNurse.setShift(shift);
                for (int i = 0;
                     i < problem.weekData.minNurseNums[weekday][shift][skill];
                     ++i) {
                    int nurse = availableNurse.getNurse();
                    if (nurse != NurseRostering::Scenario::Nurse::ID_NONE) {
                        addAssign(weekday, nurse, Assign(shift, skill));
                    } else {
                        return false;
                    }
                }
            }
        }
    }

    evaluateObjValue();
    return true;
}

bool NurseRostering::Solution::genInitAssign_Dynamic() {
    resetAssign();
    resetAssistData();

    History history(problem.history);
    Scenario scenario(problem.scenario);

    const int skillTypeNum = problem.scenario.skillTypeNum;
    const int skillSize = problem.scenario.skillSize;
    const int shiftSize = problem.scenario.shiftSize;
    const int nurseNum = problem.scenario.nurseNum;

    const NurseNumOfSkill &nurseNumOfSkill(solver.getNurseNumOfSkill());

    for (int weekday = Weekday::Mon; weekday <= Weekday::Sun; ++weekday) {
        vector<bool> flag(nurseNum, false);
        vector<vector<int>> needShiftNurse(shiftSize+2);
        vector<int> finalSelect;

        for(int nurse = 0; nurse < nurseNum; ++nurse) {
            if (history.totalWorkingWeekendNums[nurse] >=
                    scenario.contracts[scenario.nurses[nurse].contract]
                        .maxWorkingWeekendNum &&
                weekday >= Weekday::Sat) {
                finalSelect.push_back(nurse);
                continue;
            }
            int lastShift = history.lastShifts[nurse];
            if(lastShift == 0){ //如果之前没有排班
                int consecutiveDayOffNum = history.consecutiveDayoffNums[nurse];
                //获取当前护士合约的要求，连续工作天数
                int minNum = scenario.contracts[scenario.nurses[nurse].contract].minConsecutiveDayoffNum;
                int maxNum = scenario.contracts[scenario.nurses[nurse].contract].maxConsecutiveDayoffNum;
                if(consecutiveDayOffNum >= minNum && consecutiveDayOffNum < maxNum){
                    needShiftNurse[shiftSize].push_back(nurse);
                }else if(consecutiveDayOffNum >= maxNum){
                    needShiftNurse[0].push_back(nurse);
                }else if(consecutiveDayOffNum < minNum){
                    finalSelect.push_back(nurse);
                }
            }else{ //如果之前排班了
                int consecutiveShiftNum = history.consecutiveShiftNums[nurse];
                int consecutiveDayNum = history.consecutiveDayNums[nurse];
                //获取当前护士合约的要求，连续工作天数
                int minShiftNum = scenario.shifts[lastShift].minConsecutiveShiftNum;
                int maxShiftNum = scenario.shifts[lastShift].maxConsecutiveShiftNum;
                int minDayNum = scenario.contracts[scenario.nurses[nurse].contract].minConsecutiveDayNum;
                int maxDayNum = scenario.contracts[scenario.nurses[nurse].contract].maxConsecutiveDayNum;
                if(consecutiveShiftNum >= minShiftNum && consecutiveShiftNum < maxShiftNum && consecutiveDayNum >= minDayNum && consecutiveDayNum < maxDayNum){
                    //像这种逻辑，只是有的不可以安排而已，可以把validsuccession的逻辑写到选护士的那里
                    if(scenario.shifts[lastShift].illegalNextShiftNum == 0) needShiftNurse[shiftSize].push_back(nurse);
                    else {
                        int maxLastday = maxDayNum - consecutiveDayNum;
                        for (int sh = 1; sh < shiftSize; sh++) {
                            if (maxLastday >= scenario.shifts[sh].minConsecutiveShiftNum) { //todo 这里还可以改进
                                needShiftNurse[sh].push_back(nurse);
                            }
                        }
                    }
                    //todo 需要考虑一下maxlastday的问题
                    //needShiftNurse[shiftSize].push_back(nurse);
                }else if(consecutiveShiftNum >= minShiftNum && consecutiveShiftNum < maxShiftNum && consecutiveDayNum < minDayNum){
                    int maxLastday = maxDayNum - consecutiveDayNum;
                    for (int sh = 1; sh < shiftSize; sh++) {
                        if (maxLastday >= scenario.shifts[sh].minConsecutiveShiftNum) { //todo 这里还可以改进
                            needShiftNurse[sh].push_back(nurse);
                        }
                    }
                }else if(consecutiveShiftNum >= minShiftNum && consecutiveShiftNum < maxShiftNum && consecutiveDayNum >= maxDayNum) {
                    finalSelect.push_back(nurse);
                }else if(consecutiveShiftNum >= maxShiftNum && consecutiveDayNum >= minDayNum && consecutiveDayNum < maxDayNum) { //其实这里也有bug，可能剩下的没那么多了
                    //可以判断一下剩下多少了，如果剩下的天数不足满足最小连续工作天数就不要安排了
                    //这样的话一个护士在多个集合里面
                    //todo 看看是不是需要换一个逻辑
                    int maxLastday = maxDayNum - consecutiveDayNum;
                    for(int shift = 1; shift < shiftSize; ++shift){
                        if(shift != lastShift && maxLastday >= scenario.shifts[shift].minConsecutiveShiftNum){
                            needShiftNurse[shift].push_back(nurse);
                        }
                    }
                    needShiftNurse[shiftSize+1].push_back(nurse);
                }else if(consecutiveShiftNum >= maxShiftNum && consecutiveDayNum >= maxDayNum){
                    finalSelect.push_back(nurse);
                }else if(consecutiveShiftNum < minShiftNum && consecutiveDayNum < maxDayNum) {
                    needShiftNurse[lastShift].push_back(nurse);
                }else if(consecutiveShiftNum < minShiftNum && consecutiveDayNum >= maxDayNum){ //连续工作天数更重要，因为惩罚更大
                    finalSelect.push_back(nurse);
                }

            }

        }



        // decide assign sequence of skill
        // the greater requiredNurseNum/nurseNumOfSkill[skill] is, the smaller
        // index in skillRank a skill will get
        vector<SkillID> skillRank(problem.scenario.skillTypeNum);
        vector<double> dailyRequire(problem.scenario.skillSize, 0);
        for (int rank = 0; rank < problem.scenario.skillTypeNum; ++rank) {
            SkillID skill = rank + NurseRostering::Scenario::Skill::ID_BEGIN;
            skillRank[rank] = skill;
            for (ShiftID shift = NurseRostering::Scenario::Shift::ID_BEGIN;
                 shift < problem.scenario.shiftSize; ++shift) {
                dailyRequire[skill] +=
                    problem.weekData.minNurseNums[weekday][shift][skill];
            }
            dailyRequire[skill] /= nurseNumOfSkill[skill];
        }

        class CmpDailyRequire {
           public:
            CmpDailyRequire(vector<double> &dr) : dailyRequire(dr) {}

            bool operator()(const int &l, const int &r) {
                return (dailyRequire[l] > dailyRequire[r]);
            }

           private:
            const vector<double> &dailyRequire;

           private:  // forbidden operators
            CmpDailyRequire &operator=(const CmpDailyRequire &) {
                return *this;
            }
        };
        sort(skillRank.begin(), skillRank.end(), CmpDailyRequire(dailyRequire));
        using Shift = NurseRostering::Scenario::Shift;
        using Skill = NurseRostering::Scenario::Skill;
        // start assigning nurses
        for (int rank = 0; rank < problem.scenario.skillTypeNum; ++rank) {
            SkillID skill = skillRank[rank];

            for (ShiftID shift = NurseRostering::Scenario::Shift::ID_BEGIN;
                 shift < problem.scenario.shiftSize; ++shift) {
                for (int i = 0;
                     i < problem.weekData.minNurseNums[weekday][shift][skill];
                     ++i) {
                    int nurse = selectNurse(weekday,needShiftNurse,finalSelect, shift,skill,flag);
                    if (nurse != -1) {
                        addAssign(weekday, nurse, Assign(shift, skill));
                        //要对数据结构进行更改
                        if(history.lastShifts[nurse] == Shift::ID_NONE){
                            history.lastShifts[nurse] = shift;
                            history.consecutiveShiftNums[nurse] = 1;
                            history.consecutiveDayNums[nurse] = 1;
                            history.consecutiveDayoffNums[nurse] = 0;
                        }else if(history.lastShifts[nurse] == shift){
                            history.lastShifts[nurse] = shift;
                            history.consecutiveShiftNums[nurse] += 1;
                            history.consecutiveDayNums[nurse] += 1;
                            history.consecutiveDayoffNums[nurse] = 0;
                        }else{
                            history.lastShifts[nurse] = shift;
                            history.consecutiveShiftNums[nurse] = 1;
                            history.consecutiveDayNums[nurse] += 1;
                            history.consecutiveDayoffNums[nurse] = 0;
                        }
                        flag[nurse] = true;
                    } else {
                        return false;
                    }
                }
            }
        }
    }

    evaluateObjValue();
    return true;
}

int NurseRostering::Solution::selectNurse(int weekday,vector<vector<int>> nurseVector,vector<int> finalSelect,int shift,int skill,vector<bool> flag){
    vector<int> availableNurse;
    for(int i = 0; i < nurseVector[shift].size(); i++){
        int nurse = nurseVector[shift][i];
        if(flag[nurse] == false && isValidSuccession(nurse,shift,weekday) && problem.scenario.nurses[nurse].skills[skill] == true){
            availableNurse.push_back(nurse);
        }
    }
    if(availableNurse.size() != 0) {
        int num = availableNurse.size();
        int select = solver.randGen() % num;
        return availableNurse[select];
    }
    for(int i = 0; i < nurseVector[0].size();i++){
        int nurse = nurseVector[0][i];
        if(flag[nurse] == false && isValidSuccession(nurse,shift,weekday) && problem.scenario.nurses[nurse].skills[skill] == true){
            availableNurse.push_back(nurse);
        }
    }
    if(availableNurse.size() != 0) {
        int num = availableNurse.size();
        int select = solver.randGen() % num;
        return availableNurse[select];
    }
    for(int i = 0; i < nurseVector[nurseVector.size()-1].size();i++){
        int nurse = nurseVector[nurseVector.size()-1][i];
        if(flag[nurse] == false && isValidSuccession(nurse,shift,weekday) && problem.scenario.nurses[nurseVector[nurseVector.size()-1][i]].skills[skill] == true){
            availableNurse.push_back(nurse);
        }
    }
    if(availableNurse.size() != 0) {
        int num = availableNurse.size();
        int select = solver.randGen() % num;
        return availableNurse[select];
    }

    for(int i = 0; i < finalSelect.size();i++){
        int nurse = finalSelect[i];
        if(flag[nurse] == false  && isValidSuccession(nurse,shift,weekday) && problem.scenario.nurses[nurse].skills[skill] == true){
            availableNurse.push_back(nurse);
        }
    }
    if(availableNurse.size() != 0) {
        int num = availableNurse.size();
        int select = solver.randGen() % num;
        return availableNurse[select];
    }

    return -1;

}

bool NurseRostering::Solution::genInitAssign_BranchAndCut() {
    resetAssign();

    bool feasible =
        fillAssign(Weekday::Mon, NurseRostering::Scenario::Shift::ID_BEGIN,
                   NurseRostering::Scenario::Skill::ID_BEGIN, 0, 0);

    rebuild();

    return feasible;
}

bool NurseRostering::Solution::fillAssign(int weekday, ShiftID shift,
                                          SkillID skill, NurseID nurse,
                                          int nurseNum) {
    if (nurse >= problem.scenario.nurseNum) {
        if (nurseNum < problem.weekData.minNurseNums[weekday][shift][skill]) {
            return false;
        } else {
            return fillAssign(weekday, shift, skill + 1, 0, 0);
        }
    } else if (skill >= problem.scenario.skillSize) {
        return fillAssign(weekday, shift + 1,
                          NurseRostering::Scenario::Skill::ID_BEGIN, 0, 0);
    } else if (shift >= problem.scenario.shiftSize) {
        return fillAssign(weekday + 1,
                          NurseRostering::Scenario::Shift::ID_BEGIN,
                          NurseRostering::Scenario::Skill::ID_BEGIN, 0, 0);
    } else if (weekday > Weekday::Sun) {
        return true;
    }

    if (solver.timer.isTimeOut()) {
        return false;
    }

    Assign firstAssign(shift, skill);
    Assign secondAssign;
    NurseID firstNurseNum = nurseNum + 1;
    NurseID secondNurseNum = nurseNum;
    bool isNotAssignedBefore = !assign.isWorking(nurse, weekday);

    if (isNotAssignedBefore) {
        if (problem.scenario.nurses[nurse].skills[skill] &&
            isValidSuccession(nurse, shift, weekday)) {
            if (solver.randGen() % 2) {
                swap(firstAssign, secondAssign);
                swap(firstNurseNum, secondNurseNum);
            }

            assign[nurse][weekday] = firstAssign;
            if (fillAssign(weekday, shift, skill, nurse + 1, firstNurseNum)) {
                return true;
            }
        }

        assign[nurse][weekday] = secondAssign;
    }

    if (fillAssign(weekday, shift, skill, nurse + 1, secondNurseNum)) {
        return true;
    } else if (isNotAssignedBefore) {
        assign[nurse][weekday] = Assign();
    }

    return false;
}

void NurseRostering::Solution::resetAssign() {
    assign = AssignTable(problem.scenario.nurseNum, Weekday::SIZE);
    for (NurseID nurse = 0; nurse < problem.scenario.nurseNum; ++nurse) {
        assign[nurse][Weekday::HIS] = Assign(problem.history.lastShifts[nurse]);
        assign[nurse][Weekday::NEXT_WEEK] = Assign();
    }
}

void NurseRostering::Solution::resetAssistData() {
    // incremental evaluation
    missingNurseNums = problem.weekData.optNurseNums;
    totalAssignNums = vector<int>(problem.scenario.nurseNum, 0);
    consecutives = vector<Consecutive>(problem.scenario.nurseNum);
    for (NurseID nurse = 0; nurse < problem.scenario.nurseNum; ++nurse) {
        consecutives[nurse] = Consecutive(problem.history, nurse);
    }
    // penalty weight
    nurseWeights = vector<ObjValue>(problem.scenario.nurseNum, 1);
#ifdef INRC2_USE_TABU
    // tabu table
    if (shiftTabu.empty() || dayTabu.empty()) {
        shiftTabu = ShiftTabu(
            problem.scenario.nurseNum,
            vector<vector<vector<IterCount> > >(
                Weekday::SIZE,
                vector<vector<IterCount> >(
                    problem.scenario.shiftSize,
                    vector<IterCount>(problem.scenario.skillSize, 0))));
        dayTabu = DayTabu(problem.scenario.nurseNum,
                          vector<IterCount>(Weekday::SIZE, 0));
    } else {
        iterCount += solver.ShiftTabuTenureBase() +
                     solver.ShiftTabuTenureAmp() + solver.DayTabuTenureBase() +
                     solver.DayTabuTenureAmp();
    }
#endif
    // delta cache
    if (blockSwapCache.empty()) {
        blockSwapCache = BlockSwapCache(
            problem.scenario.nurseNum,
            vector<BlockSwapCacheItem>(problem.scenario.nurseNum));
    }
    isBlockSwapCacheValid = vector<bool>(problem.scenario.nurseNum, false);
    // flags
    findBestARLoop_flag = true;
    findBestARLoopOnBlockBorder_flag = true;
    findBestBlockSwap_startNurse = problem.scenario.nurseNum;
    isPossibilitySelect = false;
    isBlockSwapSelected = false;
}

void NurseRostering::Solution::evaluateObjValue(
    bool considerSpanningConstraint) {
    //if(problem.history.currentWeek < problem.scenario.totalWeekNum - 1){
    //    considerSpanningConstraint = false;
    //}
    objInsufficientStaff = evaluateInsufficientStaff();
    objConsecutiveShift = 0;
    objConsecutiveDay = 0;
    objConsecutiveDayOff = 0;
    objPreference = 0;
    objCompleteWeekend = 0;
    objTotalAssign = 0;
    objTotalWorkingWeekend = 0;

    for (NurseID nurse = 0; nurse < problem.scenario.nurseNum; ++nurse) {
        objConsecutiveShift += evaluateConsecutiveShift(nurse);
        objConsecutiveDay += evaluateConsecutiveDay(nurse);
        objConsecutiveDayOff += evaluateConsecutiveDayOff(nurse);
        objPreference += evaluatePreference(nurse);
        objCompleteWeekend += evaluateCompleteWeekend(nurse);
        if (considerSpanningConstraint) {
            objTotalAssign += evaluateTotalAssign(nurse);
            objTotalWorkingWeekend += evaluateTotalWorkingWeekend(nurse);
        }
    }

    objValue = objInsufficientStaff + objConsecutiveShift + objConsecutiveDay +
               objConsecutiveDayOff + objPreference + objCompleteWeekend +
               objTotalAssign + objTotalWorkingWeekend;
}

NurseRostering::History NurseRostering::Solution::genHistory() const {
    const History &history(problem.history);
    History newHistory;
    newHistory.lastShifts.resize(problem.scenario.nurseNum);
    newHistory.consecutiveShiftNums.resize(problem.scenario.nurseNum, 0);
    newHistory.consecutiveDayNums.resize(problem.scenario.nurseNum, 0);
    newHistory.consecutiveDayoffNums.resize(problem.scenario.nurseNum, 0);

    newHistory.accObjValue = history.accObjValue + objValue;
    newHistory.pastWeekCount = history.currentWeek;
    newHistory.currentWeek = history.currentWeek + 1;
    newHistory.restWeekCount = history.restWeekCount - 1;
    newHistory.totalAssignNums = history.totalAssignNums;
    newHistory.totalWorkingWeekendNums = history.totalWorkingWeekendNums;

    for (NurseID nurse = 0; nurse < problem.scenario.nurseNum; ++nurse) {
        newHistory.totalAssignNums[nurse] += totalAssignNums[nurse];
        newHistory.totalWorkingWeekendNums[nurse] +=
            (assign.isWorking(nurse, Weekday::Sat) ||
             assign.isWorking(nurse, Weekday::Sun));
        newHistory.lastShifts[nurse] = assign[nurse][Weekday::Sun].shift;
        const Consecutive &c(consecutives[nurse]);
        if (assign.isWorking(nurse, Weekday::Sun)) {
            newHistory.consecutiveShiftNums[nurse] =
                c.shiftHigh[Weekday::Sun] - c.shiftLow[Weekday::Sun] + 1;
            newHistory.consecutiveDayNums[nurse] =
                c.dayHigh[Weekday::Sun] - c.dayLow[Weekday::Sun] + 1;
        } else {
            newHistory.consecutiveDayoffNums[nurse] =
                c.dayHigh[Weekday::Sun] - c.dayLow[Weekday::Sun] + 1;
        }
    }

    return newHistory;
}

bool NurseRostering::Solution::repair(const Timer &timer) {
#ifdef INRC2_PERFORMANCE_TEST
    clock_t startTime = clock();
    IterCount startIterCount = iterCount;
#endif
    // must not use swap for swap mode is not compatible with repair mode
    // also, the repair procedure doesn't need the technique to jump through
    // infeasible solutions
    Solution::FindBestMoveTable fbmt = {
        &NurseRostering::Solution::findBestARBoth,
        &NurseRostering::Solution::findBestChange};

    ObjValue violation = solver.checkFeasibility(assign);

    if (violation == 0) {
        return true;
    }

    // reduced tabuSearch_Rand()
    penalty.setRepairMode();
    objValue = violation;
    int modeNum = fbmt.size();

    const int minWeight = 256;   // min weight
    const int maxWeight = 1024;  // max weight (less than (RAND_MAX / modeNum))
    const int initWeight = (maxWeight + minWeight) / 2;
    const int deltaIncRatio = 8;  // = weights[mode] / weightDelta
    const int incError = deltaIncRatio - 1;
    const int deltaDecRatio = 8;  // = weights[mode] / weightDelta
    const int decError = -(deltaDecRatio - 1);
    vector<int> weights(modeNum, initWeight);
    int totalWeight = initWeight * modeNum;

    for (; !timer.isTimeOut() && (objValue > 0) &&
           (iterCount != problem.maxIterCount);
         ++iterCount) {
        int modeSelect = 0;
        for (int w = solver.randGen() % totalWeight;
             (w -= weights[modeSelect]) >= 0; ++modeSelect) {
        }

        Move bestMove;
        (this->*fbmt[modeSelect])(bestMove);

#ifdef INRC2_USE_TABU
        // update tabu list first because it requires original assignment
        (this->*updateTabu[bestMove.mode])(bestMove);
#endif
        int weightDelta;
        if (bestMove.delta < DefaultPenalty::MAX_OBJ_VALUE) {
            applyBasicMove(bestMove);

            if (bestMove.delta < 0) {  // improve current solution
                weightDelta = (incError + maxWeight - weights[modeSelect]) /
                              deltaIncRatio;
            } else {  // no improve
                weightDelta = (decError + minWeight - weights[modeSelect]) /
                              deltaDecRatio;
            }
        } else {  // invalid
            weightDelta =
                (decError + minWeight - weights[modeSelect]) / deltaDecRatio;
        }

        weights[modeSelect] += weightDelta;
        totalWeight += weightDelta;
    }
    violation = objValue;
    penalty.recoverLastMode();

    evaluateObjValue();

#ifdef INRC2_PERFORMANCE_TEST
    clock_t duration = clock() - startTime;
    cout << "[RP] iter: " << (iterCount - startIterCount) << ' '
         << "time: " << duration << ' '
         << ((violation == 0) ? "(success)" : "(fail)") << endl;
#endif
    return (violation == 0);
}

void NurseRostering::Solution::adjustWeightToBiasNurseWithGreaterPenalty(
    int inverseTotalBiasRatio, int inversePenaltyBiasRatio) {
    int biasedNurseNum = 0;
    fill(nurseWeights.begin(), nurseWeights.end(), 0);

    // select worse nurses to meet the PenaltyBiasRatio
    vector<ObjValue> nurseObj(problem.scenario.nurseNum, 0);
    for (NurseID nurse = 0; nurse < problem.scenario.nurseNum; ++nurse) {
        nurseObj[nurse] += evaluateConsecutiveShift(nurse);
        nurseObj[nurse] += evaluateConsecutiveDay(nurse);
        nurseObj[nurse] += evaluateConsecutiveDayOff(nurse);
        nurseObj[nurse] += evaluatePreference(nurse);
        nurseObj[nurse] += evaluateCompleteWeekend(nurse);
        nurseObj[nurse] += evaluateTotalAssign(nurse);
        nurseObj[nurse] += evaluateTotalWorkingWeekend(nurse);
    }

#ifndef INRC2_USE_LAMBDA_IN_SORT
    class CmpNurseObj {
       public:
        CmpNurseObj(const vector<ObjValue> &nurseObjective)
            : nurseObj(nurseObjective) {}
        // sort to (greatest ... least)
        bool operator()(const NurseID &l, const NurseID &r) {
            return (nurseObj[l] > nurseObj[r]);
        }

       private:
        const vector<ObjValue> &nurseObj;

       private:  // forbidden operators
        CmpNurseObj &operator=(const CmpNurseObj &) { return *this; }
    } cmpNurseObj(nurseObj);
#endif

    for (auto iter = problem.scenario.contracts.begin();
         iter != problem.scenario.contracts.end(); ++iter) {
        vector<NurseID> &nurses(const_cast<vector<NurseID> &>(iter->nurses));
#ifndef INRC2_USE_LAMBDA_IN_SORT
        sort(nurses.begin(), nurses.end(), cmpNurseObj);
#else
        sort(nurses.begin(), nurses.end(),
             [&nurseObj](NurseID lhs, NurseID rhs) {
                 return (nurseObj[lhs] > nurseObj[rhs]);
             });
#endif

        int num = nurses.size() / inversePenaltyBiasRatio;
        unsigned remainder = nurses.size() % inversePenaltyBiasRatio;
        biasedNurseNum += num;
        for (int i = 0; i < num; ++i) {
            nurseWeights[nurses[i]] = 1;
        }
        if (solver.randGen() % inversePenaltyBiasRatio < remainder) {
            nurseWeights[nurses[num]] = 1;
            ++biasedNurseNum;
        }
    }

    // pick nurse randomly to meet the TotalBiasRatio
    int num = problem.scenario.nurseNum / inverseTotalBiasRatio;
    while (biasedNurseNum < num) {
        NurseID nurse = solver.randGen() % problem.scenario.nurseNum;
        if (nurseWeights[nurse] == 0) {
            nurseWeights[nurse] = 1;
            ++biasedNurseNum;
        }
    }
}

void NurseRostering::Solution::swapChainSearch_DoubleHead(
    const Timer &timer, IterCount maxNoImproveChainLength) {
    if (!((optima.getObjValue() < solver.getOptima().getObjValue()) ||
          (solver.randGen() % solver.PERTURB_ORIGIN_SELECT)) ||
        (optima.getObjValue() >= DefaultPenalty::MAX_OBJ_VALUE)) {
        optima = solver.getOptima();
    }

    while (true) {
        rebuild(optima);

        RandSelect<ObjValue> rs;
        Move bestMove;
        Move bestMoveForOneNurse;
        ObjValue bestDeltaForOneNurse = DefaultPenalty::FORBIDDEN_MOVE;

        // find start links for the chain of block swap
        penalty.setBlockSwapMode();
        Move move;
        for (move.nurse = 0; move.nurse < problem.scenario.nurseNum;
             ++move.nurse) {
            for (move.nurse2 = move.nurse + 1;
                 move.nurse2 < problem.scenario.nurseNum; ++move.nurse2) {
                if (solver.haveSameSkill(move.nurse, move.nurse2)) {
                    for (move.weekday = Weekday::Mon;
                         move.weekday <= Weekday::Sun; ++move.weekday) {
                        move.delta = trySwapBlock(move.weekday, move.weekday2,
                                                  move.nurse, move.nurse2);
                        if (move.delta < DefaultPenalty::MAX_OBJ_VALUE) {
                            bool isSwap = (nurseDelta > nurse2Delta);
                            if (isSwap) {
                                swap(nurseDelta, nurse2Delta);
                            }
                            if (nurseDelta < 0) {
                                if (rs.isMinimal(move.delta, bestMove.delta,
                                                 solver.randGen)) {
                                    bestMove = move;
                                    if (isSwap) {
                                        swap(bestMove.nurse, bestMove.nurse2);
                                    }
                                }
                            } else if (nurseDelta <
                                       bestDeltaForOneNurse) {  // in case no
                                                                // swap meet
                                                                // requirement
                                                                // above
                                bestMoveForOneNurse = move;
                                bestDeltaForOneNurse = nurseDelta;
                                if (isSwap) {
                                    swap(bestMoveForOneNurse.nurse,
                                         bestMoveForOneNurse.nurse2);
                                }
                            }
                        }
                    }
                }
            }
        }
        penalty.recoverLastMode();

        if (bestMove.delta < DefaultPenalty::MAX_OBJ_VALUE) {
            if (genSwapChain(timer, bestMove, maxNoImproveChainLength)) {
                continue;
            } else {
                rebuild(optima);
            }
        }

        if (!genSwapChain(timer, bestMoveForOneNurse,
                          maxNoImproveChainLength)) {
            return;
        }
    }
}

void NurseRostering::Solution::swapChainSearch(
    const Timer &timer, IterCount maxNoImproveChainLength) {
    if (!((optima.getObjValue() < solver.getOptima().getObjValue()) ||
          (solver.randGen() % solver.PERTURB_ORIGIN_SELECT)) ||
        (optima.getObjValue() >= DefaultPenalty::MAX_OBJ_VALUE)) {
        optima = solver.getOptima();
    }

    priority_queue<Move> bestMoves;

    IterCount count = solver.MaxSwapChainRestartCount();
    while ((--count) > 0) {
        rebuild(optima);

        // find start links for the chain of block swap
        if (bestMoves.empty()) {
            penalty.setBlockSwapMode();
            Move move;
            for (move.nurse = 0; move.nurse < problem.scenario.nurseNum;
                 ++move.nurse) {
                for (move.nurse2 = move.nurse + 1;
                     move.nurse2 < problem.scenario.nurseNum; ++move.nurse2) {
                    if (solver.haveSameSkill(move.nurse, move.nurse2)) {
                        for (move.weekday = Weekday::Mon;
                             move.weekday <= Weekday::Sun; ++move.weekday) {
                            move.delta =
                                trySwapBlock(move.weekday, move.weekday2,
                                             move.nurse, move.nurse2);
                            if (move.delta < DefaultPenalty::MAX_OBJ_VALUE) {
                                bool isSwap = (nurseDelta > nurse2Delta);
                                if (isSwap) {
                                    swap(nurseDelta, nurse2Delta);
                                }
                                if (nurseDelta < 0) {
                                    if (isSwap) {
                                        swap(move.nurse, move.nurse2);
                                    }
                                    bestMoves.push(move);
                                    if (isSwap) {
                                        swap(move.nurse, move.nurse2);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            penalty.recoverLastMode();
        }

        Move head;
        if (bestMoves.empty()) {
            return;
        } else {
            head = bestMoves.top();
            bestMoves.pop();
        }
        if (genSwapChain(timer, head, maxNoImproveChainLength)) {
            count = solver.MaxSwapChainRestartCount();
            bestMoves = priority_queue<Move>();
        }
    }
}

bool NurseRostering::Solution::genSwapChain(const Timer &timer,
                                            const Move &head,
                                            IterCount noImproveLen) {
    RandSelect<ObjValue> rs;
    Move bestMove(head);
    Move move;

    // try to improve a chain of worsened nurse
    IterCount len = noImproveLen;
    for (;
         !timer.isTimeOut() && (len > 0) && (iterCount != problem.maxIterCount);
         ++iterCount) {
        swapBlock(bestMove);
        objValue += bestMove.delta;
        if (updateOptima()) {
            return true;
        }

        // try to improve the worsened nurse
        bestMove.delta = DefaultPenalty::FORBIDDEN_MOVE;
        move.nurse = bestMove.nurse2;
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
             ++move.weekday) {
            if (assign.isWorking(move.nurse, move.weekday)) {
                move.delta = tryRemoveAssign(move);
                if (rs.isMinimal(move.delta, bestMove.delta, solver.randGen)) {
                    bestMove = move;
                    bestMove.mode = Move::Mode::Remove;
                }
                for (move.assign.shift =
                         NurseRostering::Scenario::Shift::ID_BEGIN;
                     move.assign.shift < problem.scenario.shiftSize;
                     ++move.assign.shift) {
                    for (move.assign.skill =
                             NurseRostering::Scenario::Skill::ID_BEGIN;
                         move.assign.skill < problem.scenario.skillSize;
                         ++move.assign.skill) {
                        move.delta = tryChangeAssign(move);
                        if (rs.isMinimal(move.delta, bestMove.delta,
                                         solver.randGen)) {
                            bestMove = move;
                            bestMove.mode = Move::Mode::Change;
                        }
                    }
                }
            } else {
                for (move.assign.shift =
                         NurseRostering::Scenario::Shift::ID_BEGIN;
                     move.assign.shift < problem.scenario.shiftSize;
                     ++move.assign.shift) {
                    for (move.assign.skill =
                             NurseRostering::Scenario::Skill::ID_BEGIN;
                         move.assign.skill < problem.scenario.skillSize;
                         ++move.assign.skill) {
                        move.delta = tryAddAssign(move);
                        if (rs.isMinimal(move.delta, bestMove.delta,
                                         solver.randGen)) {
                            bestMove = move;
                            bestMove.mode = Move::Mode::Add;
                        }
                    }
                }
            }
        }
        penalty.setExchangeMode();
        const Consecutive &c(consecutives[move.nurse]);
        move.weekday = Weekday::Mon;
        move.weekday2 = c.dayHigh[move.weekday] + 1;
        while (move.weekday2 <= Weekday::Sun) {
            move.delta =
                tryExchangeDay(move.weekday, move.nurse, move.weekday2);
            if (rs.isMinimal(move.delta, bestMove.delta, solver.randGen)) {
                bestMove = move;
                bestMove.mode = Move::Mode::Exchange;
            }
            if (move.weekday != c.dayHigh[move.weekday]) {  // start of a block
                move.weekday = c.dayHigh[move.weekday];
                move.weekday2 = c.dayHigh[move.weekday + 1];
            } else {  // end of a block
                ++move.weekday;
                move.weekday2 = c.dayHigh[move.weekday] + 1;
            }
        }
        penalty.recoverLastMode();

        // apply add/change/remove/shift if there is improvement
#ifdef INRC2_SWAP_CHAIN_MAKE_BAD_MOVE
        if (bestMove.delta < 0) {
#else
        if (objValue + bestMove.delta < optima.getObjValue()) {
#endif
            applyBasicMove(bestMove);
            if (updateOptima()) {
                return true;
            }
        }

        // find next link
        penalty.setBlockSwapMode();
        bestMove.delta = DefaultPenalty::FORBIDDEN_MOVE;
#ifdef INRC2_SWAP_CHAIN_MAKE_BAD_MOVE
        Move bestMoveForOneNurse;
        ObjValue bestDeltaForOneNurse = DefaultPenalty::FORBIDDEN_MOVE;
#endif
        for (move.nurse2 = 0; move.nurse2 < problem.scenario.nurseNum;
             ++move.nurse2) {
            if ((move.nurse != move.nurse2) &&
                solver.haveSameSkill(move.nurse, move.nurse2)) {
                for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
                     ++move.weekday) {
                    if (!(isValidSuccession(
                              move.nurse,
                              assign[move.nurse2][move.weekday].shift,
                              move.weekday) &&
                          isValidSuccession(
                              move.nurse2,
                              assign[move.nurse][move.weekday].shift,
                              move.weekday))) {
                        continue;
                    }

                    move.delta = 0;
                    ObjValue headDelta = 0;
                    move.weekday2 = move.weekday;
                    // try each block length
                    while (
                        problem.scenario.nurses[move.nurse]
                            .skills[assign[move.nurse2][move.weekday2].skill] &&
                        problem.scenario.nurses[move.nurse2]
                            .skills[assign[move.nurse][move.weekday2].skill]) {
                        // longer blocks will also miss this skill
                        move.delta += trySwapNurse(move.weekday2, move.nurse,
                                                   move.nurse2);
                        headDelta += nurseDelta;

                        if (move.delta < DefaultPenalty::MAX_OBJ_VALUE) {
                            if (isValidPrior(
                                    move.nurse,
                                    assign[move.nurse2][move.weekday2].shift,
                                    move.weekday2) &&
                                isValidPrior(
                                    move.nurse2,
                                    assign[move.nurse][move.weekday2].shift,
                                    move.weekday2)) {
                                if ((objValue + headDelta <
                                     optima.getObjValue()) ||
                                    (objValue + move.delta <
                                     optima.getObjValue())) {
                                    if (rs.isMinimal(move.delta, bestMove.delta,
                                                     solver.randGen)) {
                                        bestMove = move;
                                    }
#ifdef INRC2_SWAP_CHAIN_MAKE_BAD_MOVE
                                } else if (
                                    headDelta <
                                    bestDeltaForOneNurse) {  // in case
                                                             // no swap
                                                             // meet
                                                             // requirement
                                                             // above
                                    bestMoveForOneNurse = move;
                                    bestDeltaForOneNurse = nurseDelta;
#endif
                                }
                            }
                        } else {  // two day off
                            move.delta -= DefaultPenalty::FORBIDDEN_MOVE;
                        }

                        if (move.weekday2 >= Weekday::Sun) {
                            break;
                        }

                        (const_cast<Solution *>(this))
                            ->swapNurse(move.weekday2, move.nurse, move.nurse2);
                        ++move.weekday2;
                    }

                    // recover original data
                    while ((--move.weekday2) >= move.weekday) {
                        (const_cast<Solution *>(this))
                            ->swapNurse(move.weekday2, move.nurse, move.nurse2);
                    }
                }
            }
        }
        penalty.recoverLastMode();

#ifdef INRC2_SWAP_CHAIN_MAKE_BAD_MOVE
        if (bestMove.delta >= DefaultPenalty::MAX_OBJ_VALUE) {
            bestMove = bestMoveForOneNurse;
        }
#endif
        if (bestMove.delta >= DefaultPenalty::MAX_OBJ_VALUE) {
            return false;
        }

        (objValue + bestMove.delta < optima.getObjValue())
            ? (len = noImproveLen)
            : (--len);
    }

    return false;
}

void NurseRostering::Solution::tabuSearch_Rand(
    const Timer &timer, const FindBestMoveTable &findBestMoveTable,
    IterCount maxNoImproveCount) {
#ifdef INRC2_PERFORMANCE_TEST
    clock_t startTime = clock();
    IterCount startIterCount = iterCount;
#endif
    optima = *this;

    int modeNum = findBestMoveTable.size();

    const int weight_Invalid = 128;  // min weight
    const int weight_NoImprove = 256;
    const int weight_ImproveCur = 1024;
    const int weight_ImproveOpt =
        4096;  // max weight (less than (RAND_MAX / modeNum))
    const int initWeight = (weight_ImproveCur + weight_NoImprove) / 2;
    const int deltaIncRatio = 8;  // = weights[mode] / weightDelta
    const int incError = deltaIncRatio - 1;
    const int deltaDecRatio = 8;  // = weights[mode] / weightDelta
    const int decError = -(deltaDecRatio - 1);
    vector<int> weights(modeNum, initWeight);
    int totalWeight = initWeight * modeNum;

    IterCount noImprove = maxNoImproveCount;
    for (; !timer.isTimeOut() && (noImprove > 0) &&
           (iterCount < problem.maxIterCount);
         ++iterCount) {
        int modeSelect = 0;
        for (int w = solver.randGen() % totalWeight;
             (w -= weights[modeSelect]) >= 0; ++modeSelect) {
        }

        Move bestMove;
        (this->*findBestMoveTable[modeSelect])(bestMove);

        int weightDelta;
        if (bestMove.delta < DefaultPenalty::MAX_OBJ_VALUE) {
#ifdef INRC2_USE_TABU
            // update tabu list first because it requires original assignment
            (this->*updateTabu[bestMove.mode])(bestMove);
#endif
            applyBasicMove(bestMove);

            if (updateOptima()) {  // improve optima
#ifdef INRC2_LS_AFTER_TSR_UPDATE_OPT
                localSearch(timer, findBestMoveTable);
#endif
                noImprove = maxNoImproveCount;
                weightDelta =
                    (incError + weight_ImproveOpt - weights[modeSelect]) /
                    deltaIncRatio;
            } else {
                --noImprove;
                if (bestMove.delta < 0) {  // improve current solution
                    weightDelta = (weights[modeSelect] < weight_ImproveCur)
                                      ? (incError + weight_ImproveCur -
                                         weights[modeSelect]) /
                                            deltaIncRatio
                                      : (decError + weight_ImproveCur -
                                         weights[modeSelect]) /
                                            deltaDecRatio;
                } else {  // no improve but valid
                    weightDelta = (weights[modeSelect] < weight_NoImprove)
                                      ? (incError + weight_NoImprove -
                                         weights[modeSelect]) /
                                            deltaIncRatio
                                      : (decError + weight_NoImprove -
                                         weights[modeSelect]) /
                                            deltaDecRatio;
                }
            }
        } else {  // invalid
            weightDelta = (decError + weight_Invalid - weights[modeSelect]) /
                          deltaDecRatio;
        }

        weights[modeSelect] += weightDelta;
        totalWeight += weightDelta;
    }
#ifdef INRC2_PERFORMANCE_TEST
    clock_t duration = clock() - startTime;
    cout << "[TS] iter: " << (iterCount - startIterCount) << ' '
         << "time: " << duration << ' ' << "speed: "
         << (iterCount - startIterCount) * static_cast<double>(CLOCKS_PER_SEC) /
                (duration + 1)
         << endl;
#endif
}

void NurseRostering::Solution::VNDSearch_Rand(
    const Timer &timer, const FindBestMoveTable &findBestMoveTable,
    IterCount maxNoImproveCount) {
#ifdef INRC2_PERFORMANCE_TEST
    clock_t startTime = clock();
    IterCount startIterCount = iterCount;
#endif
    optima = *this;

    int modeNum = findBestMoveTable.size();
    int modeSelect = 0;
    IterCount noImprove = maxNoImproveCount;
    for (; !timer.isTimeOut() && (noImprove > 0) &&
           (iterCount < problem.maxIterCount);
         ++iterCount) {
        Move bestMove;
        (this->*findBestMoveTable[modeSelect])(bestMove);

        int weightDelta;
        if (bestMove.delta < DefaultPenalty::MAX_OBJ_VALUE) {
#ifdef INRC2_USE_TABU
            // update tabu list first because it requires original assignment
            (this->*updateTabu[bestMove.mode])(bestMove);
#endif
            applyBasicMove(bestMove);

            if (updateOptima()) {  // improve optima
#ifdef INRC2_LS_AFTER_TSR_UPDATE_OPT
                localSearch(timer, findBestMoveTable);
#endif
                noImprove = maxNoImproveCount;
                modeSelect = 0;
            } else {
                --noImprove;
                if(modeSelect < modeNum - 1) {
                    ++modeSelect;
                }else{
                    modeSelect = 0;
                }
            }

            }

    }
#ifdef INRC2_PERFORMANCE_TEST
    clock_t duration = clock() - startTime;
    cout << "[TS] iter: " << (iterCount - startIterCount) << ' '
         << "time: " << duration << ' ' << "speed: "
         << (iterCount - startIterCount) * static_cast<double>(CLOCKS_PER_SEC) /
                (duration + 1)
         << endl;
#endif
}

NurseRostering::Solution::NRPEnv::NRPEnv(NurseRostering::Solution &sln,const FindBestMoveTable &findBestMoveTable,int maxNoImprove)
    : sln_(sln),initSln_(sln),optima_(sln), initState_(toTensor(sln_)), state_(initState_),findBestMoveTable_(findBestMoveTable),maxNoImprove(maxNoImprove){}

NurseRostering::Solution::Agent::Agent(int num_inputs, int num_actions, int hidden_size,
             double epsilon_start, double epsilon_end, int num_steps,
             int update_target, double gamma, int batch_size,
             int memory_capacity, double tau, NurseRostering::Solution &sln,const FindBestMoveTable &findBestMoveTable,int maxNoImprove,string qNetfilePath,string qTargetfilePath)
    : env_(sln,findBestMoveTable,maxNoImprove),
      q_net_(num_inputs, num_actions, hidden_size),
      q_target_(num_inputs, num_actions, hidden_size),
      replay_memory_(memory_capacity),
      optimizer_(q_net_.parameters(), torch::optim::AdamOptions(1e-3)),
      num_actions_(num_actions),
      epsilon_by_step_([epsilon_start, epsilon_end,
                        num_steps]() -> std::function<double(int)> {
          double epsilon_coefficient =
              log(epsilon_end / epsilon_start) / num_steps;
          return [epsilon_start, epsilon_coefficient](int step) {
              return epsilon_start * exp(epsilon_coefficient * step);
          };
      }()),
      batch_size_(batch_size),
      update_target_(update_target),
      gamma_(gamma),
      tau_(tau),
      num_steps_(num_steps),
      step_count_(0),
      max_no_improve_(0),
      episode_count_(0) {
            spdlog::info("load q_net weights from file");
            //if(sln.problem.history.currentWeek == 1){
            //
            //}else {
                q_net_.load_weights(qNetfilePath);
                q_target_.load_weights(qTargetfilePath);
            //}
     }

torch::Tensor NurseRostering::Solution::NRPEnv::reset(){
    //这样写的话从此刻开始，这个state_和sln_就不同步了，每次训练的就不一致了，所以可能还是需要一个init_solution
    //sln_.genInitAssign_Greedy();
    sln_.rebuild(initSln_);
    env_step_count_ = 0;
    step_no_improve = 0;
    cout << "reset sln_ = " << sln_.getObjValue()/DefaultPenalty::AMP << endl;
    state_ = toTensor(sln_);
    return state_;
}

std::tuple<torch::Tensor, double, bool> NurseRostering::Solution::NRPEnv::step(int action) {
    // action is the index of the best move
    ++env_step_count_;
    double reward = sln_.applyBestMoveTable(action,findBestMoveTable_);
    //在这里把逻辑换成no improve count
    if(reward <= 0) ++step_no_improve;
    //cout << env_step_count_ << endl;
    if(sln_.getObjValue() < optima_.getObjValue()) optima_ = sln_;
    //std::cout << " before step state_ = \n" << state_ << std::endl;
    state_ = toTensor(sln_);
    //std::cout << "after step state_ = \n" << state_ << std::endl;
    bool done = false;
    //env_step_count_ == (1 << 30) || step_no_improve == maxNoImprove
    if (env_step_count_ % 4500 == 0) {
        done = true;
    }
    return std::make_tuple(state_, reward, done);
}


NurseRostering::Solution::ReplayMemory::ReplayMemory(int capacity)
    : capacity_(capacity), generator_(std::random_device()()) {}

void NurseRostering::Solution::ReplayMemory::push(const torch::Tensor &state, const int action,
                        const double reward, const torch::Tensor &next_state,
                        const bool done) {
    if (memory_.size() == capacity_) {
        memory_.pop_back();
    }
    memory_.push_front({state, action, reward, next_state, done});
}

std::vector<NurseRostering::Solution::Experience> NurseRostering::Solution::ReplayMemory::sample(int batch_size) {
    std::vector<Experience> batch;
    std::uniform_int_distribution<int> dist(0, memory_.size() - 1);

    for (int i = 0; i < batch_size; ++i) {
        int idx = dist(generator_);
        batch.push_back(memory_[idx]);
    }

    return batch;
}

int NurseRostering::Solution::ReplayMemory::size() const { return memory_.size(); }

void NurseRostering::Solution::Agent::act(torch::Tensor &state, int &action, double &reward,
                torch::Tensor &next_state, bool &done) {
    if (step_count_ % update_target_ == 0) {
        // Update the target network with the weights of the current Q network
        auto q_params = q_net_.parameters();
        auto target_params = q_target_.parameters();
        for (size_t i = 0; i < q_params.size(); ++i) {
            auto &target_param = target_params[i];
            auto &q_param = q_params[i];
            target_param.data().copy_(tau_ * q_param.data() +
                                      (1.0 - tau_) * target_param.data());
        }

        // Ensure that the parameters of the two networks are the same
        TORCH_CHECK(q_params.size() == target_params.size(),
                    "Number of parameters must match.");
        for (size_t i = 0; i < q_params.size(); ++i) {
            auto &target_param = target_params[i];
            auto &q_param = q_params[i];
            TORCH_CHECK(target_param.data().equal(q_param.data()),
                        "Parameters must match.");
        }
    }

    double epsilon = epsilon_by_step_(step_count_);
    // std::cout << "state.sizes(): " << state.sizes() << std::endl;
    if (torch::rand({1}).item().toDouble() > epsilon) {
        auto q_values = q_net_.forward(state);
        action = q_values.argmax().item().toInt();
    } else {
        action = torch::randint(num_actions_, {1}).item().toInt();
    }

    std::tie(next_state, reward, done) = env_.step(action);
    // std::cout << "reward: " << reward << std::endl;
    //  std::cout << "next_state.sizes(): " << next_state.sizes() << std::endl;
    replay_memory_.push(state, action, reward, next_state, done);
    state = next_state;
    step_count_++;
}

void NurseRostering::Solution::Agent::learn() {
    if (replay_memory_.size() < batch_size_) {
        return;
    }
    optimizer_.zero_grad();

    auto batch = replay_memory_.sample(batch_size_);
    std::vector<torch::Tensor> states, next_states;
    std::vector<int> actions;
    std::vector<double> rewards;
    std::vector<bool> dones;
    for (const auto &experience : batch) {
        states.push_back(experience.state);
        actions.push_back(experience.action);
        rewards.push_back(experience.reward);
        next_states.push_back(experience.next_state);
        dones.push_back(experience.done);
    }
    //std::cout << "states = \n" << states << std::endl;
    torch::Tensor state_batch = torch::stack(states, 0);
    torch::Tensor action_batch =
        torch::tensor(actions, torch::TensorOptions().dtype(torch::kInt64))
            .unsqueeze(1);
    torch::Tensor reward_batch =
        torch::tensor(rewards, torch::TensorOptions().dtype(torch::kFloat32))
            .unsqueeze(1);
    torch::Tensor next_state_batch = torch::stack(next_states, 0);
    std::vector<int> done_int(dones.begin(), dones.end());
    torch::Tensor done_batch =
        torch::tensor(done_int, torch::TensorOptions().dtype(torch::kInt32))
            .unsqueeze(1);

    // std::cout << "state_batch.sizes(): " << state_batch.sizes() << std::endl;
    // std::cout << "action_batch.sizes(): " << action_batch.sizes() <<
    // std::endl; std::cout << "next_state_batch.sizes(): " <<
    // next_state_batch.sizes() << std::endl;

    // output q_net_ parameters
    // auto q_params = q_net_.parameters();
    // for (size_t i = 0; i < q_params.size(); ++i) {
    //    auto &q_param = q_params[i];
    //    std::cout << "q_param = \n" << q_param << std::endl;
    //}

    //std::cout << "state_batch = \n" << state_batch << std::endl;
    auto q_values =
        q_net_.forward(state_batch).gather(1, action_batch).squeeze(1);
    auto next_q_values =
        std::get<0>(q_target_.forward(next_state_batch).max(1));
    auto expected_q_values = reward_batch.view({-1}) + gamma_ * next_q_values;

    auto loss = torch::mse_loss(q_values, expected_q_values);
    //std::cout << "q_values = \n" << q_values << std::endl;
    //std::cout << "expected_q_values = \n" << expected_q_values << std::endl;
    //std::cout << "loss = " << loss.item().toDouble() << std::endl;
    // if (env_step_count_ % 100 == 0) {
    //     std::cout << "loss = " << loss.item().toDouble() << std::endl;
    //     std::cout << "q_values = " << q_values.mean().item().toDouble()
    //               << std::endl;
    // }

    loss.backward();
    optimizer_.step();
}

void NurseRostering::Solution::Agent::train(int kNumEpisodes,string q_net_path,string q_target_path) {
    torch::Tensor state = env_.reset();
    //std::cout << "state = \n"  << state << std::endl;
    int action;
    double reward;
    torch::Tensor next_state;
    bool done;

    std::vector<int> optimaVal;
    while (episode_count_ < kNumEpisodes) {
        if (episode_count_ % 100 == 0 && episode_count_ / 100 > 0) {
            std::cout << "episode_count_/kNumEpisodes = " << episode_count_
                      << "/" << kNumEpisodes << std::endl;
        }
        act(state, action, reward, next_state, done);
        //learn();

        if (done) {
            cout <<"new optima:" << env_.getOptima().getObjValue()/DefaultPenalty::AMP << endl;
            state = env_.reset();
            episode_count_++;
        } else {
            state = next_state;
        }
    }

    // save the network weights
    //q_net_.save_weights(q_net_path);
    //q_target_.save_weights(q_target_path);
}

torch::Tensor NurseRostering::Solution::NRPEnv::toTensor(Solution &solution){
    const int nurseNum = solution.problem.scenario.nurseNum;
    std::vector<double> assign;
    for (int w = 1; w <= 7; ++w) {
        for (int n = 0; n < 120; ++n) {
            if(n < nurseNum) {
                int shift = solution.getAssign(n, w).shift;
                int skill = solution.getAssign(n, w).skill;
                if (shift == NurseRostering::Scenario::Shift::ID_NONE) {
                    skill = 0;
                }
                assign.push_back(static_cast<double>(shift));
                assign.push_back(static_cast<double>(skill));
            }else{
                assign.push_back(static_cast<double>(-1));
                assign.push_back(static_cast<double>(-1));
            }
        }
    }
    // print assign
    //for (int i = 0; i < assign.size(); ++i) {
    //    std::cout << assign[i] << " ";
    //}
    //std::cout << std::endl;
    // create a tensor from assign
    size_t n = assign.size();
    float *data = new float[n];
    for (size_t i = 0; i < n; ++i) {
        data[i] = static_cast<float>(assign[i]);
    }
    torch::Tensor tensor =
        torch::from_blob(data, {7 * 120 * 2}, torch::kFloat32);
    //std::cout << "tensor = \n" << std::endl;
    //for (int i = 0; i < tensor.size(0); ++i) {
    //    std::cout << tensor[i].item().toDouble() << " ";
    //}
    //std::cout << std::endl;
    //delete[] data;
    return tensor;
}



void NurseRostering::Solution::dqnSearch(const Timer &timer,const FindBestMoveTable &findBestMoveTable,NurseRostering::IterCount maxNoImproveCount) {
    optima = *this;
    const int kBatchSize = 4;
    const double kGamma = 0.90;
    const double kEpsilonStart = 1.0;
    const double kEpsilonEnd = 0.01;
    const int kNumSteps = 1000000;
    const int kUpdateTarget = 1000;
    const int kNumEpisodes = 1;
    const int kHiddenSize = 128;
    const int kMemoryCapacity = 1000;
    const int nurseNum = problem.scenario.nurseNum;
    const double kTau = 1.0; // the target network update rate
    int modeNum = findBestMoveTable.size();
    string q_net_path = "q_net_basic.pt";
    string q_target_path = "q_target_basic.pt";
    Agent agent(2 * 120 * 7, findBestMoveTable.size(), kHiddenSize, kEpsilonStart, kEpsilonEnd, kNumSteps,
          kUpdateTarget, kGamma, kBatchSize, kMemoryCapacity, kTau, *this,findBestMoveTable,maxNoImproveCount,q_net_path,q_target_path);
    agent.train(kNumEpisodes,q_net_path,q_target_path);
}

void NurseRostering::Solution::dqnSearchDecetralization(const Timer &timer,const FindBestMoveTable &findBestMoveTable,NurseRostering::IterCount maxNoImproveCount) {
    optima = *this;
    const int kBatchSize = 4;
    const double kGamma = 0.90;
    const double kEpsilonStart = 1.0;
    const double kEpsilonEnd = 0.01;
    const int kNumSteps = 1000000;
    const int kUpdateTarget = 1000;
    const int kNumEpisodes = 1;
    const int kHiddenSize = 128;
    const int kMemoryCapacity = 1000;
    const int nurseNum = problem.scenario.nurseNum;
    const double kTau = 1.0; // the target network update rate
    int modeNum = findBestMoveTable.size();
    cout << modeNum << endl;
    string q_net_path = "q_net_dec_4.pt";
    string q_target_path = "q_target_dec_4.pt";
    Agent agent(2 * 120 * 7, findBestMoveTable.size(), kHiddenSize, kEpsilonStart, kEpsilonEnd, kNumSteps,
                kUpdateTarget, kGamma, kBatchSize, kMemoryCapacity, kTau, *this,findBestMoveTable,maxNoImproveCount,q_net_path,q_target_path);
    agent.train(kNumEpisodes,q_net_path,q_target_path);
}

void NurseRostering::Solution::compareSearch(const Timer &timer,const FindBestMoveTable &findBestMoveTable,NurseRostering::IterCount maxNoImproveCount){
    Output initSolution = *this;
    for(int i = 0; i < 1; i++){
        tabuSearch_Rand(findBestMoveTable, 4500);
        rebuild(initSolution);
    }
}

void NurseRostering::Solution::VNDSearch(const Timer &timer,const FindBestMoveTable &findBestMoveTable,NurseRostering::IterCount maxNoImproveCount){
    Output initSolution = *this;
    for(int i = 0; i < 1; i++){
        VNDSearch_Rand(findBestMoveTable, 4500);
        rebuild(initSolution);
    }
}

void NurseRostering::Solution::tabuSearch_Rand(
    const FindBestMoveTable &findBestMoveTable,
    IterCount maxIterCount) {
#ifdef INRC2_PERFORMANCE_TEST
    clock_t startTime = clock();
    IterCount startIterCount = iterCount;
#endif
    optima = *this;

    int modeNum = findBestMoveTable.size();

    const int weight_Invalid = 128;  // min weight
    const int weight_NoImprove = 256;
    const int weight_ImproveCur = 1024;
    const int weight_ImproveOpt =
        4096;  // max weight (less than (RAND_MAX / modeNum))
    const int initWeight = (weight_ImproveCur + weight_NoImprove) / 2;
    const int deltaIncRatio = 8;  // = weights[mode] / weightDelta
    const int incError = deltaIncRatio - 1;
    const int deltaDecRatio = 8;  // = weights[mode] / weightDelta
    const int decError = -(deltaDecRatio - 1);
    vector<int> weights(modeNum, initWeight);
    int totalWeight = initWeight * modeNum;

    int iterCount = 0;
    for (; iterCount < maxIterCount; ++iterCount) {
        int modeSelect = 0;
        for (int w = solver.randGen() % totalWeight;
             (w -= weights[modeSelect]) >= 0; ++modeSelect) {
        }

        Move bestMove;
        (this->*findBestMoveTable[modeSelect])(bestMove);

        int weightDelta;
        if (bestMove.delta < DefaultPenalty::MAX_OBJ_VALUE) {
#ifdef INRC2_USE_TABU
            // update tabu list first because it requires original assignment
            (this->*updateTabu[bestMove.mode])(bestMove);
#endif
            applyBasicMove(bestMove);

            if (updateOptima()) {  // improve optima
#ifdef INRC2_LS_AFTER_TSR_UPDATE_OPT
                localSearch(timer, findBestMoveTable);
#endif
                weightDelta =
                    (incError + weight_ImproveOpt - weights[modeSelect]) /
                    deltaIncRatio;
            } else {
                if (bestMove.delta < 0) {  // improve current solution
                    weightDelta = (weights[modeSelect] < weight_ImproveCur)
                                      ? (incError + weight_ImproveCur -
                                         weights[modeSelect]) /
                                            deltaIncRatio
                                      : (decError + weight_ImproveCur -
                                         weights[modeSelect]) /
                                            deltaDecRatio;
                } else {  // no improve but valid
                    weightDelta = (weights[modeSelect] < weight_NoImprove)
                                      ? (incError + weight_NoImprove -
                                         weights[modeSelect]) /
                                            deltaIncRatio
                                      : (decError + weight_NoImprove -
                                         weights[modeSelect]) /
                                            deltaDecRatio;
                }
            }
        } else {  // invalid
            weightDelta = (decError + weight_Invalid - weights[modeSelect]) /
                          deltaDecRatio;
        }

        weights[modeSelect] += weightDelta;
        totalWeight += weightDelta;
    }
#ifdef INRC2_PERFORMANCE_TEST
    clock_t duration = clock() - startTime;
    cout << "[TS] iter: " << (iterCount - startIterCount) << ' '
         << "time: " << duration << ' ' << "speed: "
         << (iterCount - startIterCount) * static_cast<double>(CLOCKS_PER_SEC) /
                (duration + 1)
         << endl;
#endif
}

void NurseRostering::Solution::VNDSearch_Rand(
    const FindBestMoveTable &findBestMoveTable,
    IterCount maxIterCount) {
#ifdef INRC2_PERFORMANCE_TEST
    clock_t startTime = clock();
    IterCount startIterCount = iterCount;
#endif
    optima = *this;

    int modeNum = findBestMoveTable.size();

    int iterCount = 0;
    int modeSelect = 0;
    for (; iterCount < maxIterCount; ++iterCount) {

        Move bestMove;
        (this->*findBestMoveTable[modeSelect])(bestMove);

        if (bestMove.delta < DefaultPenalty::MAX_OBJ_VALUE) {
#ifdef INRC2_USE_TABU
            // update tabu list first because it requires original assignment
            (this->*updateTabu[bestMove.mode])(bestMove);
#endif
            applyBasicMove(bestMove);
            iterCount++;

            if (updateOptima()) {  // improve optima
#ifdef INRC2_LS_AFTER_TSR_UPDATE_OPT
                localSearch(timer, findBestMoveTable);
#endif
                modeSelect = 0;
            } else {
                if(modeSelect < modeNum - 1) {
                    modeSelect++;
                }else{
                    modeSelect = 0;
                }
            }
        }
    }
    cout << "iterCount: " << iterCount << endl;
#ifdef INRC2_PERFORMANCE_TEST
    clock_t duration = clock() - startTime;
    cout << "[TS] iter: " << (iterCount - startIterCount) << ' '
         << "time: " << duration << ' ' << "speed: "
         << (iterCount - startIterCount) * static_cast<double>(CLOCKS_PER_SEC) /
                (duration + 1)
         << endl;
#endif
}

double NurseRostering::Solution::applyBestMoveTable(int action, const FindBestMoveTable &findBestMoveTable){
    Move bestMove;
    (this->*findBestMoveTable[action])(bestMove);
    if (bestMove.delta < DefaultPenalty::MAX_OBJ_VALUE) {
        applyBasicMove(bestMove);
        updateOptima();
        //if(this->getObjValue() < optima.getObjValue()){ //这里更新的是sln的最优解，不是全局最优解，怎么去更新全局最优解呢
        //    optima = *this;
        //    //cout << "new optima: " << optima.getObjValue()/DefaultPenalty::AMP << endl;
        //}
        return (-1.0) * (bestMove.delta/DefaultPenalty::AMP) ;
    }
    return -100;
}

void NurseRostering::Solution::tabuSearch_Loop(
    const Timer &timer, const FindBestMoveTable &findBestMoveTable,
    IterCount maxNoImproveCount) {
#ifdef INRC2_PERFORMANCE_TEST
    clock_t startTime = clock();
    IterCount startIterCount = iterCount;
#endif
    optima = *this;

    int modeNum = findBestMoveTable.size();

    int failCount = modeNum;
    int modeSelect = 0;
    // since there is randomness on the search trajectory,
    // there will be chance to make a difference on neighborhoods
    // which have been searched. so search (modeNum + 1) times
    while (!timer.isTimeOut() && (failCount >= 0)) {
        // reset current solution to best solution found in last neighborhood
        rebuild(optima);

        IterCount noImprove_Single = maxNoImproveCount;
        for (; !timer.isTimeOut() && (noImprove_Single > 0) &&
               (iterCount < problem.maxIterCount);
             ++iterCount) {
            Move bestMove;
            (this->*findBestMoveTable[modeSelect])(bestMove);

            if (bestMove.delta >= DefaultPenalty::MAX_OBJ_VALUE) {
                break;
            }

#ifdef INRC2_USE_TABU
            // update tabu list first because it requires original assignment
            (this->*updateTabu[bestMove.mode])(bestMove);
#endif
            applyBasicMove(bestMove);

            if (updateOptima()) {  // improved
                failCount = modeNum;
                noImprove_Single = maxNoImproveCount;
            } else {  // not improved
                --noImprove_Single;
            }
        }

        --failCount;
        (++modeSelect) %= modeNum;
    }
#ifdef INRC2_PERFORMANCE_TEST
    clock_t duration = clock() - startTime;
    cout << "[TS] iter: " << (iterCount - startIterCount) << ' '
         << "time: " << duration << ' ' << "speed: "
         << (iterCount - startIterCount) * static_cast<double>(CLOCKS_PER_SEC) /
                (duration + 1)
         << endl;
#endif
}

void NurseRostering::Solution::tabuSearch_Possibility(
    const Timer &timer, const FindBestMoveTable &findBestMoveTable,
    IterCount maxNoImproveCount) {
#ifdef INRC2_PERFORMANCE_TEST
    clock_t startTime = clock();
    IterCount startIterCount = iterCount;
#endif
    isPossibilitySelect = true;

    optima = *this;

    int modeNum = findBestMoveTable.size();
    int startMode = 0;

    const unsigned maxP_local =
        (solver.randGen.max() - solver.randGen.min()) / modeNum;
    const unsigned maxP_global =
        (solver.randGen.max() - solver.randGen.min()) * (modeNum - 1) / modeNum;
    const double amp_local = 1.0 / (2 * modeNum);
    const double amp_global = 1.0 / (4 * modeNum * modeNum);
    const double dec_local = (2.0 * modeNum - 1) / (2 * modeNum);
    const double dec_global =
        (2.0 * modeNum * modeNum - 1) / (2 * modeNum * modeNum);
    unsigned P_global = (solver.randGen.max() - solver.randGen.min()) / modeNum;
    vector<unsigned> P_local(modeNum, 0);

    IterCount noImprove = maxNoImproveCount;
    for (; !timer.isTimeOut() && (noImprove > 0) &&
           (iterCount < problem.maxIterCount);
         ++iterCount) {
        int modeSelect = startMode;
        Move::Mode moveMode = Move::Mode::SIZE;
        Move bestMove;
        isBlockSwapSelected = false;
        // judge every neighborhood whether to select and search when selected
        // start from big end to make sure block swap will be tested before swap
        for (int i = modeNum - 1; i >= 0; --i) {
            if (solver.randGen() <
                (P_global + P_local[i] + solver.randGen.min())) {  // selected
                (this->*findBestMoveTable[i])(bestMove);
                if (moveMode != bestMove.mode) {
                    moveMode = bestMove.mode;
                    modeSelect = i;
                }
            }
        }

        // no one is selected
        while (bestMove.delta >= DefaultPenalty::MAX_OBJ_VALUE) {
            (this->*findBestMoveTable[modeSelect])(bestMove);
            modeSelect += (bestMove.delta >= DefaultPenalty::MAX_OBJ_VALUE);
            modeSelect %= modeNum;
        }

#ifdef INRC2_USE_TABU
        // update tabu list first because it requires original assignment
        (this->*updateTabu[bestMove.mode])(bestMove);
#endif
        applyBasicMove(bestMove);

        if (updateOptima()) {  // improved
            noImprove = maxNoImproveCount;
            P_global = static_cast<unsigned>(P_global * dec_global);
            P_local[modeSelect] += static_cast<unsigned>(
                amp_local * (maxP_local - P_local[modeSelect]));
        } else {  // not improved
            --noImprove;
            (++startMode) %= modeNum;
            P_global +=
                static_cast<unsigned>(amp_global * (maxP_global - P_global));
            P_local[modeSelect] =
                static_cast<unsigned>(P_local[modeSelect] * dec_local);
        }
    }

    isPossibilitySelect = false;
#ifdef INRC2_PERFORMANCE_TEST
    clock_t duration = clock() - startTime;
    cout << "[TS] iter: " << (iterCount - startIterCount) << ' '
         << "time: " << duration << ' ' << "speed: "
         << (iterCount - startIterCount) * static_cast<double>(CLOCKS_PER_SEC) /
                (duration + 1)
         << endl;
#endif
}

void NurseRostering::Solution::localSearch(
    const Timer &timer, const FindBestMoveTable &findBestMoveTable) {
#ifdef INRC2_PERFORMANCE_TEST
    clock_t startTime = clock();
    IterCount startIterCount = iterCount;
#endif
    optima = *this;

    int modeNum = findBestMoveTable.size();

    int failCount = modeNum;
    int modeSelect = 0;
    while (!timer.isTimeOut() && (failCount > 0) &&
           (iterCount != problem.maxIterCount)) {
        Move bestMove;
        if ((this->*findBestMoveTable[modeSelect])(bestMove)) {
            applyBasicMove(bestMove);
            updateOptima();
            ++iterCount;
            failCount = modeNum;
        } else {
            --failCount;
            (++modeSelect) %= modeNum;
        }
    }
#ifdef INRC2_PERFORMANCE_TEST
    clock_t duration = clock() - startTime;
    cout << "[LS] iter: " << (iterCount - startIterCount) << ' '
         << "time: " << duration << ' ' << "speed: "
         << (iterCount - startIterCount) * static_cast<double>(CLOCKS_PER_SEC) /
                (duration + 1)
         << endl;
#endif
}

void NurseRostering::Solution::randomWalk(const Timer &timer,
                                          IterCount stepNum) {
#ifdef INRC2_PERFORMANCE_TEST
    clock_t startTime = clock();
    IterCount startIterCount = stepNum;
#endif
    optima = *this;

    stepNum += iterCount;
    while ((iterCount < stepNum) && !timer.isTimeOut() &&
           (iterCount < problem.maxIterCount)) {
        Move move;
        move.mode = static_cast<Move::Mode>(solver.randGen() %
                                            Move::Mode::BASIC_MOVE_SIZE);
        move.weekday = (solver.randGen() % Weekday::NUM) + Weekday::Mon;
        move.weekday2 = (solver.randGen() % Weekday::NUM) + Weekday::Mon;
        if (move.weekday > move.weekday2) {
            swap(move.weekday, move.weekday2);
        }
        move.nurse = solver.randGen() % problem.scenario.nurseNum;
        move.nurse2 = solver.randGen() % problem.scenario.nurseNum;
        move.assign.shift = NurseRostering::Scenario::Shift::ID_BEGIN +
                            (solver.randGen() % problem.scenario.shiftTypeNum);
        move.assign.skill = NurseRostering::Scenario::Skill::ID_BEGIN +
                            (solver.randGen() % problem.scenario.skillTypeNum);

        move.delta = (this->*tryMove[move.mode])(move);
        if (move.delta < DefaultPenalty::MAX_OBJ_VALUE) {
            applyBasicMove(move);
            ++iterCount;
        }
    }
#ifdef INRC2_PERFORMANCE_TEST
    clock_t duration = clock() - startTime;
    cout << "[RW] iter: " << startIterCount << ' ' << "time: " << duration
         << ' ' << "speed: "
         << startIterCount * static_cast<double>(CLOCKS_PER_SEC) /
                (duration + 1)
         << endl;
#endif
}

void NurseRostering::Solution::perturb(double strength) {
    // TODO : make this change solution structure in certain complexity
    int randomWalkStepCount =
        static_cast<int>(strength * problem.scenario.shiftTypeNum *
                         problem.scenario.nurseNum * Weekday::NUM);

    randomWalk(solver.timer, randomWalkStepCount);

    updateOptima();
}

bool NurseRostering::Solution::findBestAdd(Move &bestMove) const {
    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Add;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
             ++move.weekday) {
            if (!assign.isWorking(move.nurse, move.weekday)) {
                for (move.assign.shift =
                         NurseRostering::Scenario::Shift::ID_BEGIN;
                     move.assign.shift < problem.scenario.shiftSize;
                     ++move.assign.shift) {
                    for (move.assign.skill =
                             NurseRostering::Scenario::Skill::ID_BEGIN;
                         move.assign.skill < problem.scenario.skillSize;
                         ++move.assign.skill) {
                        move.delta = tryAddAssign(move);
#ifdef INRC2_USE_TABU
                        if (noAddTabu(move)) {
#endif
                            if (rs.isMinimal(move.delta, bestMove.delta,
                                             solver.randGen)) {
                                bestMove = move;
                            }
#ifdef INRC2_USE_TABU
                        } else {  // tabu
                            if (rs_tabu.isMinimal(move.delta,
                                                  bestMove_tabu.delta,
                                                  solver.randGen)) {
                                bestMove_tabu = move;
                            }
                        }
#endif
                    }
                }
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestChange(Move &bestMove) const {
    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Change;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
             ++move.weekday) {
            if (assign.isWorking(move.nurse, move.weekday)) {
                for (move.assign.shift =
                         NurseRostering::Scenario::Shift::ID_BEGIN;
                     move.assign.shift < problem.scenario.shiftSize;
                     ++move.assign.shift) {
                    for (move.assign.skill =
                             NurseRostering::Scenario::Skill::ID_BEGIN;
                         move.assign.skill < problem.scenario.skillSize;
                         ++move.assign.skill) {
                        move.delta = tryChangeAssign(move);
#ifdef INRC2_USE_TABU
                        if (noChangeTabu(move)) {
#endif
                            if (rs.isMinimal(move.delta, bestMove.delta,
                                             solver.randGen)) {
                                bestMove = move;
                            }
#ifdef INRC2_USE_TABU
                        } else {  // tabu
                            if (rs_tabu.isMinimal(move.delta,
                                                  bestMove_tabu.delta,
                                                  solver.randGen)) {
                                bestMove_tabu = move;
                            }
                        }
#endif
                    }
                }
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestRemove(Move &bestMove) const {
    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Remove;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
             ++move.weekday) {
            if (assign.isWorking(move.nurse, move.weekday)) {
                move.delta = tryRemoveAssign(move);
#ifdef INRC2_USE_TABU
                if (noRemoveTabu(move)) {
#endif
                    if (rs.isMinimal(move.delta, bestMove.delta,
                                     solver.randGen)) {
                        bestMove = move;
                    }
#ifdef INRC2_USE_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(move.delta, bestMove_tabu.delta,
                                          solver.randGen)) {
                        bestMove_tabu = move;
                    }
                }
#endif
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestSwap(Move &bestMove) const {
    if (isPossibilitySelect && isBlockSwapSelected) {
        return false;
    }

    penalty.setSwapMode();

    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Swap;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        for (move.nurse2 = move.nurse + 1;
             move.nurse2 < problem.scenario.nurseNum; ++move.nurse2) {
            if ((nurseWeights[move.nurse] == 0) &&
                (nurseWeights[move.nurse2] == 0)) {
                continue;
            }
            for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
                 ++move.weekday) {
                move.delta =
                    trySwapNurse(move.weekday, move.nurse, move.nurse2);
#ifdef INRC2_USE_TABU
                if (noSwapTabu(move)) {
#endif
                    if (rs.isMinimal(move.delta, bestMove.delta,
                                     solver.randGen)) {
                        bestMove = move;
                    }
#ifdef INRC2_USE_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(move.delta, bestMove_tabu.delta,
                                          solver.randGen)) {
                        bestMove_tabu = move;
                    }
                }
#endif
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestBlockSwap(Move &bestMove) const {
    const NurseID maxNurseID = problem.scenario.nurseNum - 1;

    isBlockSwapSelected = true;
    penalty.setBlockSwapMode();

    RandSelect<ObjValue> rs;

    Move move;
    move.mode = Move::Mode::BlockSwap;
    move.nurse = findBestBlockSwap_startNurse;
    for (NurseID count = problem.scenario.nurseNum; count > 0; --count) {
        (move.nurse < maxNurseID) ? (++move.nurse) : (move.nurse = 0);
        move.nurse2 = move.nurse;
        for (NurseID count2 = count - 1; count2 > 0; --count2) {
            (move.nurse2 < maxNurseID) ? (++move.nurse2) : (move.nurse2 = 0);
            if ((nurseWeights[move.nurse] == 0) &&
                (nurseWeights[move.nurse2] == 0)) {
                continue;
            }
            if (solver.haveSameSkill(move.nurse, move.nurse2)) {
                for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
                     ++move.weekday) {
                    move.delta = trySwapBlock(move.weekday, move.weekday2,
                                              move.nurse, move.nurse2);
                    if (rs.isMinimal(move.delta, bestMove.delta,
                                     solver.randGen)) {
                        bestMove = move;
#ifdef INRC2_BLOCK_SWAP_FIRST_IMPROVE
                        if (bestMove.delta < 0) {
                            findBestBlockSwap_startNurse = move.nurse;
                            penalty.recoverLastMode();
                            return true;
                        }
#endif
                    }
                }
            }
        }
    }

    findBestBlockSwap_startNurse = move.nurse;
    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestBlockSwap_cached(Move &bestMove) const {
    isBlockSwapSelected = true;
    penalty.setBlockSwapMode();

    RandSelect<ObjValue> rs;

    Move move;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        for (move.nurse2 = move.nurse + 1;
             move.nurse2 < problem.scenario.nurseNum; ++move.nurse2) {
            if (((nurseWeights[move.nurse] == 0) &&
                 (nurseWeights[move.nurse2] == 0)) ||
                !solver.haveSameSkill(move.nurse, move.nurse2)) {
                continue;
            }
            BlockSwapCacheItem &cache(blockSwapCache[move.nurse][move.nurse2]);
            if (!(isBlockSwapCacheValid[move.nurse] &&
                  isBlockSwapCacheValid[move.nurse2])) {
                RandSelect<ObjValue> rs_currentPair;
                cache.delta = DefaultPenalty::FORBIDDEN_MOVE;
                for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
                     ++move.weekday) {
                    move.delta = trySwapBlock(move.weekday, move.weekday2,
                                              move.nurse, move.nurse2);
                    if (rs_currentPair.isMinimal(move.delta, cache.delta,
                                                 solver.randGen)) {
                        cache.delta = move.delta;
                        cache.weekday = move.weekday;
                        cache.weekday2 = move.weekday2;
                    }
                }
            }
            if (rs.isMinimal(cache.delta, bestMove.delta, solver.randGen)) {
                bestMove.mode = Move::Mode::BlockSwap;
                bestMove.delta = cache.delta;
                bestMove.nurse = move.nurse;
                bestMove.nurse2 = move.nurse2;
                bestMove.weekday = cache.weekday;
                bestMove.weekday2 = cache.weekday2;
            }
        }
        isBlockSwapCacheValid[move.nurse] = true;
    }

    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestBlockSwap_fast(Move &bestMove) const {
    const NurseID maxNurseID = problem.scenario.nurseNum - 1;

    isBlockSwapSelected = true;
    penalty.setBlockSwapMode();

    RandSelect<ObjValue> rs;

    Move move;
    move.mode = Move::Mode::BlockSwap;
    move.nurse = findBestBlockSwap_startNurse;
    for (NurseID count = problem.scenario.nurseNum; count > 0; --count) {
        (move.nurse < maxNurseID) ? (++move.nurse) : (move.nurse = 0);
        move.nurse2 = move.nurse;
        for (NurseID count2 = count - 1; count2 > 0; --count2) {
            (move.nurse2 < maxNurseID) ? (++move.nurse2) : (move.nurse2 = 0);
            if ((nurseWeights[move.nurse] == 0) &&
                (nurseWeights[move.nurse2] == 0)) {
                continue;
            }
            if (solver.haveSameSkill(move.nurse, move.nurse2)) {
                move.delta = trySwapBlock_fast(move.weekday, move.weekday2,
                                               move.nurse, move.nurse2);
                if (rs.isMinimal(move.delta, bestMove.delta, solver.randGen)) {
                    bestMove = move;
#ifdef INRC2_BLOCK_SWAP_FIRST_IMPROVE
                    if (bestMove.delta < 0) {
                        findBestBlockSwap_startNurse = move.nurse;
                        penalty.recoverLastMode();
                        return true;
                    }
#endif
                }
            }
        }
    }

    findBestBlockSwap_startNurse = move.nurse;
    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestBlockSwap_part(Move &bestMove) const {
    const int nurseNum_noTry =
        problem.scenario.nurseNum - problem.scenario.nurseNum / 4;
    const NurseID maxNurseID = problem.scenario.nurseNum - 1;

    isBlockSwapSelected = true;
    penalty.setBlockSwapMode();

    RandSelect<ObjValue> rs;

    Move move;
    move.mode = Move::Mode::BlockSwap;
    move.nurse = findBestBlockSwap_startNurse;
    for (NurseID count = problem.scenario.nurseNum; count > nurseNum_noTry;
         --count) {
        (move.nurse < maxNurseID) ? (++move.nurse) : (move.nurse = 0);
        move.nurse2 = move.nurse;
        for (NurseID count2 = count - 1; count2 > 0; --count2) {
            (move.nurse2 < maxNurseID) ? (++move.nurse2) : (move.nurse2 = 0);
            if ((nurseWeights[move.nurse] == 0) &&
                (nurseWeights[move.nurse2] == 0)) {
                continue;
            }
            if (solver.haveSameSkill(move.nurse, move.nurse2)) {
                move.delta = trySwapBlock_fast(move.weekday, move.weekday2,
                                               move.nurse, move.nurse2);
                if (rs.isMinimal(move.delta, bestMove.delta, solver.randGen)) {
                    bestMove = move;
#ifdef INRC2_BLOCK_SWAP_FIRST_IMPROVE
                    if (bestMove.delta < 0) {
                        findBestBlockSwap_startNurse = move.nurse;
                        penalty.recoverLastMode();
                        return true;
                    }
#endif
                }
            }
        }
    }

    findBestBlockSwap_startNurse = move.nurse;
    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestBlockSwap_rand(Move &bestMove) const {
    const int nurseNum_noTry =
        problem.scenario.nurseNum - problem.scenario.nurseNum / 4;
    const NurseID maxNurseID = problem.scenario.nurseNum - 1;

    isBlockSwapSelected = true;
    penalty.setBlockSwapMode();

    RandSelect<ObjValue> rs;

    Move move;
    move.mode = Move::Mode::BlockSwap;
    for (NurseID count = problem.scenario.nurseNum; count > nurseNum_noTry;
         --count) {
        move.nurse = solver.randGen() % problem.scenario.nurseNum;
        move.nurse2 = move.nurse;
        for (NurseID count2 = count - 1; count2 > 0; --count2) {
            (move.nurse2 < maxNurseID) ? (++move.nurse2) : (move.nurse2 = 0);
            if ((nurseWeights[move.nurse] == 0) &&
                (nurseWeights[move.nurse2] == 0)) {
                continue;
            }
            if (solver.haveSameSkill(move.nurse, move.nurse2)) {
                move.delta = trySwapBlock_fast(move.weekday, move.weekday2,
                                               move.nurse, move.nurse2);
                if (rs.isMinimal(move.delta, bestMove.delta, solver.randGen)) {
                    bestMove = move;
#ifdef INRC2_BLOCK_SWAP_FIRST_IMPROVE
                    if (bestMove.delta < 0) {
                        findBestBlockSwap_startNurse = move.nurse;
                        penalty.recoverLastMode();
                        return true;
                    }
#endif
                }
            }
        }
    }

    findBestBlockSwap_startNurse = move.nurse;
    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestExchange(Move &bestMove) const {
    penalty.setExchangeMode();

    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Exchange;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
             ++move.weekday) {
            for (move.weekday2 = move.weekday + 1;
                 move.weekday2 <= Weekday::Sun; ++move.weekday2) {
                move.delta =
                    tryExchangeDay(move.weekday, move.nurse, move.weekday2);
#ifdef INRC2_USE_TABU
                if (noExchangeTabu(move)) {
#endif
                    if (rs.isMinimal(move.delta, bestMove.delta,
                                     solver.randGen)) {
                        bestMove = move;
                    }
#ifdef INRC2_USE_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(move.delta, bestMove_tabu.delta,
                                          solver.randGen)) {
                        bestMove_tabu = move;
                    }
                }
#endif
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestBlockShift(Move &bestMove) const {
    penalty.setExchangeMode();

    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Exchange;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        const Consecutive &c(consecutives[move.nurse]);
        move.weekday = Weekday::Mon;
        move.weekday2 = c.dayHigh[move.weekday] + 1;
        while (move.weekday2 <= Weekday::Sun) {
            move.delta =
                tryExchangeDay(move.weekday, move.nurse, move.weekday2);
#ifdef INRC2_USE_TABU
            if (noExchangeTabu(move)) {
#endif
                if (rs.isMinimal(move.delta, bestMove.delta, solver.randGen)) {
                    bestMove = move;
                }
#ifdef INRC2_USE_TABU
            } else {  // tabu
                if (rs_tabu.isMinimal(move.delta, bestMove_tabu.delta,
                                      solver.randGen)) {
                    bestMove_tabu = move;
                }
            }
#endif
            if (move.weekday != c.dayHigh[move.weekday]) {  // start of a block
                move.weekday = c.dayHigh[move.weekday];
                move.weekday2 = c.dayHigh[move.weekday + 1];
            } else {  // end of a block
                ++move.weekday;
                move.weekday2 = c.dayHigh[move.weekday] + 1;
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestARLoop(Move &bestMove) const {
    bool isImproved;
    isImproved =
        findBestARLoop_flag ? findBestAdd(bestMove) : findBestRemove(bestMove);

    if (!isImproved) {
        findBestARLoop_flag = !findBestARLoop_flag;
    }

    return isImproved;
}

bool NurseRostering::Solution::findBestARRand(Move &bestMove) const {
    return ((solver.randGen() % 2) ? findBestAdd(bestMove)
                                   : findBestRemove(bestMove));
}

bool NurseRostering::Solution::findBestARBoth(Move &bestMove) const {
    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;
             ++move.weekday) {
            if (assign.isWorking(move.nurse, move.weekday)) {
                move.delta = tryRemoveAssign(move);
#ifdef INRC2_USE_TABU
                if (noRemoveTabu(move)) {
#endif
                    if (rs.isMinimal(move.delta, bestMove.delta,
                                     solver.randGen)) {
                        bestMove = move;
                        bestMove.mode = Move::Mode::Remove;
                    }
#ifdef INRC2_USE_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(move.delta, bestMove_tabu.delta,
                                          solver.randGen)) {
                        bestMove_tabu = move;
                        bestMove_tabu.mode = Move::Mode::Remove;
                    }
                }
#endif
            } else {
                for (move.assign.shift =
                         NurseRostering::Scenario::Shift::ID_BEGIN;
                     move.assign.shift < problem.scenario.shiftSize;
                     ++move.assign.shift) {
                    for (move.assign.skill =
                             NurseRostering::Scenario::Skill::ID_BEGIN;
                         move.assign.skill < problem.scenario.skillSize;
                         ++move.assign.skill) {
                        move.delta = tryAddAssign(move);
#ifdef INRC2_USE_TABU
                        if (noAddTabu(move)) {
#endif
                            if (rs.isMinimal(move.delta, bestMove.delta,
                                             solver.randGen)) {
                                bestMove = move;
                                bestMove.mode = Move::Mode::Add;
                            }
#ifdef INRC2_USE_TABU
                        } else {  // tabu
                            if (rs_tabu.isMinimal(move.delta,
                                                  bestMove_tabu.delta,
                                                  solver.randGen)) {
                                bestMove_tabu = move;
                                bestMove_tabu.mode = Move::Mode::Add;
                            }
                        }
#endif
                    }
                }
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestAddOnBlockBorder(Move &bestMove) const {
    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Add;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        const Consecutive &c(consecutives[move.nurse]);
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;) {
            if (!assign.isWorking(move.nurse, move.weekday)) {
                for (move.assign.shift =
                         NurseRostering::Scenario::Shift::ID_BEGIN;
                     move.assign.shift < problem.scenario.shiftSize;
                     ++move.assign.shift) {
                    for (move.assign.skill =
                             NurseRostering::Scenario::Skill::ID_BEGIN;
                         move.assign.skill < problem.scenario.skillSize;
                         ++move.assign.skill) {
                        move.delta = tryAddAssign(move);
#ifdef INRC2_USE_TABU
                        if (noAddTabu(move)) {
#endif
                            if (rs.isMinimal(move.delta, bestMove.delta,
                                             solver.randGen)) {
                                bestMove = move;
                            }
#ifdef INRC2_USE_TABU
                        } else {  // tabu
                            if (rs_tabu.isMinimal(move.delta,
                                                  bestMove_tabu.delta,
                                                  solver.randGen)) {
                                bestMove_tabu = move;
                            }
                        }
#endif
                    }
                }
                move.weekday = (move.weekday != c.dayHigh[move.weekday])
                                   ? c.dayHigh[move.weekday]
                                   : (move.weekday + 1);
            } else {
                move.weekday = c.dayHigh[move.weekday] + 1;
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestChangeOnBlockBorder(
    Move &bestMove) const {
    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Change;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        const Consecutive &c(consecutives[move.nurse]);
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;) {
            if (assign.isWorking(move.nurse, move.weekday)) {
                for (move.assign.shift =
                         NurseRostering::Scenario::Shift::ID_BEGIN;
                     move.assign.shift < problem.scenario.shiftSize;
                     ++move.assign.shift) {
                    for (move.assign.skill =
                             NurseRostering::Scenario::Skill::ID_BEGIN;
                         move.assign.skill < problem.scenario.skillSize;
                         ++move.assign.skill) {
                        move.delta = tryChangeAssign(move);
#ifdef INRC2_USE_TABU
                        if (noChangeTabu(move)) {
#endif
                            if (rs.isMinimal(move.delta, bestMove.delta,
                                             solver.randGen)) {
                                bestMove = move;
                            }
#ifdef INRC2_USE_TABU
                        } else {  // tabu
                            if (rs_tabu.isMinimal(move.delta,
                                                  bestMove_tabu.delta,
                                                  solver.randGen)) {
                                bestMove_tabu = move;
                            }
                        }
#endif
                    }
                }
                move.weekday = (move.weekday != c.shiftHigh[move.weekday])
                                   ? c.shiftHigh[move.weekday]
                                   : (move.weekday + 1);
            } else {
                move.weekday = c.shiftHigh[move.weekday] + 1;
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestRemoveOnBlockBorder(
    Move &bestMove) const {
    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Remove;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        const Consecutive &c(consecutives[move.nurse]);
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;) {
            if (assign.isWorking(move.nurse, move.weekday)) {
                move.delta = tryRemoveAssign(move);
#ifdef INRC2_USE_TABU
                if (noRemoveTabu(move)) {
#endif
                    if (rs.isMinimal(move.delta, bestMove.delta,
                                     solver.randGen)) {
                        bestMove = move;
                    }
#ifdef INRC2_USE_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(move.delta, bestMove_tabu.delta,
                                          solver.randGen)) {
                        bestMove_tabu = move;
                    }
                }
#endif
                move.weekday = (move.weekday != c.dayHigh[move.weekday])
                                   ? c.dayHigh[move.weekday]
                                   : (move.weekday + 1);
            } else {
                move.weekday = c.dayHigh[move.weekday] + 1;
            }
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestSwapOnBlockBorder(Move &bestMove) const {
    if (isPossibilitySelect && isBlockSwapSelected) {
        return false;
    }

    penalty.setSwapMode();

    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Swap;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        const Consecutive &c(consecutives[move.nurse]);
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;) {
            for (move.nurse2 = 0; move.nurse2 < problem.scenario.nurseNum;
                 ++move.nurse2) {
                if ((nurseWeights[move.nurse] == 0) &&
                    (nurseWeights[move.nurse2] == 0)) {
                    continue;
                }
                move.delta =
                    trySwapNurse(move.weekday, move.nurse, move.nurse2);
#ifdef INRC2_USE_TABU
                if (noSwapTabu(move)) {
#endif
                    if (rs.isMinimal(move.delta, bestMove.delta,
                                     solver.randGen)) {
                        bestMove = move;
                    }
#ifdef INRC2_USE_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(move.delta, bestMove_tabu.delta,
                                          solver.randGen)) {
                        bestMove_tabu = move;
                    }
                }
#endif
            }
            move.weekday = (move.weekday != c.dayHigh[move.weekday])
                               ? c.dayHigh[move.weekday]
                               : (move.weekday + 1);
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestExchangeOnBlockBorder(
    Move &bestMove) const {
    penalty.setExchangeMode();

    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    move.mode = Move::Mode::Exchange;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        const Consecutive &c(consecutives[move.nurse]);
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;) {
            for (move.weekday2 = Weekday::Mon; move.weekday2 <= Weekday::Sun;
                 ++move.weekday2) {
                move.delta =
                    tryExchangeDay(move.weekday, move.nurse, move.weekday2);
#ifdef INRC2_USE_TABU
                if (noExchangeTabu(move)) {
#endif
                    if (rs.isMinimal(move.delta, bestMove.delta,
                                     solver.randGen)) {
                        bestMove = move;
                    }
#ifdef INRC2_USE_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(move.delta, bestMove_tabu.delta,
                                          solver.randGen)) {
                        bestMove_tabu = move;
                    }
                }
#endif
            }
            move.weekday = (move.weekday != c.dayHigh[move.weekday])
                               ? c.dayHigh[move.weekday]
                               : (move.weekday + 1);
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    penalty.recoverLastMode();
    return (bestMove.delta < 0);
}

bool NurseRostering::Solution::findBestARLoopOnBlockBorder(
    Move &bestMove) const {
    bool isImproved;
    isImproved = findBestARLoopOnBlockBorder_flag
                     ? findBestAddOnBlockBorder(bestMove)
                     : findBestRemove(bestMove);

    if (!isImproved) {
        findBestARLoopOnBlockBorder_flag = !findBestARLoopOnBlockBorder_flag;
    }

    return isImproved;
}

bool NurseRostering::Solution::findBestARRandOnBlockBorder(
    Move &bestMove) const {
    return ((solver.randGen() % 2) ? findBestAddOnBlockBorder(bestMove)
                                   : findBestRemoveOnBlockBorder(bestMove));
}

bool NurseRostering::Solution::findBestARBothOnBlockBorder(
    Move &bestMove) const {
    RandSelect<ObjValue> rs;
#ifdef INRC2_USE_TABU
    Move bestMove_tabu;
    RandSelect<ObjValue> rs_tabu;
#endif

    Move move;
    for (move.nurse = 0; move.nurse < problem.scenario.nurseNum; ++move.nurse) {
        const Consecutive &c(consecutives[move.nurse]);
        for (move.weekday = Weekday::Mon; move.weekday <= Weekday::Sun;) {
            if (assign.isWorking(move.nurse, move.weekday)) {
                move.delta = tryRemoveAssign(move);
#ifdef INRC2_USE_TABU
                if (noRemoveTabu(move)) {
#endif
                    if (rs.isMinimal(move.delta, bestMove.delta,
                                     solver.randGen)) {
                        bestMove = move;
                        bestMove.mode = Move::Mode::Remove;
                    }
#ifdef INRC2_USE_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(move.delta, bestMove_tabu.delta,
                                          solver.randGen)) {
                        bestMove_tabu = move;
                        bestMove_tabu.mode = Move::Mode::Remove;
                    }
                }
#endif
            } else {
                for (move.assign.shift =
                         NurseRostering::Scenario::Shift::ID_BEGIN;
                     move.assign.shift < problem.scenario.shiftSize;
                     ++move.assign.shift) {
                    for (move.assign.skill =
                             NurseRostering::Scenario::Skill::ID_BEGIN;
                         move.assign.skill < problem.scenario.skillSize;
                         ++move.assign.skill) {
                        move.delta = tryAddAssign(move);
#ifdef INRC2_USE_TABU
                        if (noAddTabu(move)) {
#endif
                            if (rs.isMinimal(move.delta, bestMove.delta,
                                             solver.randGen)) {
                                bestMove = move;
                                bestMove.mode = Move::Mode::Add;
                            }
#ifdef INRC2_USE_TABU
                        } else {  // tabu
                            if (rs_tabu.isMinimal(move.delta,
                                                  bestMove_tabu.delta,
                                                  solver.randGen)) {
                                bestMove_tabu = move;
                                bestMove_tabu.mode = Move::Mode::Add;
                            }
                        }
#endif
                    }
                }
            }
            move.weekday = (move.weekday != c.dayHigh[move.weekday])
                               ? c.dayHigh[move.weekday]
                               : (move.weekday + 1);
        }
    }

#ifdef INRC2_USE_TABU
    if (aspirationCritiera(bestMove.delta, bestMove_tabu.delta)) {
        bestMove = bestMove_tabu;
    }
#endif

    return (bestMove.delta < 0);
}

NurseRostering::ObjValue NurseRostering::Solution::tryAddAssign(
    int weekday, NurseID nurse, const Assign &a) const {
    ObjValue delta = 0;

    // TODO : make sure they won't be the same and leave out this
    if (!a.isWorking() || assign.isWorking(nurse, weekday)) {
        return DefaultPenalty::FORBIDDEN_MOVE;
    }

    // hard constraint check
    delta +=
        penalty.MissSkill() * (!problem.scenario.nurses[nurse].skills[a.skill]);

    delta +=
        penalty.Succession() * (!isValidSuccession(nurse, a.shift, weekday));
    delta += penalty.Succession() * (!isValidPrior(nurse, a.shift, weekday));

    if (delta >= DefaultPenalty::MAX_OBJ_VALUE) {
        return delta;
    }

    const WeekData &weekData(problem.weekData);
    delta -= penalty.UnderStaff() *
             (weekData.minNurseNums[weekday][a.shift][a.skill] >
              (weekData.optNurseNums[weekday][a.shift][a.skill] -
               missingNurseNums[weekday][a.shift][a.skill]));

    int prevDay = weekday - 1;
    int nextDay = weekday + 1;
    ContractID contractID = problem.scenario.nurses[nurse].contract;
    const Scenario::Contract &contract(problem.scenario.contracts[contractID]);
    const Consecutive &c(consecutives[nurse]);

    // insufficient staff
    delta -= penalty.InsufficientStaff() *
             (missingNurseNums[weekday][a.shift][a.skill] > 0);

    if (nurseWeights[nurse] == 0) {
        return delta;  // TODO : weight ?
    }

    // consecutive shift
    const vector<Scenario::Shift> &shifts(problem.scenario.shifts);
    const Scenario::Shift &shift(shifts[a.shift]);
    ShiftID prevShiftID = assign[nurse][prevDay].shift;
    if (weekday == Weekday::Sun) {  // there is no blocks on the right
        // shiftHigh[weekday] will always be equal to Weekday::Sun
        if ((Weekday::Sun == c.shiftLow[weekday]) && (a.shift == prevShiftID)) {
            const Scenario::Shift &prevShift(shifts[prevShiftID]);
            delta -= penalty.ConsecutiveShift() *
                     distanceToRange(Weekday::Sun - c.shiftLow[Weekday::Sat],
                                     prevShift.minConsecutiveShiftNum,
                                     prevShift.maxConsecutiveShiftNum);
            delta += penalty.ConsecutiveShift() *
                     exceedCount(Weekday::Sun - c.shiftLow[Weekday::Sat] + 1,
                                 shift.maxConsecutiveShiftNum);
        } else {  // have nothing to do with previous block
            delta += penalty.ConsecutiveShift() *  // penalty on day off is
                                                   // counted later
                     exceedCount(1, shift.maxConsecutiveShiftNum);
        }
    } else {
        ShiftID nextShiftID = assign[nurse][nextDay].shift;
        if (c.shiftHigh[weekday] == c.shiftLow[weekday]) {
            int high = weekday;
            int low = weekday;
            if (prevShiftID == a.shift) {
                const Scenario::Shift &prevShift(shifts[prevShiftID]);
                low = c.shiftLow[prevDay];
                delta -= penalty.ConsecutiveShift() *
                         distanceToRange(weekday - c.shiftLow[prevDay],
                                         prevShift.minConsecutiveShiftNum,
                                         prevShift.maxConsecutiveShiftNum);
            }
            if (nextShiftID == a.shift) {
                const Scenario::Shift &nextShift(shifts[nextShiftID]);
                high = c.shiftHigh[nextDay];
                delta -= penalty.ConsecutiveShift() *
                         penaltyDayNum(c.shiftHigh[nextDay] - weekday,
                                       c.shiftHigh[nextDay],
                                       nextShift.minConsecutiveShiftNum,
                                       nextShift.maxConsecutiveShiftNum);
            }
            delta += penalty.ConsecutiveShift() *
                     penaltyDayNum(high - low + 1, high,
                                   shift.minConsecutiveShiftNum,
                                   shift.maxConsecutiveShiftNum);
        } else if (weekday == c.shiftHigh[weekday]) {
            if (a.shift == nextShiftID) {
                const Scenario::Shift &nextShift(shifts[nextShiftID]);
                int consecutiveShiftOfNextBlock =
                    c.shiftHigh[nextDay] - weekday;
                if (consecutiveShiftOfNextBlock >=
                    nextShift.maxConsecutiveShiftNum) {
                    delta += penalty.ConsecutiveShift();
                } else if ((c.shiftHigh[nextDay] < Weekday::Sun) &&
                           (consecutiveShiftOfNextBlock <
                            nextShift.minConsecutiveShiftNum)) {
                    delta -= penalty.ConsecutiveShift();
                }
            } else {
                delta += penalty.ConsecutiveShift() *
                         distanceToRange(1, shift.minConsecutiveShiftNum,
                                         shift.maxConsecutiveShiftNum);
            }
        } else if (weekday == c.shiftLow[weekday]) {
            if (a.shift == prevShiftID) {
                const Scenario::Shift &prevShift(shifts[prevShiftID]);
                int consecutiveShiftOfPrevBlock = weekday - c.shiftLow[prevDay];
                if (consecutiveShiftOfPrevBlock >=
                    prevShift.maxConsecutiveShiftNum) {
                    delta += penalty.ConsecutiveShift();
                } else if (consecutiveShiftOfPrevBlock <
                           prevShift.minConsecutiveShiftNum) {
                    delta -= penalty.ConsecutiveShift();
                }
            } else {
                delta += penalty.ConsecutiveShift() *
                         distanceToRange(1, shift.minConsecutiveShiftNum,
                                         shift.maxConsecutiveShiftNum);
            }
        } else {
            delta += penalty.ConsecutiveShift() *
                     distanceToRange(1, shift.minConsecutiveShiftNum,
                                     shift.maxConsecutiveShiftNum);
        }
    }

    // consecutive day and day-off
    if (weekday == Weekday::Sun) {  // there is no block on the right
        // dayHigh[weekday] will always be equal to Weekday::Sun
        if (Weekday::Sun == c.dayLow[weekday]) {
            delta -= penalty.ConsecutiveDay() *
                     distanceToRange(Weekday::Sun - c.dayLow[Weekday::Sat],
                                     contract.minConsecutiveDayNum,
                                     contract.maxConsecutiveDayNum);
            delta -= penalty.ConsecutiveDayOff() *
                     exceedCount(1, contract.maxConsecutiveDayoffNum);
            delta += penalty.ConsecutiveDay() *
                     exceedCount(Weekday::Sun - c.dayLow[Weekday::Sat] + 1,
                                 contract.maxConsecutiveDayNum);
        } else {  // day off block length over 1
            delta -= penalty.ConsecutiveDayOff() *
                     exceedCount(Weekday::Sun - c.dayLow[Weekday::Sun] + 1,
                                 contract.maxConsecutiveDayoffNum);
            delta += penalty.ConsecutiveDayOff() *
                     distanceToRange(Weekday::Sun - c.dayLow[Weekday::Sun],
                                     contract.minConsecutiveDayoffNum,
                                     contract.maxConsecutiveDayoffNum);
            delta += penalty.ConsecutiveDay() *
                     exceedCount(1, contract.maxConsecutiveDayNum);
        }
    } else {
        if (c.dayHigh[weekday] == c.dayLow[weekday]) {
            delta -= penalty.ConsecutiveDay() *
                     distanceToRange(weekday - c.dayLow[prevDay],
                                     contract.minConsecutiveDayNum,
                                     contract.maxConsecutiveDayNum);
            delta -= penalty.ConsecutiveDayOff() *
                     distanceToRange(1, contract.minConsecutiveDayoffNum,
                                     contract.maxConsecutiveDayoffNum);
            delta -=
                penalty.ConsecutiveDay() *
                penaltyDayNum(c.dayHigh[nextDay] - weekday, c.dayHigh[nextDay],
                              contract.minConsecutiveDayNum,
                              contract.maxConsecutiveDayNum);
            delta +=
                penalty.ConsecutiveDay() *
                penaltyDayNum(c.dayHigh[nextDay] - c.dayLow[prevDay] + 1,
                              c.dayHigh[nextDay], contract.minConsecutiveDayNum,
                              contract.maxConsecutiveDayNum);
        } else if (weekday == c.dayHigh[weekday]) {
            int consecutiveDayOfNextBlock = c.dayHigh[nextDay] - weekday;
            if (consecutiveDayOfNextBlock >= contract.maxConsecutiveDayNum) {
                delta += penalty.ConsecutiveDay();
            } else if ((c.dayHigh[nextDay] < Weekday::Sun) &&
                       (consecutiveDayOfNextBlock <
                        contract.minConsecutiveDayNum)) {
                delta -= penalty.ConsecutiveDay();
            }
            int consecutiveDayOfThisBlock = weekday - c.dayLow[weekday] + 1;
            if (consecutiveDayOfThisBlock > contract.maxConsecutiveDayoffNum) {
                delta -= penalty.ConsecutiveDayOff();
            } else if (consecutiveDayOfThisBlock <=
                       contract.minConsecutiveDayoffNum) {
                delta += penalty.ConsecutiveDayOff();
            }
        } else if (weekday == c.dayLow[weekday]) {
            int consecutiveDayOfPrevBlock = weekday - c.dayLow[prevDay];
            if (consecutiveDayOfPrevBlock >= contract.maxConsecutiveDayNum) {
                delta += penalty.ConsecutiveDay();
            } else if (consecutiveDayOfPrevBlock <
                       contract.minConsecutiveDayNum) {
                delta -= penalty.ConsecutiveDay();
            }
            int consecutiveDayOfThisBlock = c.dayHigh[weekday] - weekday + 1;
            if (consecutiveDayOfThisBlock > contract.maxConsecutiveDayoffNum) {
                delta -= penalty.ConsecutiveDayOff();
            } else if ((c.dayHigh[weekday] < Weekday::Sun) &&
                       (consecutiveDayOfThisBlock <=
                        contract.minConsecutiveDayoffNum)) {
                delta += penalty.ConsecutiveDayOff();
            }
        } else {
            delta -= penalty.ConsecutiveDayOff() *
                     penaltyDayNum(c.dayHigh[weekday] - c.dayLow[weekday] + 1,
                                   c.dayHigh[weekday],
                                   contract.minConsecutiveDayoffNum,
                                   contract.maxConsecutiveDayoffNum);
            delta += penalty.ConsecutiveDayOff() *
                     distanceToRange(weekday - c.dayLow[weekday],
                                     contract.minConsecutiveDayoffNum,
                                     contract.maxConsecutiveDayoffNum);
            delta += penalty.ConsecutiveDay() *
                     distanceToRange(1, contract.minConsecutiveDayNum,
                                     contract.maxConsecutiveDayNum);
            delta +=
                penalty.ConsecutiveDayOff() *
                penaltyDayNum(c.dayHigh[weekday] - weekday, c.dayHigh[weekday],
                              contract.minConsecutiveDayoffNum,
                              contract.maxConsecutiveDayoffNum);
        }
    }

    // preference
    delta += penalty.Preference() * weekData.shiftOffs[weekday][a.shift][nurse];

    int currentWeek = problem.history.currentWeek;
    if (weekday > Weekday::Fri) {
        int theOtherDay = Weekday::Sat + (weekday == Weekday::Sat);
        // complete weekend
        if (contract.completeWeekend) {
            if (assign.isWorking(nurse, theOtherDay)) {
                delta -= penalty.CompleteWeekend();
            } else {
                delta += penalty.CompleteWeekend();
            }
        }

        // total working weekend
        if (!assign.isWorking(nurse, theOtherDay)) {
#ifdef INRC2_AVERAGE_MAX_WORKING_WEEKEND
            const History &history(problem.history);
            delta -= penalty.TotalWorkingWeekend() *
                     exceedCount(history.totalWorkingWeekendNums[nurse] *
                                     problem.scenario.totalWeekNum,
                                 contract.maxWorkingWeekendNum * currentWeek) /
                     problem.scenario.totalWeekNum;
            delta += penalty.TotalWorkingWeekend() *
                     exceedCount((history.totalWorkingWeekendNums[nurse] + 1) *
                                     problem.scenario.totalWeekNum,
                                 contract.maxWorkingWeekendNum * currentWeek) /
                     problem.scenario.totalWeekNum;
#else
            delta -=
                penalty.TotalWorkingWeekend() *
                exceedCount(
                    0,
                    problem.scenario.nurses[nurse].restMaxWorkingWeekendNum) /
                problem.history.restWeekCount;
            delta +=
                penalty.TotalWorkingWeekend() *
                exceedCount(
                    problem.history.restWeekCount,
                    problem.scenario.nurses[nurse].restMaxWorkingWeekendNum) /
                problem.history.restWeekCount;
#endif
        }
    }

    // total assign (expand problem.history.restWeekCount times)
#ifdef INRC2_AVERAGE_TOTAL_SHIFT_NUM
    int totalAssign =
        (totalAssignNums[nurse] + problem.history.totalAssignNums[nurse]) *
        problem.scenario.totalWeekNum;
    delta -= penalty.TotalAssign() *
             distanceToRange(totalAssign, contract.minShiftNum * currentWeek,
                             contract.maxShiftNum * currentWeek) /
             problem.scenario.totalWeekNum;
    delta += penalty.TotalAssign() *
             distanceToRange(totalAssign + problem.scenario.totalWeekNum,
                             contract.minShiftNum * currentWeek,
                             contract.maxShiftNum * currentWeek) /
             problem.scenario.totalWeekNum;
#else
    int restMinShift = problem.scenario.nurses[nurse].restMinShiftNum;
    int restMaxShift = problem.scenario.nurses[nurse].restMaxShiftNum;
    int totalAssign = totalAssignNums[nurse] * problem.history.restWeekCount;
    delta -= penalty.TotalAssign() *
             distanceToRange(totalAssign, restMinShift, restMaxShift) /
             problem.history.restWeekCount;
    delta += penalty.TotalAssign() *
             distanceToRange(totalAssign + problem.history.restWeekCount,
                             restMinShift, restMaxShift) /
             problem.history.restWeekCount;
#endif

    return delta;  // TODO : weight ?
}

NurseRostering::ObjValue NurseRostering::Solution::tryAddAssign(
    const Move &move) const {
    return tryAddAssign(move.weekday, move.nurse, move.assign);
}

NurseRostering::ObjValue NurseRostering::Solution::tryChangeAssign(
    int weekday, NurseID nurse, const Assign &a) const {
    ObjValue delta = 0;

    ShiftID oldShiftID = assign[nurse][weekday].shift;
    SkillID oldSkillID = assign[nurse][weekday].skill;
    // TODO : make sure they won't be the same and leave out this
    if (!a.isWorking() || !Assign::isWorking(oldShiftID) ||
        ((a.shift == oldShiftID) && (a.skill == oldSkillID))) {
        return DefaultPenalty::FORBIDDEN_MOVE;
    }

    delta +=
        penalty.MissSkill() * (!problem.scenario.nurses[nurse].skills[a.skill]);

    delta +=
        penalty.Succession() * (!isValidSuccession(nurse, a.shift, weekday));
    delta += penalty.Succession() * (!isValidPrior(nurse, a.shift, weekday));

    const WeekData &weekData(problem.weekData);
    delta += penalty.UnderStaff() *
             (weekData.minNurseNums[weekday][oldShiftID][oldSkillID] >=
              (weekData.optNurseNums[weekday][oldShiftID][oldSkillID] -
               missingNurseNums[weekday][oldShiftID][oldSkillID]));

    if (delta >= DefaultPenalty::MAX_OBJ_VALUE) {
        return delta;
    }

    delta -=
        penalty.Succession() * (!isValidSuccession(nurse, oldShiftID, weekday));
    delta -= penalty.Succession() * (!isValidPrior(nurse, oldShiftID, weekday));

    delta -= penalty.UnderStaff() *
             (weekData.minNurseNums[weekday][a.shift][a.skill] >
              (weekData.optNurseNums[weekday][a.shift][a.skill] -
               missingNurseNums[weekday][a.shift][a.skill]));

    int prevDay = weekday - 1;
    int nextDay = weekday + 1;
    const Consecutive &c(consecutives[nurse]);

    // insufficient staff
    delta += penalty.InsufficientStaff() *
             (missingNurseNums[weekday][oldShiftID][oldSkillID] >= 0);
    delta -= penalty.InsufficientStaff() *
             (missingNurseNums[weekday][a.shift][a.skill] > 0);

    if (nurseWeights[nurse] == 0) {
        return delta;  // TODO : weight ?
    }

    if (a.shift != oldShiftID) {
        // consecutive shift
        const vector<Scenario::Shift> &shifts(problem.scenario.shifts);
        const Scenario::Shift &shift(shifts[a.shift]);
        const Scenario::Shift &oldShift(shifts[oldShiftID]);
        ShiftID prevShiftID = assign[nurse][prevDay].shift;
        if (weekday == Weekday::Sun) {  // there is no blocks on the right
            // shiftHigh[weekday] will always equal to Weekday::Sun
            if (Weekday::Sun == c.shiftLow[weekday]) {
                if (a.shift == prevShiftID) {
                    const Scenario::Shift &prevShift(shifts[prevShiftID]);
                    delta -=
                        penalty.ConsecutiveShift() *
                        distanceToRange(Weekday::Sun - c.shiftLow[Weekday::Sat],
                                        prevShift.minConsecutiveShiftNum,
                                        prevShift.maxConsecutiveShiftNum);
                    delta -= penalty.ConsecutiveShift() *
                             exceedCount(1, oldShift.maxConsecutiveShiftNum);
                    delta +=
                        penalty.ConsecutiveShift() *
                        exceedCount(Weekday::Sun - c.shiftLow[Weekday::Sat] + 1,
                                    shift.maxConsecutiveShiftNum);
                } else {
                    delta -= penalty.ConsecutiveShift() *
                             exceedCount(1, oldShift.maxConsecutiveShiftNum);
                    delta += penalty.ConsecutiveShift() *
                             exceedCount(1, shift.maxConsecutiveShiftNum);
                }
            } else {  // block length over 1
                delta -=
                    penalty.ConsecutiveShift() *
                    exceedCount(Weekday::Sun - c.shiftLow[Weekday::Sun] + 1,
                                oldShift.maxConsecutiveShiftNum);
                delta +=
                    penalty.ConsecutiveShift() *
                    distanceToRange(Weekday::Sun - c.shiftLow[Weekday::Sun],
                                    oldShift.minConsecutiveShiftNum,
                                    oldShift.maxConsecutiveShiftNum);
                delta += penalty.ConsecutiveShift() *
                         exceedCount(1, shift.maxConsecutiveShiftNum);
            }
        } else {
            ShiftID nextShiftID = assign[nurse][nextDay].shift;
            if (c.shiftHigh[weekday] == c.shiftLow[weekday]) {
                int high = weekday;
                int low = weekday;
                if (prevShiftID == a.shift) {
                    const Scenario::Shift &prevShift(shifts[prevShiftID]);
                    low = c.shiftLow[prevDay];
                    delta -= penalty.ConsecutiveShift() *
                             distanceToRange(weekday - c.shiftLow[prevDay],
                                             prevShift.minConsecutiveShiftNum,
                                             prevShift.maxConsecutiveShiftNum);
                }
                if (nextShiftID == a.shift) {
                    const Scenario::Shift &nextShift(shifts[nextShiftID]);
                    high = c.shiftHigh[nextDay];
                    delta -= penalty.ConsecutiveShift() *
                             penaltyDayNum(c.shiftHigh[nextDay] - weekday,
                                           c.shiftHigh[nextDay],
                                           nextShift.minConsecutiveShiftNum,
                                           nextShift.maxConsecutiveShiftNum);
                }
                delta -= penalty.ConsecutiveShift() *
                         distanceToRange(1, oldShift.minConsecutiveShiftNum,
                                         oldShift.maxConsecutiveShiftNum);
                delta += penalty.ConsecutiveShift() *
                         penaltyDayNum(high - low + 1, high,
                                       shift.minConsecutiveShiftNum,
                                       shift.maxConsecutiveShiftNum);
            } else if (weekday == c.shiftHigh[weekday]) {
                if (nextShiftID == a.shift) {
                    const Scenario::Shift &nextShift(shifts[nextShiftID]);
                    int consecutiveShiftOfNextBlock =
                        c.shiftHigh[nextDay] - weekday;
                    if (consecutiveShiftOfNextBlock >=
                        nextShift.maxConsecutiveShiftNum) {
                        delta += penalty.ConsecutiveShift();
                    } else if ((c.shiftHigh[nextDay] < Weekday::Sun) &&
                               (consecutiveShiftOfNextBlock <
                                nextShift.minConsecutiveShiftNum)) {
                        delta -= penalty.ConsecutiveShift();
                    }
                } else {
                    delta += penalty.ConsecutiveShift() *
                             distanceToRange(1, shift.minConsecutiveShiftNum,
                                             shift.maxConsecutiveShiftNum);
                }
                int consecutiveShiftOfThisBlock =
                    weekday - c.shiftLow[weekday] + 1;
                if (consecutiveShiftOfThisBlock >
                    oldShift.maxConsecutiveShiftNum) {
                    delta -= penalty.ConsecutiveShift();
                } else if (consecutiveShiftOfThisBlock <=
                           oldShift.minConsecutiveShiftNum) {
                    delta += penalty.ConsecutiveShift();
                }
            } else if (weekday == c.shiftLow[weekday]) {
                if (prevShiftID == a.shift) {
                    const Scenario::Shift &prevShift(shifts[prevShiftID]);
                    int consecutiveShiftOfPrevBlock =
                        weekday - c.shiftLow[prevDay];
                    if (consecutiveShiftOfPrevBlock >=
                        prevShift.maxConsecutiveShiftNum) {
                        delta += penalty.ConsecutiveShift();
                    } else if (consecutiveShiftOfPrevBlock <
                               prevShift.minConsecutiveShiftNum) {
                        delta -= penalty.ConsecutiveShift();
                    }
                } else {
                    delta += penalty.ConsecutiveShift() *
                             distanceToRange(1, shift.minConsecutiveShiftNum,
                                             shift.maxConsecutiveShiftNum);
                }
                int consecutiveShiftOfThisBlock =
                    c.shiftHigh[weekday] - weekday + 1;
                if (consecutiveShiftOfThisBlock >
                    oldShift.maxConsecutiveShiftNum) {
                    delta -= penalty.ConsecutiveShift();
                } else if ((c.shiftHigh[weekday] < Weekday::Sun) &&
                           (consecutiveShiftOfThisBlock <=
                            oldShift.minConsecutiveShiftNum)) {
                    delta += penalty.ConsecutiveShift();
                }
            } else {
                delta -=
                    penalty.ConsecutiveShift() *
                    penaltyDayNum(
                        c.shiftHigh[weekday] - c.shiftLow[weekday] + 1,
                        c.shiftHigh[weekday], oldShift.minConsecutiveShiftNum,
                        oldShift.maxConsecutiveShiftNum);
                delta += penalty.ConsecutiveShift() *
                         distanceToRange(weekday - c.shiftLow[weekday],
                                         oldShift.minConsecutiveShiftNum,
                                         oldShift.maxConsecutiveShiftNum);
                delta += penalty.ConsecutiveShift() *
                         distanceToRange(1, shift.minConsecutiveShiftNum,
                                         shift.maxConsecutiveShiftNum);
                delta += penalty.ConsecutiveShift() *
                         penaltyDayNum(c.shiftHigh[weekday] - weekday,
                                       c.shiftHigh[weekday],
                                       oldShift.minConsecutiveShiftNum,
                                       oldShift.maxConsecutiveShiftNum);
            }
        }

        // preference
        delta +=
            penalty.Preference() * weekData.shiftOffs[weekday][a.shift][nurse];
        delta -= penalty.Preference() *
                 weekData.shiftOffs[weekday][oldShiftID][nurse];
    }

    return delta;  // TODO : weight ?
}

NurseRostering::ObjValue NurseRostering::Solution::tryChangeAssign(
    const Move &move) const {
    return tryChangeAssign(move.weekday, move.nurse, move.assign);
}

NurseRostering::ObjValue NurseRostering::Solution::tryRemoveAssign(
    int weekday, NurseID nurse) const {
    ObjValue delta = 0;

    ShiftID oldShiftID = assign[nurse][weekday].shift;
    SkillID oldSkillID = assign[nurse][weekday].skill;
    // TODO : make sure they won't be the same and leave out this
    if (!Assign::isWorking(oldShiftID)) {
        return DefaultPenalty::FORBIDDEN_MOVE;
    }

    const WeekData &weekData(problem.weekData);
    delta += penalty.UnderStaff() *
             (weekData.minNurseNums[weekday][oldShiftID][oldSkillID] >=
              (weekData.optNurseNums[weekday][oldShiftID][oldSkillID] -
               missingNurseNums[weekday][oldShiftID][oldSkillID]));

    if (delta >= DefaultPenalty::MAX_OBJ_VALUE) {
        return delta;
    }

    delta -=
        penalty.Succession() * (!isValidSuccession(nurse, oldShiftID, weekday));
    delta -= penalty.Succession() * (!isValidPrior(nurse, oldShiftID, weekday));

    int prevDay = weekday - 1;
    int nextDay = weekday + 1;
    ContractID contractID = problem.scenario.nurses[nurse].contract;
    const Scenario::Contract &contract(problem.scenario.contracts[contractID]);
    const Consecutive &c(consecutives[nurse]);

    // insufficient staff
    delta += penalty.InsufficientStaff() *
             (missingNurseNums[weekday][oldShiftID][oldSkillID] >= 0);

    if (nurseWeights[nurse] == 0) {
        return delta;  // TODO : weight ?
    }

    // consecutive shift
    const vector<Scenario::Shift> &shifts(problem.scenario.shifts);
    const Scenario::Shift &oldShift(shifts[oldShiftID]);
    if (weekday == Weekday::Sun) {  // there is no block on the right
        if (Weekday::Sun == c.shiftLow[weekday]) {
            delta -= penalty.ConsecutiveShift() *
                     exceedCount(1, oldShift.maxConsecutiveShiftNum);
        } else {
            delta -= penalty.ConsecutiveShift() *
                     exceedCount(Weekday::Sun - c.shiftLow[weekday] + 1,
                                 oldShift.maxConsecutiveShiftNum);
            delta += penalty.ConsecutiveShift() *
                     distanceToRange(Weekday::Sun - c.shiftLow[weekday],
                                     oldShift.minConsecutiveShiftNum,
                                     oldShift.maxConsecutiveShiftNum);
        }
    } else {
        if (c.shiftHigh[weekday] == c.shiftLow[weekday]) {
            delta -= penalty.ConsecutiveShift() *
                     distanceToRange(1, oldShift.minConsecutiveShiftNum,
                                     oldShift.maxConsecutiveShiftNum);
        } else if (weekday == c.shiftHigh[weekday]) {
            int consecutiveShiftOfThisBlock = weekday - c.shiftLow[weekday] + 1;
            if (consecutiveShiftOfThisBlock > oldShift.maxConsecutiveShiftNum) {
                delta -= penalty.ConsecutiveShift();
            } else if (consecutiveShiftOfThisBlock <=
                       oldShift.minConsecutiveShiftNum) {
                delta += penalty.ConsecutiveShift();
            }
        } else if (weekday == c.shiftLow[weekday]) {
            int consecutiveShiftOfThisBlock =
                c.shiftHigh[weekday] - weekday + 1;
            if (consecutiveShiftOfThisBlock > oldShift.maxConsecutiveShiftNum) {
                delta -= penalty.ConsecutiveShift();
            } else if ((c.shiftHigh[weekday] < Weekday::Sun) &&
                       (consecutiveShiftOfThisBlock <=
                        oldShift.minConsecutiveShiftNum)) {
                delta += penalty.ConsecutiveShift();
            }
        } else {
            delta -= penalty.ConsecutiveShift() *
                     penaltyDayNum(
                         c.shiftHigh[weekday] - c.shiftLow[weekday] + 1,
                         c.shiftHigh[weekday], oldShift.minConsecutiveShiftNum,
                         oldShift.maxConsecutiveShiftNum);
            delta += penalty.ConsecutiveShift() *
                     distanceToRange(weekday - c.shiftLow[weekday],
                                     oldShift.minConsecutiveShiftNum,
                                     oldShift.maxConsecutiveShiftNum);
            delta += penalty.ConsecutiveShift() *
                     penaltyDayNum(c.shiftHigh[weekday] - weekday,
                                   c.shiftHigh[weekday],
                                   oldShift.minConsecutiveShiftNum,
                                   oldShift.maxConsecutiveShiftNum);
        }
    }

    // consecutive day and day-off
    if (weekday == Weekday::Sun) {  // there is no blocks on the right
        // dayHigh[weekday] will always be equal to Weekday::Sun
        if (Weekday::Sun == c.dayLow[weekday]) {
            delta -= penalty.ConsecutiveDayOff() *
                     distanceToRange(Weekday::Sun - c.dayLow[Weekday::Sat],
                                     contract.minConsecutiveDayoffNum,
                                     contract.maxConsecutiveDayoffNum);
            delta -= penalty.ConsecutiveDay() *
                     exceedCount(1, contract.maxConsecutiveDayNum);
            delta += penalty.ConsecutiveDayOff() *
                     exceedCount(Weekday::Sun - c.dayLow[Weekday::Sat] + 1,
                                 contract.maxConsecutiveDayoffNum);
        } else {  // day off block length over 1
            delta -= penalty.ConsecutiveDay() *
                     exceedCount(Weekday::Sun - c.dayLow[Weekday::Sun] + 1,
                                 contract.maxConsecutiveDayNum);
            delta += penalty.ConsecutiveDay() *
                     distanceToRange(Weekday::Sun - c.dayLow[Weekday::Sun],
                                     contract.minConsecutiveDayNum,
                                     contract.maxConsecutiveDayNum);
            delta += penalty.ConsecutiveDayOff() *
                     exceedCount(1, contract.maxConsecutiveDayoffNum);
        }
    } else {
        if (c.dayHigh[weekday] == c.dayLow[weekday]) {
            delta -= penalty.ConsecutiveDayOff() *
                     distanceToRange(weekday - c.dayLow[prevDay],
                                     contract.minConsecutiveDayoffNum,
                                     contract.maxConsecutiveDayoffNum);
            delta -= penalty.ConsecutiveDay() *
                     distanceToRange(1, contract.minConsecutiveDayNum,
                                     contract.maxConsecutiveDayNum);
            delta -=
                penalty.ConsecutiveDayOff() *
                penaltyDayNum(c.dayHigh[nextDay] - weekday, c.dayHigh[nextDay],
                              contract.minConsecutiveDayoffNum,
                              contract.maxConsecutiveDayoffNum);
            delta += penalty.ConsecutiveDayOff() *
                     penaltyDayNum(c.dayHigh[nextDay] - c.dayLow[prevDay] + 1,
                                   c.dayHigh[nextDay],
                                   contract.minConsecutiveDayoffNum,
                                   contract.maxConsecutiveDayoffNum);
        } else if (weekday == c.dayHigh[weekday]) {
            int consecutiveDayOfNextBlock = c.dayHigh[nextDay] - weekday;
            if (consecutiveDayOfNextBlock >= contract.maxConsecutiveDayoffNum) {
                delta += penalty.ConsecutiveDayOff();
            } else if ((c.dayHigh[nextDay] < Weekday::Sun) &&
                       (consecutiveDayOfNextBlock <
                        contract.minConsecutiveDayoffNum)) {
                delta -= penalty.ConsecutiveDayOff();
            }
            int consecutiveDayOfThisBlock = weekday - c.dayLow[weekday] + 1;
            if (consecutiveDayOfThisBlock > contract.maxConsecutiveDayNum) {
                delta -= penalty.ConsecutiveDay();
            } else if (consecutiveDayOfThisBlock <=
                       contract.minConsecutiveDayNum) {
                delta += penalty.ConsecutiveDay();
            }
        } else if (weekday == c.dayLow[weekday]) {
            int consecutiveDayOfPrevBlock = weekday - c.dayLow[prevDay];
            if (consecutiveDayOfPrevBlock >= contract.maxConsecutiveDayoffNum) {
                delta += penalty.ConsecutiveDayOff();
            } else if (consecutiveDayOfPrevBlock <
                       contract.minConsecutiveDayoffNum) {
                delta -= penalty.ConsecutiveDayOff();
            }
            int consecutiveDayOfThisBlock = c.dayHigh[weekday] - weekday + 1;
            if (consecutiveDayOfThisBlock > contract.maxConsecutiveDayNum) {
                delta -= penalty.ConsecutiveDay();
            } else if ((c.dayHigh[weekday] < Weekday::Sun) &&
                       (consecutiveDayOfThisBlock <=
                        contract.minConsecutiveDayNum)) {
                delta += penalty.ConsecutiveDay();
            }
        } else {
            delta -=
                penalty.ConsecutiveDay() *
                penaltyDayNum(c.dayHigh[weekday] - c.dayLow[weekday] + 1,
                              c.dayHigh[weekday], contract.minConsecutiveDayNum,
                              contract.maxConsecutiveDayNum);
            delta += penalty.ConsecutiveDay() *
                     distanceToRange(weekday - c.dayLow[weekday],
                                     contract.minConsecutiveDayNum,
                                     contract.maxConsecutiveDayNum);
            delta += penalty.ConsecutiveDayOff() *
                     distanceToRange(1, contract.minConsecutiveDayoffNum,
                                     contract.maxConsecutiveDayoffNum);
            delta +=
                penalty.ConsecutiveDay() *
                penaltyDayNum(c.dayHigh[weekday] - weekday, c.dayHigh[weekday],
                              contract.minConsecutiveDayNum,
                              contract.maxConsecutiveDayNum);
        }
    }

    // preference
    delta -=
        penalty.Preference() * weekData.shiftOffs[weekday][oldShiftID][nurse];

    int currentWeek = problem.history.currentWeek;
    if (weekday > Weekday::Fri) {
        int theOtherDay = Weekday::Sat + (weekday == Weekday::Sat);
        // complete weekend
        if (contract.completeWeekend) {
            if (assign.isWorking(nurse, theOtherDay)) {
                delta += penalty.CompleteWeekend();
            } else {
                delta -= penalty.CompleteWeekend();
            }
        }

        // total working weekend
        if (!assign.isWorking(nurse, theOtherDay)) {
#ifdef INRC2_AVERAGE_MAX_WORKING_WEEKEND
            const History &history(problem.history);
            delta -= penalty.TotalWorkingWeekend() *
                     exceedCount((history.totalWorkingWeekendNums[nurse] + 1) *
                                     problem.scenario.totalWeekNum,
                                 contract.maxWorkingWeekendNum * currentWeek) /
                     problem.scenario.totalWeekNum;
            delta += penalty.TotalWorkingWeekend() *
                     exceedCount(history.totalWorkingWeekendNums[nurse] *
                                     problem.scenario.totalWeekNum,
                                 contract.maxWorkingWeekendNum * currentWeek) /
                     problem.scenario.totalWeekNum;
#else
            delta -=
                penalty.TotalWorkingWeekend() *
                exceedCount(
                    problem.history.restWeekCount,
                    problem.scenario.nurses[nurse].restMaxWorkingWeekendNum) /
                problem.history.restWeekCount;
            delta +=
                penalty.TotalWorkingWeekend() *
                exceedCount(
                    0,
                    problem.scenario.nurses[nurse].restMaxWorkingWeekendNum) /
                problem.history.restWeekCount;
#endif
        }
    }

    // total assign (expand problem.history.restWeekCount times)
#ifdef INRC2_AVERAGE_TOTAL_SHIFT_NUM
    int totalAssign =
        (totalAssignNums[nurse] + problem.history.totalAssignNums[nurse]) *
        problem.scenario.totalWeekNum;
    delta -= penalty.TotalAssign() *
             distanceToRange(totalAssign, contract.minShiftNum * currentWeek,
                             contract.maxShiftNum * currentWeek) /
             problem.scenario.totalWeekNum;
    delta += penalty.TotalAssign() *
             distanceToRange(totalAssign - problem.scenario.totalWeekNum,
                             contract.minShiftNum * currentWeek,
                             contract.maxShiftNum * currentWeek) /
             problem.scenario.totalWeekNum;
#else
    int restMinShift = problem.scenario.nurses[nurse].restMinShiftNum;
    int restMaxShift = problem.scenario.nurses[nurse].restMaxShiftNum;
    int totalAssign = totalAssignNums[nurse] * problem.history.restWeekCount;
    delta -= penalty.TotalAssign() *
             distanceToRange(totalAssign, restMinShift, restMaxShift) /
             problem.history.restWeekCount;
    delta += penalty.TotalAssign() *
             distanceToRange(totalAssign - problem.history.restWeekCount,
                             restMinShift, restMaxShift) /
             problem.history.restWeekCount;
#endif

    return delta;  // TODO : weight ?
}

NurseRostering::ObjValue NurseRostering::Solution::tryRemoveAssign(
    const Move &move) const {
    return tryRemoveAssign(move.weekday, move.nurse);
}

NurseRostering::ObjValue NurseRostering::Solution::trySwapNurse(
    int weekday, NurseID nurse, NurseID nurse2) const {
    // TODO : make sure they won't be the same and leave out this
    if (nurse == nurse2) {
        return DefaultPenalty::FORBIDDEN_MOVE;
    }

    if (assign.isWorking(nurse, weekday)) {
        if (assign.isWorking(nurse2, weekday)) {
            nurseDelta =
                tryChangeAssign(weekday, nurse, assign[nurse2][weekday]);
            nurse2Delta = ((nurseDelta >= DefaultPenalty::MAX_OBJ_VALUE)
                               ? 0
                               : tryChangeAssign(weekday, nurse2,
                                                 assign[nurse][weekday]));
        } else {
            nurseDelta = tryRemoveAssign(weekday, nurse);
            nurse2Delta =
                ((nurseDelta >= DefaultPenalty::MAX_OBJ_VALUE)
                     ? 0
                     : tryAddAssign(weekday, nurse2, assign[nurse][weekday]));
        }
    } else {
        if (assign.isWorking(nurse2, weekday)) {
            nurseDelta = tryAddAssign(weekday, nurse, assign[nurse2][weekday]);
            nurse2Delta = ((nurseDelta >= DefaultPenalty::MAX_OBJ_VALUE)
                               ? 0
                               : tryRemoveAssign(weekday, nurse2));
        } else {  // no change
            nurseDelta = 0;
            nurse2Delta = 0;
            return DefaultPenalty::FORBIDDEN_MOVE;
        }
    }

    return (nurseDelta + nurse2Delta);
}

NurseRostering::ObjValue NurseRostering::Solution::trySwapNurse(
    const Move &move) const {
    penalty.setSwapMode();
    ObjValue delta = trySwapNurse(move.weekday, move.nurse, move.nurse2);
    penalty.recoverLastMode();

    return delta;
}

NurseRostering::ObjValue NurseRostering::Solution::trySwapBlock(
    int weekday, int &weekday2, NurseID nurse, NurseID nurse2) const {
    // TODO : make sure they won't be the same and leave out this
    if (nurse == nurse2) {
        return DefaultPenalty::FORBIDDEN_MOVE;
    }

    if (!(isValidSuccession(nurse, assign[nurse2][weekday].shift, weekday) &&
          isValidSuccession(nurse2, assign[nurse][weekday].shift, weekday))) {
        return DefaultPenalty::FORBIDDEN_MOVE;
    }

    RandSelect<ObjValue> rs;
    ObjValue delta = 0;
    ObjValue delta1 = 0;
    ObjValue delta2 = 0;
    ObjValue minDelta = DefaultPenalty::FORBIDDEN_MOVE;
    ObjValue minNurseDelta = DefaultPenalty::FORBIDDEN_MOVE;
    ObjValue minNurse2Delta = DefaultPenalty::FORBIDDEN_MOVE;
#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
    RandSelect<ObjValue> rs_tabu;
    ObjValue minDelta_tabu = DefaultPenalty::FORBIDDEN_MOVE;
    ObjValue minNurseDelta_tabu = DefaultPenalty::FORBIDDEN_MOVE;
    ObjValue minNurse2Delta_tabu = DefaultPenalty::FORBIDDEN_MOVE;
    int weekday2_tabu = weekday;

    int count = 0;
    int noTabuCount = 0;
#endif
    // try each block length
    int w = weekday;
    while (problem.scenario.nurses[nurse].skills[assign[nurse2][w].skill] &&
           problem.scenario.nurses[nurse2].skills[assign[nurse][w].skill]) {
        // longer blocks will also miss this skill
        delta += trySwapNurse(w, nurse, nurse2);
        delta1 += nurseDelta;
        delta2 += nurse2Delta;

#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
        ++count;
        noTabuCount += noSwapTabu(w, nurse, nurse2);
#endif

        if (delta < DefaultPenalty::MAX_OBJ_VALUE) {
            if (isValidPrior(nurse, assign[nurse2][w].shift, w) &&
                isValidPrior(nurse2, assign[nurse][w].shift, w)) {
#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
                if (noBlockSwapTabu(noTabuCount, count)) {
#endif
                    if (rs.isMinimal(delta, minDelta, solver.randGen)) {
                        minDelta = delta;
                        weekday2 = w;
                        minNurseDelta = delta1;
                        minNurse2Delta = delta2;
                    }
#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(delta, minDelta_tabu,
                                          solver.randGen)) {
                        minDelta_tabu = delta;
                        weekday2_tabu = w;
                        minNurseDelta_tabu = delta1;
                        minNurse2Delta_tabu = delta2;
                    }
                }
#endif
            }
        } else {  // two day off
            delta -= DefaultPenalty::FORBIDDEN_MOVE;
        }

        if (w >= Weekday::Sun) {
            break;
        }

        (const_cast<Solution *>(this))->swapNurse(w, nurse, nurse2);
        ++w;
    }

    nurseDelta = minNurseDelta;
    nurse2Delta = minNurse2Delta;
#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
    if (aspirationCritiera(minDelta, minDelta_tabu)) {
        minDelta = minDelta_tabu;
        weekday2 = weekday2_tabu;
        nurseDelta = minNurseDelta_tabu;
        nurse2Delta = minNurse2Delta_tabu;
    }
#endif

    // recover original data
    while ((--w) >= weekday) {
        (const_cast<Solution *>(this))->swapNurse(w, nurse, nurse2);
    }

    return minDelta;
}

NurseRostering::ObjValue NurseRostering::Solution::trySwapBlock(
    const Move &move) const {
    penalty.setBlockSwapMode();
    ObjValue delta =
        trySwapBlock(move.weekday, move.weekday2, move.nurse, move.nurse2);
    penalty.recoverLastMode();

    return delta;
}

NurseRostering::ObjValue NurseRostering::Solution::trySwapBlock_fast(
    int &weekday, int &weekday2, NurseID nurse, NurseID nurse2) const {
    // TODO : make sure they won't be the same and leave out this
    if (nurse == nurse2) {
        return DefaultPenalty::FORBIDDEN_MOVE;
    }

    RandSelect<ObjValue> rs;
    ObjValue delta = 0;
    ObjValue delta1 = 0;
    ObjValue delta2 = 0;
    ObjValue minDelta = DefaultPenalty::FORBIDDEN_MOVE;
    ObjValue minNurseDelta = DefaultPenalty::FORBIDDEN_MOVE;
    ObjValue minNurse2Delta = DefaultPenalty::FORBIDDEN_MOVE;

    // prepare for hard constraint check and tabu judgment
    bool hasSkill[Weekday::SIZE];
    for (int w = Weekday::Mon; w <= Weekday::Sun; ++w) {
        hasSkill[w] =
            (problem.scenario.nurses[nurse].skills[assign[nurse2][w].skill] &&
             problem.scenario.nurses[nurse2].skills[assign[nurse][w].skill]);
    }

    weekday = Weekday::Mon;
    weekday2 = Weekday::Mon;

#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
    RandSelect<ObjValue> rs_tabu;
    ObjValue minDelta_tabu = DefaultPenalty::FORBIDDEN_MOVE;
    ObjValue minNurseDelta_tabu = DefaultPenalty::FORBIDDEN_MOVE;
    ObjValue minNurse2Delta_tabu = DefaultPenalty::FORBIDDEN_MOVE;

    int noTabuCount[Weekday::SIZE][Weekday::SIZE];
    bool noTabu[Weekday::SIZE][Weekday::SIZE];
    for (int w = Weekday::Mon; w <= Weekday::Sun; ++w) {
        noTabuCount[w][w] = noSwapTabu(w, nurse, nurse2);
        noTabu[w][w] = (noTabuCount[w][w] > 0);
    }
    for (int w = Weekday::Mon; w <= Weekday::Sun; ++w) {
        for (int w2 = w + 1; w2 <= Weekday::Sun; ++w2) {
            noTabuCount[w][w2] = noTabuCount[w][w2 - 1] + noTabuCount[w2][w2];
            noTabu[w][w2] = noBlockSwapTabu(noTabuCount[w][w2], w2 - w + 1);
        }
    }

    int weekday_tabu = weekday;
    int weekday2_tabu = weekday2;
#endif

    // try each block length
    for (int w = Weekday::Mon, w2; w <= Weekday::Sun; ++w) {
        if (!(isValidSuccession(nurse, assign[nurse2][w].shift, w) &&
              isValidSuccession(nurse2, assign[nurse][w].shift, w))) {
            continue;
        }

        w2 = w;
        for (; (w2 <= Weekday::Sun) && hasSkill[w2];
             ++w2) {  // longer blocks will also miss this skill
            delta += trySwapNurse(w2, nurse, nurse2);
            delta1 += nurseDelta;
            delta2 += nurse2Delta;

            if (delta < DefaultPenalty::MAX_OBJ_VALUE) {
                (const_cast<Solution *>(this))->swapNurse(w2, nurse, nurse2);

                if (isValidPrior(nurse, assign[nurse][w2].shift, w2) &&
                    isValidPrior(nurse2, assign[nurse2][w2].shift, w2)) {
#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
                    if (noTabu[w][w2]) {
#endif
                        if (rs.isMinimal(delta, minDelta, solver.randGen)) {
                            minDelta = delta;
                            weekday = w;
                            weekday2 = w2;
                            minNurseDelta = delta1;
                            minNurse2Delta = delta2;
                        }
#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
                    } else {  // tabu
                        if (rs_tabu.isMinimal(delta, minDelta_tabu,
                                              solver.randGen)) {
                            minDelta_tabu = delta;
                            weekday_tabu = w;
                            weekday2_tabu = w2;
                            minNurseDelta_tabu = delta1;
                            minNurse2Delta_tabu = delta2;
                        }
                    }
#endif
                }
            } else {  // two day off
                delta -= DefaultPenalty::FORBIDDEN_MOVE;
            }
        }

        if (w == w2) {
            continue;
        }  // the first day is not swapped

        do {
            delta += trySwapNurse(w, nurse, nurse2);
            delta1 += nurseDelta;
            delta2 += nurse2Delta;
            if (delta < DefaultPenalty::MAX_OBJ_VALUE) {
                (const_cast<Solution *>(this))->swapNurse(w, nurse, nurse2);
            } else {  // two day off
                delta -= DefaultPenalty::FORBIDDEN_MOVE;
            }
            ++w;
        } while ((w < w2) &&
                 !(isValidSuccession(nurse, assign[nurse][w].shift, w) &&
                   isValidSuccession(nurse2, assign[nurse2][w].shift, w)));

        while (w < (w2--)) {
            if (isValidPrior(nurse, assign[nurse][w2].shift, w2) &&
                isValidPrior(nurse2, assign[nurse2][w2].shift, w2)) {
#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
                if (noTabu[w][w2]) {
#endif
                    if (rs.isMinimal(delta, minDelta, solver.randGen)) {
                        minDelta = delta;
                        weekday = w;
                        weekday2 = w2;
                        minNurseDelta = delta1;
                        minNurse2Delta = delta2;
                    }
#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
                } else {  // tabu
                    if (rs_tabu.isMinimal(delta, minDelta_tabu,
                                          solver.randGen)) {
                        minDelta_tabu = delta;
                        weekday_tabu = w;
                        weekday2_tabu = w2;
                        minNurseDelta_tabu = delta1;
                        minNurse2Delta_tabu = delta2;
                    }
                }
#endif
            }

            delta += trySwapNurse(w2, nurse, nurse2);
            delta1 += nurseDelta;
            delta2 += nurse2Delta;
            if (delta < DefaultPenalty::MAX_OBJ_VALUE) {
                (const_cast<Solution *>(this))->swapNurse(w2, nurse, nurse2);
            } else {  // two day off
                delta -= DefaultPenalty::FORBIDDEN_MOVE;
            }
        }
    }

    nurseDelta = minNurseDelta;
    nurse2Delta = minNurse2Delta;
#if INRC2_BLOCK_SWAP_TABU_STRENGTH != INRC2_BLOCK_SWAP_NO_TABU
    if (aspirationCritiera(minDelta, minDelta_tabu)) {
        minDelta = minDelta_tabu;
        weekday = weekday_tabu;
        weekday2 = weekday2_tabu;
        nurseDelta = minNurseDelta_tabu;
        nurse2Delta = minNurse2Delta_tabu;
    }
#endif
    return minDelta;
}

NurseRostering::ObjValue NurseRostering::Solution::trySwapBlock_fast(
    const Move &move) const {
    penalty.setBlockSwapMode();
    ObjValue delta =
        trySwapBlock_fast(move.weekday, move.weekday2, move.nurse, move.nurse2);
    penalty.recoverLastMode();

    return delta;
}

NurseRostering::ObjValue NurseRostering::Solution::tryExchangeDay(
    int weekday, NurseID nurse, int weekday2) const {
    // TODO : make sure they won't be the same and leave out this
    if (weekday == weekday2) {
        return DefaultPenalty::FORBIDDEN_MOVE;
    }

    // both are day off will change nothing
    if (!assign.isWorking(nurse, weekday) &&
        !assign.isWorking(nurse, weekday2)) {
        return DefaultPenalty::FORBIDDEN_MOVE;
    }

    // check succession
    ShiftID shift = assign[nurse][weekday].shift;
    ShiftID shift2 = assign[nurse][weekday2].shift;
    if (weekday == weekday2 + 1) {
        if (!(isValidSuccession(nurse, shift, weekday2) &&
              problem.scenario.shifts[shift].legalNextShifts[shift2] &&
              isValidPrior(nurse, shift2, weekday))) {
            return DefaultPenalty::FORBIDDEN_MOVE;
        }
    } else if (weekday == weekday2 - 1) {
        if (!(isValidSuccession(nurse, shift2, weekday) &&
              problem.scenario.shifts[shift2].legalNextShifts[shift] &&
              isValidPrior(nurse, shift, weekday2))) {
            return DefaultPenalty::FORBIDDEN_MOVE;
        }
    } else {
        if (!(isValidSuccession(nurse, shift, weekday2) &&
              isValidPrior(nurse, shift, weekday2) &&
              isValidSuccession(nurse, shift2, weekday) &&
              isValidPrior(nurse, shift2, weekday))) {
            return DefaultPenalty::FORBIDDEN_MOVE;
        }
    }

    ObjValue delta = 0;

    if (assign.isWorking(nurse, weekday)) {
        if (assign.isWorking(nurse, weekday2)) {
            delta += tryChangeAssign(weekday, nurse, assign[nurse][weekday2]);
            if (delta < DefaultPenalty::MAX_OBJ_VALUE) {
                Assign a(assign[nurse][weekday]);
                (const_cast<Solution *>(this))
                    ->changeAssign(weekday, nurse, assign[nurse][weekday2]);
                delta += tryChangeAssign(weekday2, nurse, a);
                (const_cast<Solution *>(this))->changeAssign(weekday, nurse, a);
            }
        } else {
            delta += tryRemoveAssign(weekday, nurse);
            if (delta < DefaultPenalty::MAX_OBJ_VALUE) {
                Assign a(assign[nurse][weekday]);
                (const_cast<Solution *>(this))->removeAssign(weekday, nurse);
                delta += tryAddAssign(weekday2, nurse, a);
                (const_cast<Solution *>(this))->addAssign(weekday, nurse, a);
            }
        }
    } else {
        delta += tryAddAssign(weekday, nurse, assign[nurse][weekday2]);
        if (delta < DefaultPenalty::MAX_OBJ_VALUE) {
            (const_cast<Solution *>(this))
                ->addAssign(weekday, nurse, assign[nurse][weekday2]);
            delta += tryRemoveAssign(weekday2, nurse);
            (const_cast<Solution *>(this))->removeAssign(weekday, nurse);
        }
    }

    return delta;
}

NurseRostering::ObjValue NurseRostering::Solution::tryExchangeDay(
    const Move &move) const {
    penalty.setExchangeMode();
    ObjValue delta = tryExchangeDay(move.weekday, move.nurse, move.weekday2);
    penalty.recoverLastMode();

    return delta;
}

void NurseRostering::Solution::addAssign(int weekday, NurseID nurse,
                                         const Assign &a) {
    updateConsecutive(weekday, nurse, a.shift);

    --missingNurseNums[weekday][a.shift][a.skill];

    ++totalAssignNums[nurse];

    assign[nurse][weekday] = a;
}

void NurseRostering::Solution::addAssign(const Move &move) {
    addAssign(move.weekday, move.nurse, move.assign);
}

void NurseRostering::Solution::changeAssign(int weekday, NurseID nurse,
                                            const Assign &a) {
    if (a.shift != assign[nurse][weekday].shift) {  // for just change skill
        updateConsecutive(weekday, nurse, a.shift);
    }

    --missingNurseNums[weekday][a.shift][a.skill];
    ++missingNurseNums[weekday][assign[nurse][weekday].shift]
                      [assign[nurse][weekday].skill];

    assign[nurse][weekday] = a;
}

void NurseRostering::Solution::changeAssign(const Move &move) {
    changeAssign(move.weekday, move.nurse, move.assign);
}

void NurseRostering::Solution::removeAssign(int weekday, NurseID nurse) {
    updateConsecutive(weekday, nurse, NurseRostering::Scenario::Shift::ID_NONE);

    ++missingNurseNums[weekday][assign[nurse][weekday].shift]
                      [assign[nurse][weekday].skill];

    --totalAssignNums[nurse];

    assign[nurse][weekday] = Assign();
}

void NurseRostering::Solution::removeAssign(const Move &move) {
    removeAssign(move.weekday, move.nurse);
}

void NurseRostering::Solution::swapNurse(int weekday, NurseID nurse,
                                         NurseID nurse2) {
    if (assign.isWorking(nurse, weekday)) {
        if (assign.isWorking(nurse2, weekday)) {
            Assign a(assign[nurse][weekday]);
            changeAssign(weekday, nurse, assign[nurse2][weekday]);
            changeAssign(weekday, nurse2, a);
        } else {
            addAssign(weekday, nurse2, assign[nurse][weekday]);
            removeAssign(weekday, nurse);
        }
    } else {
        if (assign.isWorking(nurse2, weekday)) {
            addAssign(weekday, nurse, assign[nurse2][weekday]);
            removeAssign(weekday, nurse2);
        }
    }
}

void NurseRostering::Solution::swapNurse(const Move &move) {
    swapNurse(move.weekday, move.nurse, move.nurse2);
}

void NurseRostering::Solution::swapBlock(int weekday, int weekday2,
                                         NurseID nurse, NurseID nurse2) {
    for (; weekday <= weekday2; ++weekday) {
        swapNurse(weekday, nurse, nurse2);
    }
}

void NurseRostering::Solution::swapBlock(const Move &move) {
    swapBlock(move.weekday, move.weekday2, move.nurse, move.nurse2);
}

void NurseRostering::Solution::exchangeDay(int weekday, NurseID nurse,
                                           int weekday2) {
    if (assign.isWorking(nurse, weekday)) {
        if (assign.isWorking(nurse, weekday2)) {
            Assign a(assign[nurse][weekday]);
            changeAssign(weekday, nurse, assign[nurse][weekday2]);
            changeAssign(weekday2, nurse, a);
        } else {
            addAssign(weekday2, nurse, assign[nurse][weekday]);
            removeAssign(weekday, nurse);
        }
    } else {
        if (assign.isWorking(nurse, weekday2)) {
            addAssign(weekday, nurse, assign[nurse][weekday2]);
            removeAssign(weekday2, nurse);
        }
    }
}

void NurseRostering::Solution::exchangeDay(const Move &move) {
    exchangeDay(move.weekday, move.nurse, move.weekday2);
}

void NurseRostering::Solution::updateConsecutive(int weekday, NurseID nurse,
                                                 ShiftID shift) {
    Consecutive &c(consecutives[nurse]);
    int nextDay = weekday + 1;
    int prevDay = weekday - 1;

    // consider day
    bool isDayHigh = (weekday == c.dayHigh[weekday]);
    bool isDayLow = (weekday == c.dayLow[weekday]);
    if (assign.isWorking(nurse, weekday) != Assign::isWorking(shift)) {
        if (isDayHigh && isDayLow) {
            assignSingle(weekday, c.dayHigh, c.dayLow,
                         (weekday != Weekday::Sun), true);
        } else if (isDayHigh) {
            assignHigh(weekday, c.dayHigh, c.dayLow, (weekday != Weekday::Sun));
        } else if (isDayLow) {
            assignLow(weekday, c.dayHigh, c.dayLow, true);
        } else {
            assignMiddle(weekday, c.dayHigh, c.dayLow);
        }
    }

    // consider shift
    bool isShiftHigh = (weekday == c.shiftHigh[weekday]);
    bool isShiftLow = (weekday == c.shiftLow[weekday]);
    if (isShiftHigh && isShiftLow) {
        assignSingle(weekday, c.shiftHigh, c.shiftLow,
                     ((nextDay <= Weekday::Sun) &&
                      (shift == assign[nurse][nextDay].shift)),
                     (shift == assign[nurse][prevDay].shift));
    } else if (isShiftHigh) {
        assignHigh(weekday, c.shiftHigh, c.shiftLow,
                   ((nextDay <= Weekday::Sun) &&
                    (shift == assign[nurse][nextDay].shift)));
    } else if (isShiftLow) {
        assignLow(weekday, c.shiftHigh, c.shiftLow,
                  (shift == assign[nurse][prevDay].shift));
    } else {
        assignMiddle(weekday, c.shiftHigh, c.shiftLow);
    }
}

void NurseRostering::Solution::assignHigh(int weekday, int high[], int low[],
                                          bool affectRight) {
    int nextDay = weekday + 1;
    int prevDay = weekday - 1;
    for (int d = prevDay; (d >= Weekday::HIS) && (d >= low[weekday]); --d) {
        high[d] = prevDay;
    }
    if (affectRight) {
        for (int d = nextDay; d <= high[nextDay]; ++d) {
            low[d] = weekday;
        }
        high[weekday] = high[nextDay];
    } else {
        high[weekday] = weekday;
    }
    low[weekday] = weekday;
}

void NurseRostering::Solution::assignLow(int weekday, int high[], int low[],
                                         bool affectLeft) {
    int nextDay = weekday + 1;
    int prevDay = weekday - 1;
    for (int d = nextDay; d <= high[weekday]; ++d) {
        low[d] = nextDay;
    }
    if (affectLeft) {
        for (int d = prevDay; (d >= Weekday::HIS) && (d >= low[prevDay]); --d) {
            high[d] = weekday;
        }
        low[weekday] = low[prevDay];
    } else {
        low[weekday] = weekday;
    }
    high[weekday] = weekday;
}

void NurseRostering::Solution::assignMiddle(int weekday, int high[],
                                            int low[]) {
    int nextDay = weekday + 1;
    int prevDay = weekday - 1;
    for (int d = nextDay; d <= high[weekday]; ++d) {
        low[d] = nextDay;
    }
    for (int d = prevDay; (d >= Weekday::HIS) && (d >= low[weekday]); --d) {
        high[d] = prevDay;
    }
    high[weekday] = weekday;
    low[weekday] = weekday;
}

void NurseRostering::Solution::assignSingle(int weekday, int high[], int low[],
                                            bool affectRight, bool affectLeft) {
    int nextDay = weekday + 1;
    int prevDay = weekday - 1;
    int h = affectRight ? high[nextDay] : weekday;
    int l = affectLeft ? low[prevDay] : weekday;
    if (affectRight) {
        for (int d = nextDay; d <= high[nextDay]; ++d) {
            low[d] = l;
        }
        high[weekday] = h;
    }
    if (affectLeft) {
        for (int d = prevDay; (d >= Weekday::HIS) && (d >= low[prevDay]); --d) {
            high[d] = h;
        }
        low[weekday] = l;
    }
}

#ifdef INRC2_USE_TABU
void NurseRostering::Solution::updateDayTabu(NurseID nurse, int weekday) {
    dayTabu[nurse][weekday] = iterCount + solver.DayTabuTenureBase() +
                              (solver.randGen() % solver.DayTabuTenureAmp());
}

void NurseRostering::Solution::updateShiftTabu(NurseID nurse, int weekday,
                                               const Assign &a) {
    shiftTabu[nurse][weekday][a.shift][a.skill] =
        iterCount + solver.ShiftTabuTenureBase() +
        (solver.randGen() % solver.ShiftTabuTenureAmp());
}
#endif

bool NurseRostering::Solution::checkIncrementalUpdate() {
    bool correct = true;
    ostringstream oss;
    oss << iterCount;

    ObjValue incrementalVal = objValue;
    evaluateObjValue();
    if (solver.checkFeasibility(assign) != 0) {
        solver.errorLog("infeasible solution @" + oss.str());
        correct = false;
    }
    ObjValue checkResult = solver.checkObjValue(assign);
    if (checkResult != objValue) {
        solver.errorLog("check conflict with evaluate @" + oss.str());
        correct = false;
    }
    if (objValue != incrementalVal) {
        solver.errorLog("evaluate conflict with incremental update @" +
                        oss.str());
        correct = false;
    }

    return correct;
}

NurseRostering::ObjValue NurseRostering::Solution::evaluateInsufficientStaff() {
    ObjValue obj = 0;

    for (int weekday = Weekday::Mon; weekday <= Weekday::Sun; ++weekday) {
        for (ShiftID shift = NurseRostering::Scenario::Shift::ID_BEGIN;
             shift < problem.scenario.shiftSize; ++shift) {
            for (SkillID skill = NurseRostering::Scenario::Skill::ID_BEGIN;
                 skill < problem.scenario.skillSize; ++skill) {
                if (missingNurseNums[weekday][shift][skill] > 0) {
                    obj += penalty.InsufficientStaff() *
                           missingNurseNums[weekday][shift][skill];
                }
            }
        }
    }

    return obj;
}

NurseRostering::ObjValue NurseRostering::Solution::evaluateConsecutiveShift(
    NurseID nurse) {
    ObjValue obj = 0;

    const History &history(problem.history);
    const Consecutive &c(consecutives[nurse]);
    const vector<Scenario::Shift> &shifts(problem.scenario.shifts);

    int nextday = c.shiftHigh[Weekday::Mon] + 1;
    if (nextday <= Weekday::Sun) {  // the entire week is not one block
        // handle first block with history
        if (assign.isWorking(nurse, Weekday::Mon)) {
            const Scenario::Shift &shift(
                shifts[assign[nurse][Weekday::Mon].shift]);
            if (history.lastShifts[nurse] ==
                assign[nurse][Weekday::Mon].shift) {
                if (history.consecutiveShiftNums[nurse] >
                    shift.maxConsecutiveShiftNum) {
                    // (high - low + 1) which low is Mon for exceeding part in
                    // previous week has been counted
                    obj += penalty.ConsecutiveShift() *
                           (c.shiftHigh[Weekday::Mon] - Weekday::Mon + 1);
                } else {
                    obj += penalty.ConsecutiveShift() *
                           distanceToRange(c.shiftHigh[Weekday::Mon] -
                                               c.shiftLow[Weekday::Mon] + 1,
                                           shift.minConsecutiveShiftNum,
                                           shift.maxConsecutiveShiftNum);
                }
            } else {
                obj += penalty.ConsecutiveShift() *
                       distanceToRange(
                           c.shiftHigh[Weekday::Mon] - Weekday::Mon + 1,
                           shift.minConsecutiveShiftNum,
                           shift.maxConsecutiveShiftNum);
                if (Assign::isWorking(history.lastShifts[nurse])) {
                    obj += penalty.ConsecutiveShift() *
                           absentCount(history.consecutiveShiftNums[nurse],
                                       shifts[history.lastShifts[nurse]]
                                           .minConsecutiveShiftNum);
                }
            }
        } else if (Assign::isWorking(history.lastShifts[nurse])) {
            obj +=
                penalty.ConsecutiveShift() *
                absentCount(
                    history.consecutiveShiftNums[nurse],
                    shifts[history.lastShifts[nurse]].minConsecutiveShiftNum);
        }
        // handle blocks in the middle of the week
        for (; c.shiftHigh[nextday] < Weekday::Sun;
             nextday = c.shiftHigh[nextday] + 1) {
            if (assign.isWorking(nurse, nextday)) {
                const ShiftID &shiftID(assign[nurse][nextday].shift);
                const Scenario::Shift &shift(shifts[shiftID]);
                obj += penalty.ConsecutiveShift() *
                       distanceToRange(
                           c.shiftHigh[nextday] - c.shiftLow[nextday] + 1,
                           shift.minConsecutiveShiftNum,
                           shift.maxConsecutiveShiftNum);
            }
        }
    }
    // handle last consecutive block
    int consecutiveShift_EntireWeek =
        history.consecutiveShiftNums[nurse] + Weekday::NUM;
    int consecutiveShift =
        c.shiftHigh[Weekday::Sun] - c.shiftLow[Weekday::Sun] + 1;
    if (assign.isWorking(nurse, Weekday::Sun)) {
        const ShiftID &shiftID(assign[nurse][Weekday::Sun].shift);
        const Scenario::Shift &shift(problem.scenario.shifts[shiftID]);
        if (c.isSingleConsecutiveShift()) {  // the entire week is one block
            if (history.lastShifts[nurse] ==
                assign[nurse][Weekday::Sun].shift) {
                if (history.consecutiveShiftNums[nurse] >
                    shift.maxConsecutiveShiftNum) {
                    obj += penalty.ConsecutiveShift() * Weekday::NUM;
                } else {
                    obj += penalty.ConsecutiveShift() *
                           exceedCount(consecutiveShift_EntireWeek,
                                       shift.maxConsecutiveShiftNum);
                }
            } else {  // different shifts
                if (Weekday::NUM > shift.maxConsecutiveShiftNum) {
                    obj += penalty.ConsecutiveShift() *
                           (Weekday::NUM - shift.maxConsecutiveShiftNum);
                }
                if (Assign::isWorking(history.lastShifts[nurse])) {
                    obj += penalty.ConsecutiveShift() *
                           absentCount(history.consecutiveShiftNums[nurse],
                                       shifts[history.lastShifts[nurse]]
                                           .minConsecutiveShiftNum);
                }
            }
        } else {
            obj += penalty.ConsecutiveShift() *
                   exceedCount(consecutiveShift, shift.maxConsecutiveShiftNum);
        }
    } else if (c.isSingleConsecutiveShift()  // the entire week is one block
               && Assign::isWorking(history.lastShifts[nurse])) {
        obj += penalty.ConsecutiveShift() *
               absentCount(
                   history.consecutiveShiftNums[nurse],
                   shifts[history.lastShifts[nurse]].minConsecutiveShiftNum);
    }

    return obj;
}

NurseRostering::ObjValue NurseRostering::Solution::evaluateConsecutiveDay(
    NurseID nurse) {
    ObjValue obj = 0;

    const History &history(problem.history);
    const Consecutive &c(consecutives[nurse]);
    const ContractID &contractID(problem.scenario.nurses[nurse].contract);
    const Scenario::Contract &contract(problem.scenario.contracts[contractID]);

    int nextday = c.dayHigh[Weekday::Mon] + 1;
    if (nextday <= Weekday::Sun) {  // the entire week is not one block
        // handle first block with history
        if (assign.isWorking(nurse, Weekday::Mon)) {
            if (history.consecutiveDayNums[nurse] >
                contract.maxConsecutiveDayNum) {
                // (high - low + 1) which low is Mon for exceeding part in
                // previous week has been counted
                obj += penalty.ConsecutiveDay() *
                       (c.dayHigh[Weekday::Mon] - Weekday::Mon + 1);
            } else {
                obj += penalty.ConsecutiveDay() *
                       distanceToRange(
                           c.dayHigh[Weekday::Mon] - c.dayLow[Weekday::Mon] + 1,
                           contract.minConsecutiveDayNum,
                           contract.maxConsecutiveDayNum);
            }
        } else if (Assign::isWorking(history.lastShifts[nurse])) {
            obj += penalty.ConsecutiveDay() *
                   absentCount(history.consecutiveDayNums[nurse],
                               contract.minConsecutiveDayNum);
        }
        // handle blocks in the middle of the week
        for (; c.dayHigh[nextday] < Weekday::Sun;
             nextday = c.dayHigh[nextday] + 1) {
            if (assign.isWorking(nurse, nextday)) {
                obj +=
                    penalty.ConsecutiveDay() *
                    distanceToRange(c.dayHigh[nextday] - c.dayLow[nextday] + 1,
                                    contract.minConsecutiveDayNum,
                                    contract.maxConsecutiveDayNum);
            }
        }
    }
    // handle last consecutive block
    int consecutiveDay = c.dayHigh[Weekday::Sun] - c.dayLow[Weekday::Sun] + 1;
    if (assign.isWorking(nurse, Weekday::Sun)) {
        if (c.isSingleConsecutiveDay()) {  // the entire week is one block
            if (history.consecutiveDayNums[nurse] >
                contract.maxConsecutiveDayNum) {
                obj += penalty.ConsecutiveDay() * Weekday::NUM;
            } else {
                obj +=
                    penalty.ConsecutiveDay() *
                    exceedCount(consecutiveDay, contract.maxConsecutiveDayNum);
            }
        } else {
            obj += penalty.ConsecutiveDay() *
                   exceedCount(consecutiveDay, contract.maxConsecutiveDayNum);
        }
    } else if (c.isSingleConsecutiveDay()  // the entire week is one block
               && Assign::isWorking(history.lastShifts[nurse])) {
        obj += penalty.ConsecutiveDay() *
               absentCount(history.consecutiveDayNums[nurse],
                           contract.minConsecutiveDayNum);
    }

    return obj;
}

NurseRostering::ObjValue NurseRostering::Solution::evaluateConsecutiveDayOff(
    NurseID nurse) {
    ObjValue obj = 0;

    const History &history(problem.history);
    const Consecutive &c(consecutives[nurse]);
    const ContractID &contractID(problem.scenario.nurses[nurse].contract);
    const Scenario::Contract &contract(problem.scenario.contracts[contractID]);

    int nextday = c.dayHigh[Weekday::Mon] + 1;
    if (nextday <= Weekday::Sun) {  // the entire week is not one block
        // handle first block with history
        if (!assign.isWorking(nurse, Weekday::Mon)) {
            if (history.consecutiveDayoffNums[nurse] >
                contract.maxConsecutiveDayoffNum) {
                obj += penalty.ConsecutiveDayOff() *
                       (c.dayHigh[Weekday::Mon] - Weekday::Mon + 1);
            } else {
                obj += penalty.ConsecutiveDayOff() *
                       distanceToRange(
                           c.dayHigh[Weekday::Mon] - c.dayLow[Weekday::Mon] + 1,
                           contract.minConsecutiveDayoffNum,
                           contract.maxConsecutiveDayoffNum);
            }
        } else if (!Assign::isWorking(history.lastShifts[nurse])) {
            obj += penalty.ConsecutiveDayOff() *
                   absentCount(history.consecutiveDayoffNums[nurse],
                               contract.minConsecutiveDayoffNum);
        }
        // handle blocks in the middle of the week
        for (; c.dayHigh[nextday] < Weekday::Sun;
             nextday = c.dayHigh[nextday] + 1) {
            if (!assign.isWorking(nurse, nextday)) {
                obj +=
                    penalty.ConsecutiveDayOff() *
                    distanceToRange(c.dayHigh[nextday] - c.dayLow[nextday] + 1,
                                    contract.minConsecutiveDayoffNum,
                                    contract.maxConsecutiveDayoffNum);
            }
        }
    }
    // handle last consecutive block
    int consecutiveDay = c.dayHigh[Weekday::Sun] - c.dayLow[Weekday::Sun] + 1;
    if (!assign.isWorking(nurse, Weekday::Sun)) {
        if (c.isSingleConsecutiveDay()) {  // the entire week is one block
            if (history.consecutiveDayoffNums[nurse] >
                contract.maxConsecutiveDayoffNum) {
                obj += penalty.ConsecutiveDayOff() * Weekday::NUM;
            } else {
                obj += penalty.ConsecutiveDayOff() *
                       exceedCount(consecutiveDay,
                                   contract.maxConsecutiveDayoffNum);
            }
        } else {
            obj +=
                penalty.ConsecutiveDayOff() *
                exceedCount(consecutiveDay, contract.maxConsecutiveDayoffNum);
        }
    } else if (c.isSingleConsecutiveDay()  // the entire week is one block
               && (!Assign::isWorking(history.lastShifts[nurse]))) {
        obj += penalty.ConsecutiveDayOff() *
               absentCount(history.consecutiveDayoffNums[nurse],
                           contract.minConsecutiveDayoffNum);
    }

    return obj;
}

NurseRostering::ObjValue NurseRostering::Solution::evaluatePreference(
    NurseID nurse) {
    ObjValue obj = 0;

    for (int weekday = Weekday::Mon; weekday <= Weekday::Sun; ++weekday) {
        const ShiftID &shift = assign[nurse][weekday].shift;
        obj += penalty.Preference() *
               problem.weekData.shiftOffs[weekday][shift][nurse];
    }

    return obj;
}

NurseRostering::ObjValue NurseRostering::Solution::evaluateCompleteWeekend(
    NurseID nurse) {
    ObjValue obj = 0;

    obj += penalty.CompleteWeekend() *
           (problem.scenario.contracts[problem.scenario.nurses[nurse].contract]
                .completeWeekend &&
            (assign.isWorking(nurse, Weekday::Sat) !=
             assign.isWorking(nurse, Weekday::Sun)));

    return obj;
}

NurseRostering::ObjValue NurseRostering::Solution::evaluateTotalAssign(
    NurseID nurse) {
    ObjValue obj = 0;

    int min = problem.scenario.nurses[nurse].restMinShiftNum;
    int max = problem.scenario.nurses[nurse].restMaxShiftNum;
    obj +=
        penalty.TotalAssign() *
        distanceToRange(totalAssignNums[nurse] * problem.history.restWeekCount,
                        min, max) /
        problem.history.restWeekCount;

    return obj;
}

NurseRostering::ObjValue NurseRostering::Solution::evaluateTotalWorkingWeekend(
    NurseID nurse) {
    ObjValue obj = 0;

#ifdef INRC2_AVERAGE_MAX_WORKING_WEEKEND
    int maxWeekend =
        problem.scenario.contracts[problem.scenario.nurses[nurse].contract]
            .maxWorkingWeekendNum;
    int historyWeekend = problem.history.totalWorkingWeekendNums[nurse] *
                         problem.scenario.totalWeekNum;
    int exceedingWeekend = historyWeekend -
                           (maxWeekend * problem.history.currentWeek) +
                           ((assign.isWorking(nurse, Weekday::Sat) ||
                             assign.isWorking(nurse, Weekday::Sun)) *
                            problem.scenario.totalWeekNum);
    if (exceedingWeekend > 0) {
        obj += penalty.TotalWorkingWeekend() * exceedingWeekend /
               problem.scenario.totalWeekNum;
    }
#else
    int workingWeekendNum = (assign.isWorking(nurse, Weekday::Sat) ||
                             assign.isWorking(nurse, Weekday::Sun));
    obj +=
        penalty.TotalWorkingWeekend() *
        exceedCount(workingWeekendNum * problem.history.restWeekCount,
                    problem.scenario.nurses[nurse].restMaxWorkingWeekendNum) /
        problem.history.restWeekCount;
#endif

    return obj;
}

NurseRostering::Solution::AvailableNurses::AvailableNurses(const Solution &s)
    : sln(s), nurseWithSkill(s.solver.getNurseWithSkill()) {}

void NurseRostering::Solution::AvailableNurses::setEnvironment(int w,
                                                               SkillID s) {
    weekday = w;
    skill = s;
    minSkillNum = 0;
    int size = nurseWithSkill[skill].size();
    validNurseNum_CurDay = vector<int>(size);
    validNurseNum_CurShift = vector<int>(size);
    for (int i = 0; i < size; ++i) {
        validNurseNum_CurDay[i] = nurseWithSkill[skill][i].size();
        validNurseNum_CurShift[i] = validNurseNum_CurDay[i];
    }
}

void NurseRostering::Solution::AvailableNurses::setShift(ShiftID s) {
    shift = s;
    minSkillNum = 0;
    validNurseNum_CurShift = validNurseNum_CurDay;
}

NurseRostering::NurseID NurseRostering::Solution::AvailableNurses::getNurse() {
    while (true) {
        // find nurses who have the required skill with minimum skill number
        while (true) {
            if (validNurseNum_CurShift[minSkillNum] > 0) {
                break;
            } else if (++minSkillNum == validNurseNum_CurShift.size()) {
                return NurseRostering::Scenario::Nurse::ID_NONE;
            }
        }

        // select one nurse from it
        while (true) {
            int n = sln.solver.randGen() % validNurseNum_CurShift[minSkillNum];
            NurseID nurse = nurseWithSkill[skill][minSkillNum][n];
            vector<NurseID> &nurseSet = nurseWithSkill[skill][minSkillNum];
            if (sln.getAssignTable().isWorking(
                    nurse, weekday)) {  // set the nurse invalid for current day
                swap(nurseSet[n],
                     nurseSet[--validNurseNum_CurShift[minSkillNum]]);
                swap(nurseSet[validNurseNum_CurShift[minSkillNum]],
                     nurseSet[--validNurseNum_CurDay[minSkillNum]]);
            } else if (sln.isValidSuccession(nurse, shift, weekday)) {
                swap(nurseSet[n],
                     nurseSet[--validNurseNum_CurShift[minSkillNum]]);
                swap(nurseSet[validNurseNum_CurShift[minSkillNum]],
                     nurseSet[--validNurseNum_CurDay[minSkillNum]]);
                return nurse;
            } else {  // set the nurse invalid for current shift
                swap(nurseSet[n],
                     nurseSet[--validNurseNum_CurShift[minSkillNum]]);
            }
            if (validNurseNum_CurShift[minSkillNum] == 0) {
                break;
            }
        }
    }
}
