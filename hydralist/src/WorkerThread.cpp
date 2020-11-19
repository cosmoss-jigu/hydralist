//
// Created by ajit on 3/7/19.
//

#include "WorkerThread.h"
#include "Oplog.h"
#include "Combiner.h"
#include "HydraList.h"
#include <assert.h>

WorkerThread::WorkerThread(int id, int activeNuma) {
    this->workerThreadId = id;
    this->activeNuma = activeNuma;
    this->workQueue = &g_workQueue[workerThreadId];
    this->logDoneCount = 0;
    this->opcount = 0;
    if (id == 0) freeQueue = new std::queue<std::pair<uint64_t, void*>>;
}

bool WorkerThread::applyOperation() {
    std::vector<OpStruct *>* oplog = workQueue->front();
    int numaNode = workerThreadId % activeNuma;
    SearchLayer* sl = g_perNumaSlPtr[numaNode];
    uint8_t hash = static_cast<uint8_t>(workerThreadId / activeNuma);
    bool ret = false;
    for (auto opsPtr : *oplog) {
        OpStruct &ops = *opsPtr;
        if (ops.hash != hash) continue;
        opcount++;
        if (ops.op == OpStruct::insert)
            sl->insert(ops.key, ops.listNodePtr);
        else if (ops.op == OpStruct::remove) {
            sl->remove(ops.key, ops.listNodePtr);
            if (workerThreadId == 0) {
                std::pair<uint64_t, void *> removePair;
                removePair.first = logDoneCount;
                removePair.second = ops.listNodePtr;
                freeQueue->push(removePair);
                ret = true;
            }
        }
        else
            assert(0);
    }
    workQueue->pop();
    logDoneCount++;
    return ret;
}

bool WorkerThread::isWorkQueueEmpty() {
    return !workQueue->read_available();
}

void WorkerThread::freeListNodes(uint64_t removeCount) {
    assert(workerThreadId == 0 && freeQueue != NULL);
    if (freeQueue->empty()) return;
    while (!freeQueue->empty()) {
        std::pair<uint64_t, void*> removePair = freeQueue->front();
        if (removePair.first < removeCount) {
            free(removePair.second);
            freeQueue->pop();
        }
        else break;
    }
}
