#include "Oplog.h"
#include "Combiner.h"
#include <gtest/gtest.h>

TEST(oplogTest, sanityTest) {
    Oplog log;
    std::vector<OpStruct::Operation> ops = {OpStruct::insert, OpStruct::insert, OpStruct::remove};
    for (auto op : ops) {
        log.enq(op, 100, nullptr);
    }
    int qnum = log.incrementCurQ();

    for(auto op:ops) {
        log.enq(op, 200, nullptr);
    }

    auto log1 = (log.getQ(qnum - 1));
    for(int i = 0; i < ops.size(); i++) {
        ASSERT_EQ(ops[i], (*log1)[i].op);
        ASSERT_EQ(100, (*log1)[i].key);
    }

    std::vector<OpStruct> *log2 = log.getQ(qnum);
    for(int i = 0; i < ops.size(); i++) {
        ASSERT_EQ(ops[i], (*log2)[i].op);
        ASSERT_EQ(200, (*log2)[i].key);
    }
    log.resetQ(qnum-1);
    log.resetQ(qnum);
    ASSERT_EQ(true, log1->empty());
    ASSERT_EQ(true, log2->empty());
}

TEST(oplogTest, combinerTest) {
    int MAX_THREADS = 4;
    Oplog log[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        g_perThreadLog.insert(&log[i]);
    }
    std::vector<OpStruct::Operation> ops = {OpStruct::insert, OpStruct::insert, OpStruct::remove};
    for(int i = 0; i < MAX_THREADS; i++) {
        for (auto op : ops) {
            log[i].enq(op, i*100, nullptr);
        }
    }

    CombinerThread combinerThread;
    auto mergedLog = combinerThread.combineLogs();

    int count = 0;
    for(auto i = mergedLog->begin(); i != mergedLog->end(); ++i) {
        OpStruct testops = *i;
        ASSERT_EQ(ops[count % ops.size()], testops.op);
        ASSERT_EQ(count/ops.size()*100, testops.key);
        count++;
    }
    combinerThread.broadcastMergedLog(mergedLog, 1);
    for (int i = 0; i < 1; i++) {
        ASSERT_EQ(mergedLog, g_workQueue[i].front());
    }
    free(mergedLog);
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
