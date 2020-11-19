//
// Created by ajit on 3/22/19.
//

#ifndef HYDRALIST_COMBINER_H
#define HYDRALIST_COMBINER_H
#include "Oplog.h"
#include "WorkerThread.h"
#include <climits>


class CombinerThread {
private:
    std::queue<std::pair<unsigned long, std::vector<OpStruct *>*>> logQueue;
    unsigned long doneCountCombiner;

public:
    CombinerThread () { doneCountCombiner = 0; }
    std::vector<OpStruct *>* combineLogs() {
        std::atomic_fetch_add(&curQ, 1ul);
        int qnum = static_cast<int>((curQ - 1) % 2);
        auto mergedLog = new std::vector<OpStruct *>;
        for (auto &i : g_perThreadLog) {
            Oplog &log = *i;
            log.lock(qnum);
            auto op_ = log.getQ(qnum);
            if(!op_->empty())
                mergedLog->insert(std::end(*mergedLog), std::begin(*op_), std::end(*op_));
            log.resetQ(qnum);
            log.unlock(qnum);
        }
        std::sort(mergedLog->begin(), mergedLog->end());
        combinerSplits +=mergedLog->size();
        if (!mergedLog->empty()) {
            logQueue.push(make_pair(doneCountCombiner, mergedLog));
            doneCountCombiner++;
            return mergedLog;
        } else {
            delete mergedLog;
            return nullptr;
        }
    }

    void broadcastMergedLog(std::vector<OpStruct *>* mergedLog, int activeNuma) {
        for(auto i = 0; i < activeNuma * WORKER_THREAD_PER_NUMA; i++)
            g_workQueue[i].push(mergedLog);
    }

    uint64_t freeMergedLogs(int activeNuma, bool force) {
        if (!force && logQueue.size() < 100) return 0;

        unsigned long minDoneCountWt = ULONG_MAX;
        for (auto i = 0; i < activeNuma * WORKER_THREAD_PER_NUMA; i++) {
            unsigned long logDoneCount = g_WorkerThreadInst[i]->getLogDoneCount();
            if (logDoneCount < minDoneCountWt)
                minDoneCountWt = logDoneCount;
        }

        while (!logQueue.empty() && logQueue.front().first < minDoneCountWt) {
            auto mergedLog = logQueue.front().second;
            for (auto opsPtr : *mergedLog)
                delete opsPtr;

            delete mergedLog;
            logQueue.pop();
        }
        return minDoneCountWt;
    }

    bool mergedLogsToBeFreed() { return logQueue.empty(); }
};

#endif //HYDRALIST_COMBINER_H
