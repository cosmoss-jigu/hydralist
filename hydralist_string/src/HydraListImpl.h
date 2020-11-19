//
// Created by ajit on 3/8/19.
//

#ifndef HYDRALIST_HYDRALIST_H
#define HYDRALIST_HYDRALIST_H

#include <utility>
#include <vector>
#include <algorithm>
#include "common.h"
#include "linkedList.h"
#include <thread>
#include <set>
#include "SearchLayer.h"
#include "threadData.h"

//Temp class. Will be replaced by ART
// Uses a sorted array for search
extern std::vector<SearchLayer*> g_perNumaSlPtr;
extern std::set<ThreadData*> g_threadDataSet;
typedef LinkedList DataLayer;
class HydraListImpl {
private:
    SearchLayer sl;
    DataLayer dl;
    std::vector<std::thread*> wtArray; // CurrentOne but there should be numa number of threads
    std::thread* combinerThead;
    static thread_local int threadNumaNode;
    void createWorkerThread(int numNuma);
    void createCombinerThread();
    ListNode* getJumpNode(Key_t &key);
    static int totalNumaActive;
    std::atomic<uint32_t> numThreads;



public:
    explicit HydraListImpl(int numNuma);
    ~HydraListImpl();
    bool insert(Key_t &key, Val_t val);
    bool update(Key_t &key, Val_t val);
    bool remove(Key_t &key);
    void registerThread();
    void unregisterThread();
    Val_t lookup(Key_t &key);
    uint64_t scan(Key_t &startKey, int range, std::vector<Val_t> &result);
    static SearchLayer* createSearchLayer();
    static int getThreadNuma();

#ifdef HYDRALIST_ENABLE_STATS
    std::atomic<uint64_t> total_sl_time;
    std::atomic<uint64_t> total_dl_time;
#endif
};


#endif //HYDRALIST_HYDRALIST_H
