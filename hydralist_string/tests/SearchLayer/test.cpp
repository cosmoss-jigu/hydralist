#include "HydraList.h"
#include <gtest/gtest.h>
#include <fstream>
#include <string.h>
using namespace std;


void insert (SearchLayer *sl, int threadId, int totalInserts, int MAX_THREADS, std::vector<uint64_t >* keyptr) {
    std::vector<uint64_t > &keys = *keyptr;
    int range = totalInserts / MAX_THREADS;
    uint64_t *dataptr = keys.data();
    for(unsigned long i = threadId * range; i < (threadId+1) * range; i++) {
        sl->insert(keys[i], (void*)&keys[i]);
    }
}
void insert1 (SearchLayer *sl, int threadId, int totalInserts, int MAX_THREADS, uint64_t *keys) {
    int range = totalInserts / MAX_THREADS;
    for(unsigned long i = threadId * range; i < (threadId+1) * range; i++) {
        if (i % 8 == 0)
            sl->insert(keys[i], (void*)&keys[i]);
    }
}
void lookup (SearchLayer *sl, int threadId, int totalInserts, int MAX_THREADS, std::vector<uint64_t >* keyptr) {
    std::vector<uint64_t > &keys = *keyptr;
    int range = totalInserts / MAX_THREADS;
    uint64_t *dataptr = keys.data();
    for(unsigned long i = threadId * range; i < (threadId+1) * range; i++) {
        ASSERT_EQ(&keys[i], sl->lookup(keys[i]));
    }
}
void lookupInsert (SearchLayer *sl, int threadId, int totalInserts, int MAX_THREADS, std::vector<uint64_t >* keyptr) {
    std::vector<uint64_t > &keys = *keyptr;
    int range = totalInserts / MAX_THREADS;
    uint64_t *dataptr = keys.data();
    for(unsigned long i = threadId * range; i < (threadId+1) * range; i++) {
        sl->lookup(keys[i]);
        sl->insert(keys[i], (void*)&keys[i]);
    }
}
TEST(searchLayerTest, sanityTest) {
    SearchLayer *sl = new SearchLayer;
    int MAX_THREADS = 200;
    std::thread* threads[MAX_THREADS];
    int totalInserts = 100000;
    std::vector<uint64_t>keys(totalInserts);
    for(uint64_t i = 0; i < totalInserts; i++)
        keys[i] = i;
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(insert, sl, i, totalInserts, MAX_THREADS, &keys);
    }
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i]->join();
    }

    for(uint64_t i = 10 ; i < totalInserts; i++) {
        ASSERT_EQ(&keys[i], sl->lookup(keys[i]));
    }
    delete sl;

}
TEST(searchLayerTest, randTest) {
    SearchLayer *sl = new SearchLayer;
    int MAX_THREADS = 200;
    std::thread* threads[MAX_THREADS];
    int totalInserts = 10000;
    std::vector<uint64_t>keys(totalInserts);
    std::set<uint64_t> keyset;
    for(uint64_t i = 0; i < totalInserts; i++) {
        uint64_t x = rand();
        assert(sizeof(x) == 8);
        if(keyset.count(x) == 0){
           keys[i] = x;
           keyset.insert(x);
        }
        else
           i--;
    }
    sort(keys.begin(), keys.end());
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(insert, sl, i, totalInserts, MAX_THREADS, &keys);
    }
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i]->join();
    }

    for(uint64_t i = 10 ; i < totalInserts; i++) {
        if (keyset.count(keys[i]+1) == 0) {
            auto val = sl->lookup(keys[i] + 1);
            uint64_t* pval = reinterpret_cast<uint64_t*>(val);
            ASSERT_TRUE(&keys[i] == pval); } else
            ASSERT_EQ(&keys[i], sl->lookup(keys[i]));  
    }
    delete sl;

}

//TODO fix this unit test
TEST(searchLayerTest, parallelLookupTest) {
    SearchLayer *sl = new SearchLayer;
    int MAX_THREADS = 1;
    std::thread* threads[MAX_THREADS];
    int totalInserts = 10000;
    std::vector<uint64_t>keys(totalInserts);
    std::set<uint64_t> keyset;
    for(uint64_t i = 0; i < totalInserts; i++) {
        uint64_t x = rand();
        assert(sizeof(x) == 8);
        if(keyset.count(x) == 0){
           keys[i] = x;
           keyset.insert(x);
        }
        else
           i--;
    }
    sort(keys.begin(), keys.end());
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(insert, sl, i, totalInserts, MAX_THREADS, &keys);
    }
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i]->join();
    }
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(lookup, sl, i, totalInserts, MAX_THREADS, &keys);
    }
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i]->join();
    }

    delete sl;

}

TEST(searchLayerTest, powerOf2Test) {
    SearchLayer *sl = new SearchLayer;
    int MAX_THREADS = 1;
    std::thread* threads[MAX_THREADS];
    int totalInserts = 100;
    std::vector<uint64_t>keys(totalInserts);
    for(uint64_t i = 1; i < totalInserts; i++)
        keys[i] = i;
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(insert, sl, i, totalInserts, MAX_THREADS, &keys);
    }
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i]->join();
    }

    for(uint64_t i = 10 ; i < totalInserts; i++) {
        ASSERT_EQ(&keys[i], sl->lookup(keys[i]));
    }
    
    uint64_t k = 512;
    ASSERT_EQ(&keys[99], sl->lookup(k));
    
    delete sl;
}

TEST(searchLayerTest, xyzTest) {
    SearchLayer *sl = new SearchLayer;
    int MAX_THREADS = 1;
    std::thread* threads[MAX_THREADS];
    int totalInserts = 100;
    std::vector<uint64_t>keys(totalInserts);
    for(uint64_t i = 1; i < totalInserts; i++)
        keys[i] = i;
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(insert, sl, i, totalInserts, MAX_THREADS, &keys);
    }
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i]->join();
    }
    delete sl;


}

TEST(searchLayerTest, logTest) {
    SearchLayer *sl = new SearchLayer;
    int MAX_THREADS = 1;
    std::thread* threads[MAX_THREADS];
    std::vector<uint64_t> keys;
    std::ifstream logfile;
    logfile.open("../../../tests/SearchLayer/log.txt");
    if (!logfile.is_open())
        return;
    do {
        uint64_t k;
        logfile >> k;
        keys.push_back(k);
    }while(!logfile.eof());
    threads[0] = new std::thread(insert, sl, 0, keys.size()-1, 1, &keys);
    threads[0]->join();

    //uint64_t k = 1;
    //ASSERT_EQ(nullptr, sl->lookup(k));
    delete sl;
}

TEST(searchLayerTest, logTest1) {
    SearchLayer *sl = new SearchLayer;
    int MAX_THREADS = 1;
    std::thread* threads[MAX_THREADS];
    std::vector<uint64_t> keys;
    std::ifstream logfile;
    logfile.open("../../../tests/SearchLayer/log.txt");
    if (!logfile.is_open())
        return;
    do {
        uint64_t k;
        logfile >> k;
        keys.push_back(k);
    }while(!logfile.eof());
    threads[0] = new std::thread(lookupInsert, sl, 0, keys.size()-1, 1, &keys);
    threads[0]->join();

    delete sl;
    
    
}

TEST(searchLayerTest, randTest2) {
    SearchLayer *sl = new SearchLayer;
    int MAX_THREADS = 200;
    std::thread* threads[MAX_THREADS];
    int totalInserts = 100000;
    std::vector<uint64_t>keys(totalInserts);
    for(uint64_t i = 0; i < totalInserts; i++) {
        keys[i] = i * 100;
    }
    sort(keys.begin(), keys.end());
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(insert, sl, i, totalInserts, MAX_THREADS, &keys);
    }
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i]->join();
    }

    for(uint64_t i = 0 ; i < totalInserts; i++) {

        uint64_t key = rand() % keys[totalInserts - 1];
        auto val = sl->lookup(key);
        uint64_t* pval = reinterpret_cast<uint64_t*>(val);
        //std::cout << &keys[key/100] << " " << pval << std::endl;

        if (&keys[key/100] != pval){
           std::cout << key << std::endl;
           std::cout << keys[key/100] << " " << *pval << std::endl;
           ASSERT_EQ(1, 0);
        }
    }
    delete sl;

}

TEST(searchLayerTest, workloadC) {
    SearchLayer *sl = new SearchLayer;
    int MAX_THREADS = 200;
    std::thread* threads[MAX_THREADS];
    std::string init_file = "/home/ajit/research/index-microbench/workloads/loadc_zipf_int_100M.dat";
    std::string txn_file = "/home/ajit/research/index-microbench/workloads/txnsc_zipf_int_100M.dat";

    std::ifstream infile_load(init_file);
    std::string op;
    uint64_t key;

    std::string insert("INSERT");
    std::string read("READ");

    static const int totalInserts = 100000000;
    int count = 0;
    uint64_t keyvec[totalInserts];
    
    while ((count < totalInserts) && infile_load.good()) {
        infile_load >> op >> key;
        ASSERT_TRUE(!op.compare(insert));
        keyvec[count] = key;
        count++;
    }

    std::ifstream infile_txn(txn_file);
    count = 0;
    static const int totalLookup = 10000000;
    uint64_t lookupvec[totalLookup];
    while ((count < totalLookup) && infile_txn.good()) {
        infile_txn >> op >> key;
        if (op.compare(read) == 0) {
            lookupvec[count] = key;
        } else exit(0);
        count++;
    }
    for (int i = 0; i < MAX_THREADS; i++) {
        threads[i] = new std::thread(insert1, sl, i, totalInserts, MAX_THREADS, keyvec);
    }
    for (int i = 0; i < MAX_THREADS; i++) {
        threads[i]->join();
    }
    auto x = sl->lookup(6970446000655125957);
    uint64_t* xy = reinterpret_cast<uint64_t*>(x);
    cout<< "x: " << *xy << endl;

    int k = 0;
    for(int i =0 ; i < totalLookup; i++) {
        uint64_t key = lookupvec[i];
        auto val = sl->lookup(key);
        uint64_t* pval = reinterpret_cast<uint64_t*>(val);
        if (pval == 0) { 
                continue;
        }
        //std::cout << &keys[key/100] << " " << pval << std::endl;
        if (key < *pval){
           k++;
           std::cout << key << " " << *pval << std::endl;
           //ASSERT_EQ(1, 0);
        }
    }
    std::cout << k << std::endl;


}
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

