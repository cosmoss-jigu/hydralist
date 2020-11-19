#include "Oplog.h"
#include "WorkerThread.h"
#include <gtest/gtest.h>
#include <Combiner.h>

TEST(workerThreadTest, sanityTest) {
    WorkerThread wt(0);
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
    ASSERT_EQ(true, wt.isWorkQueueEmpty());
    combinerThread.broadcastMergedLog(mergedLog, 1);
    ASSERT_EQ(false, wt.isWorkQueueEmpty());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

