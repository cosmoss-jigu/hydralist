//
// Created by ajit on 6/11/19.
//

#ifndef HYDRALIST_THREADS_H
#define HYDRALIST_THREADS_H
#include "common.h"
#include <atomic>

class ThreadData {
private:
    int threadId;
    uint64_t localClock;
    bool finish;
    volatile std::atomic<uint64_t> runCnt;
public:
    ThreadData(int threadId) {
        this->threadId = threadId;
        this->finish = false;
        this->runCnt = 0;
#ifdef HYDRALIST_ENABLE_STATS
        sltime = 0;
        dltime = 0;
#endif
    }
#ifdef HYDRALIST_ENABLE_STATS
    uint64_t sltime;
    uint64_t dltime;
#endif
    void setThreadId (int threadId) {this->threadId = threadId;}
    int getThreadId ()  {return this->threadId;}
    void setfinish() {this->finish = true;}
    bool getFinish() {return this->finish;}
    void setLocalClock (uint64_t clock) {this->localClock = clock;}
    uint64_t getLocalClock() {return this->localClock;}
    void incrementRunCntAtomic() { runCnt.fetch_add(1);};
    void incrementRunCnt() { runCnt++;};
    uint64_t getRunCnt() {return this->runCnt;}
    void read_lock(uint64_t clock) {
        this->setLocalClock(clock);
        this->incrementRunCntAtomic();
    }
    void read_unlock() {
        this->incrementRunCnt();
    }


};

#endif //HYDRALIST_THREADS_H
