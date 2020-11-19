//
// Created by ajit on 3/25/19.
//

#ifndef HYDRALIST_HYDRALISTAPI_H
#define HYDRALIST_HYDRALISTAPI_H
#include "HydraListImpl.h"
#include "common.h"
class HydraList {
private:
    HydraListImpl *hl;
public:
    HydraList(int numa) {
        hl = new HydraListImpl(numa);
    }
    ~HydraList() {
        delete hl;
    }
    bool insert(Key_t key, Val_t val) {
        return hl->insert(key, val);
    }
    bool update(Key_t key, Val_t val) {
        return hl->update(key, val);
    }
    Val_t lookup(Key_t key) {
        return hl->lookup(key);
    }
    Val_t remove(Key_t key) {
        return hl->remove(key);
    }
    uint64_t scan(Key_t startKey, int range, std::vector<Val_t> &result) {
        return hl->scan(startKey, range, result);
    }
    void registerThread() {
        hl->registerThread();
    }
    void unregisterThread() {
        hl->unregisterThread();
    }
};

#endif //HYDRALIST_HYDRALISTAPI_H
