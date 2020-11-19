#ifndef _OPLOG_H_
#define _OPLOG_H_

#include <vector>
#include <atomic>
#include <algorithm>
#include <queue>
#include "common.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <set>
#include <mutex>


class Oplog {
private:
    std::mutex qLock[2];
    std::vector<OpStruct *> oplog1;
    std::vector<OpStruct *> oplog2;
    std::vector<std::vector<OpStruct *>> op_{oplog1, oplog2}; //@Changwoo: Is this a good idea?
    static thread_local Oplog* perThreadLog;
public:
    Oplog();
    void resetQ(int Qnum);
    std::vector<OpStruct *> *getQ(int qnum);
    static Oplog* getPerThreadInstance(){return perThreadLog;}
    static void setPerThreadInstance(Oplog* ptr){perThreadLog = ptr;}
    static Oplog* getOpLog();
    static void enqPerThreadLog(OpStruct::Operation op, Key_t &key, uint8_t hash, void* listNodePtr);
    void enq(OpStruct::Operation op, Key_t &key, uint8_t hash, void* listNodePtr);
    void lock(int qnum) {qLock[qnum].lock();}
    void unlock(int qnum) {qLock[qnum].unlock();}
};
extern std::set<Oplog*> g_perThreadLog;
extern boost::lockfree::spsc_queue<std::vector<OpStruct *>*, boost::lockfree::capacity<10000>> g_workQueue[MAX_NUMA * WORKER_THREAD_PER_NUMA];
extern std::atomic<int> numSplits;
extern int combinerSplits;
extern std::atomic<unsigned long> curQ;
#endif
