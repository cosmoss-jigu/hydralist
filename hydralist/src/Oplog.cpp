#include "Oplog.h"
#include "ordo_clock.h"
#include <iostream>

std::set<Oplog*> g_perThreadLog;
boost::lockfree::spsc_queue<std::vector<OpStruct *>*, boost::lockfree::capacity<10000>> g_workQueue[MAX_NUMA * WORKER_THREAD_PER_NUMA];
thread_local Oplog* Oplog::perThreadLog;
std::atomic<int> numSplits;
int combinerSplits = 0;
std::atomic<unsigned long> curQ;
Oplog::Oplog() {
}

void Oplog::enq(OpStruct::Operation op, Key_t key, uint8_t hash, void* listNodePtr) {
    OpStruct* ops = new OpStruct;
    ops->op = op;
    ops->key = key;
    ops->hash = static_cast<uint8_t>(hash % WORKER_THREAD_PER_NUMA);
    ops->listNodePtr = listNodePtr;
    ops->ts = ordo_get_clock();
    unsigned long qnum = curQ % 2;
    while (!qLock[qnum].try_lock()) {
        std::atomic_thread_fence(std::memory_order_acq_rel);
        qnum = curQ % 2;
    }
    op_[qnum].push_back(ops);
    qLock[qnum].unlock();
    //std::atomic_fetch_add(&numSplits, 1);
}

void Oplog::resetQ(int qnum) {
    op_[qnum].clear();
}


std::vector<OpStruct*>* Oplog::getQ(int qnum) {
    return &op_[qnum];
}


Oplog* Oplog::getOpLog() {
    Oplog* perThreadLog = Oplog::getPerThreadInstance();
    if(!g_perThreadLog.count(perThreadLog)) {
        perThreadLog = new Oplog;
        g_perThreadLog.insert(perThreadLog);
        setPerThreadInstance(perThreadLog);
    }
    return perThreadLog;
}
void Oplog::enqPerThreadLog(OpStruct::Operation op, Key_t key, uint8_t hash, void* listNodePtr) {
    Oplog* perThreadLog = getOpLog();
    perThreadLog->enq(op, key, hash, listNodePtr);
}
