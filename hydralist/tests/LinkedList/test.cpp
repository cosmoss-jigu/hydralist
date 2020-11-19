#include "linkedList.h"
#include "VersionedLock.h"
#include "Oplog.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <climits>

TEST(LinkedListTest, initializeTest) {
    LinkedList testList;
    ASSERT_NE(nullptr, testList.initialize());
    ASSERT_EQ(1, testList.getHead()->getNumEntries());
    ASSERT_NE(nullptr, testList.getHead()->getNext());
}

TEST(LinkedListTest, insertRemoveTest) {
    LinkedList testList;
    ListNode* head = testList.initialize();

    Val_t value = 100;
    testList.insert(100, value, testList.getHead());
    testList.insert(200, value, head);
    testList.insert(50, value, head);
    testList.insert(500, value, head);
    ASSERT_EQ(true, testList.lookup(100, value, head));
    ASSERT_EQ(true, testList.lookup(200, value, head));
    ASSERT_EQ(true, testList.lookup(500, value, head));
    ASSERT_EQ(true, testList.lookup(50, value, head));
    ASSERT_EQ(5, head->getNumEntries());
    ASSERT_EQ(true, testList.lookup(0, value, head));
    ASSERT_EQ(false, testList.insert(100, value, head));

    testList.remove(100, head);
    testList.remove(50, head);
    ASSERT_EQ(false, testList.lookup(50, value, head));
    ASSERT_EQ(false, testList.lookup(100, value, head));
    ASSERT_EQ(true, testList.lookup(500, value, head));
    ASSERT_EQ(3, head->getNumEntries());
}

TEST(LinkedListTest, splitTest) {
    LinkedList testList;
    ListNode* head = testList.initialize();

    int testsize = 200;
    std::set<int> randset;
    std::vector<int> randnums;

    for (int i = 0; i < testsize; i++){
        int key = rand();
        if(!randset.count(key)){
            randset.insert(key);
            randnums.push_back(key);
        }

    }

    for (auto x : randnums){
        ASSERT_EQ(true, testList.insert(x, x, head));
    }
    //testList.print(head);
    for (auto x: randnums){
        Val_t y;
        ASSERT_EQ(true, testList.lookup(x, y, head));
    }
    for (auto x : randnums) {
        ASSERT_EQ(true, testList.remove(x, head));
    }
    //testList.print(head);

}
void multithreadInsert(LinkedList* ll, int threadId, int numInserts, int MAX_THREADS, std::vector<int> &nums) {
    int range = numInserts / MAX_THREADS;
    for(volatile int i = range*threadId; i < (range*(threadId+1)); i++) {
        ll->insert(nums[i], nums[i], ll->getHead());
    }
}
void multithreadRemove(LinkedList* ll, int threadId, int numInserts, int MAX_THREADS, std::vector<int>* numsPtr) {
    std::vector<int> &nums = *numsPtr;
    int range = numInserts/MAX_THREADS;
    for(volatile int i = range*threadId; i < range*(threadId+1); i++) {
        uint64_t x = nums[i];
        ll->remove(x, ll->getHead());
    }
}
TEST(LinkedListTest, multithread1) {
    int MAX_THREADS = 8;
    int numInserts = 10000;
    std::vector<int> nums(numInserts);
    LinkedList ll;
    ll.initialize();
    for(int i = 0; i < numInserts; i++) {
        nums[i] = rand();
        if(nums[i] == 0 || nums[i] == ULLONG_MAX)
            i--;
    }
    std::thread* threads[MAX_THREADS];
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(multithreadInsert, &ll, i, numInserts, MAX_THREADS, std::ref(nums));
    }
    for(int i = 0; i < MAX_THREADS; i++)
        threads[i]->join();

    Val_t val;
    for(int i = 0; i < numInserts; i++) {
        ASSERT_EQ(true, ll.lookup(nums[i], val, ll.getHead()));
    }
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(multithreadRemove, &ll, i, numInserts, MAX_THREADS, &nums);
    }
    for(int i = 0; i < MAX_THREADS; i++)
        threads[i]->join();
    //ll.print(ll.getHead());

}

TEST(LinkedListTest, scanTest) {
    int MAX_THREADS = 1;
    int numInserts = 10000;
    std::vector<int> nums(numInserts);
    LinkedList ll;
    ll.initialize();
    for(int i = 1; i < numInserts+1; i++) {
        nums[i] = i;
    }
    std::thread* threads[MAX_THREADS];
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(multithreadInsert, &ll, i, numInserts, MAX_THREADS, std::ref(nums));
    }
    for(int i = 0; i < MAX_THREADS; i++)
        threads[i]->join();

    std::vector<Val_t> result(100);
    uint64_t resultSize = ll.scan(100, 100, result, ll.getHead());
    ASSERT_EQ(resultSize, 100);
    for (uint32_t i = 0; i < resultSize; i++) {
        ASSERT_EQ(result[i], 100 + i);
    }

}


TEST(lockTest, sanitytest) {
    VersionedLock lock;
    std::vector<int> tvector;
    lock.write_lock();
    tvector.push_back(1);
    tvector.push_back(2);
    tvector.push_back(3);
    lock.write_unlock();
    version_t ver = lock.read_lock();
    ASSERT_EQ(4, ver);
    bool success = lock.read_unlock(ver);
    ASSERT_EQ(true, success);
}

TEST(lockTest, InterleaveRW) {
    VersionedLock lock;
    std::vector<int> tvector;
    version_t ver = lock.read_lock();
    ASSERT_EQ(2, ver);
    lock.write_lock();
    tvector.push_back(1);
    tvector.push_back(2);
    tvector.push_back(3);
    lock.write_unlock();
    bool success = lock.read_unlock(ver);
    ASSERT_EQ(false, success);
    ver = lock.read_lock();
    ASSERT_EQ(4, ver);
    success = lock.read_unlock(ver);
    ASSERT_EQ(true, success);
}

//TODO Write lock unit test again since api has changed
int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
