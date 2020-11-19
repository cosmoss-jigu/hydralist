#include "HydraList.h"
#include <gtest/gtest.h>
#include <numa-config.h>

TEST(hydraListTest, sanityTest) {
    HydraList hl(1);
    hl.registerThread();
    ASSERT_EQ(true, hl.insert(10,10));
    ASSERT_EQ(10, hl.lookup(10));
    hl.unregisterThread();

}
TEST(hydraListTest, splitTest) {
    int n = 10000;
    HydraList *hl = new HydraList(1);
    hl->registerThread();

    for(int i = 1; i < n; i++) {
        ASSERT_EQ(true, hl->insert(i, i));
    }
    //waiting for combiner and worker thread to catch up;
    sleep(1);
    for(int i = 1; i < n; i++) {
        ASSERT_EQ(i, hl->lookup(i));
    }
    hl->unregisterThread();
    delete hl;
}

void insert(int threadId, int n, int MAX_THREAD, HydraList *hl) {
    int range = n/MAX_THREAD;
    hl->registerThread();
    for(int i = range*threadId; i< range*(threadId+1); i++) {
        if (i == 0) continue;
        ASSERT_EQ(true, hl->insert(i,i));
    }
    hl->unregisterThread();
}

TEST(hydraListTest, concurrentInsert) {
    int n = 100000;
    HydraList* hl = new HydraList(1);
    int MAX_THREAD = 4;
    std::thread *threads[MAX_THREAD];
    for(int i = 0; i < MAX_THREAD; i++) {
        threads[i] = new std::thread(insert, i, n, MAX_THREAD, hl);
    }
    for(int i = 0; i < MAX_THREAD; i++) {
        threads[i]->join();
    }
    sleep(1);
    hl->registerThread();
    for(int i = 1; i < n; i++) {
            if(hl->lookup(i) != i)
                    std::cout << i << std::endl;
        ASSERT_EQ(i, hl->lookup(i));
    }
    hl->unregisterThread();
    delete hl;
}

TEST(hydraListTest, scan) {
    int n = 100000;
    HydraList* hl = new HydraList(1);
    hl->registerThread();
    int MAX_THREAD = 1;
    std::thread *threads[MAX_THREAD];
    for(int i = 0; i < MAX_THREAD; i++) {
        threads[i] = new std::thread(insert, i, n, MAX_THREAD, hl);
    }
    for(int i = 0; i < MAX_THREAD; i++) {
        threads[i]->join();
    }
    sleep(1);
    std::vector<Val_t> result(100);
    int range = 100;
    Key_t startKey = 100;
    uint64_t resultSize = hl->scan(startKey, range, result);
    ASSERT_EQ(resultSize, range);
    /*for (uint64_t i = 0; i < resultSize; i++) {
        ASSERT_EQ(result[i], startKey + i);
    }*/
    hl->unregisterThread();
    delete hl;
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
