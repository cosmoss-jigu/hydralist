#include "common.h"
#include "numa.h"
#include "../lib/ARTROWEX/Tree.h"
/*
class SortedArray {
private:
    std::vector<std::pair<Key_t, void* >>  sortedArray;
public:
    bool insert(Key_t key, void* ptr) {
        sortedArray.emplace_back(key, ptr);
        sort(sortedArray.begin(), sortedArray.end());
        return true;
    }
    bool remove(Key_t key) {
        for(auto iter = sortedArray.begin(); iter != sortedArray.end(); ++iter) {
            if(iter->first == key)
                sortedArray.erase(iter);
        }

    }
    //Gets the value of the key if present or the value of key just less than key
    void* lookup(Key_t key) {
        int i = 0;
        if(sortedArray.empty())
            return nullptr;
        while(sortedArray[i].first < key && i < sortedArray.size()) {
            i++;
        }
        if(key < sortedArray[0].first)
            return nullptr;
        if(i == sortedArray.size())
            return sortedArray.back().second;
        if(sortedArray[i].first == key)
            return sortedArray[i].second;
        else
            return sortedArray[i-1].second;
    }
};*/

class ArtRowexIndex {
private:
    Key minKey;
    Key_t curMin;
    ART_ROWEX::Tree *idx;
    uint32_t numInserts = 0;
    int numa;
public:
    ArtRowexIndex() {
        idx = new ART_ROWEX::Tree([] (TID tid, Key &key){
            key.setInt(*reinterpret_cast<uint64_t*>(tid));
        });
        minKey.setInt(0);
        curMin = ULLONG_MAX;
    }
    ~ArtRowexIndex() {
            delete idx;
    }
    void setNuma(int numa){this->numa=numa;}
    void setKey(Key& k, uint64_t key) {k.setInt(key);}
    bool insert(Key_t key, void* ptr) {
        auto t = idx->getThreadInfo();
        Key k;
        setKey(k, key);
        idx->insert(k, reinterpret_cast<uint64_t>(ptr), t);
        if (key < curMin)
            curMin = key;
        numInserts++;
        return true;
    }
    bool remove(Key_t key, void* ptr) {
        auto t = idx->getThreadInfo();
        Key k;
        setKey(k, key);
        idx->remove(k, reinterpret_cast<uint64_t>(ptr), t);
        numInserts--;
        return true;
    }
    //Gets the value of the key if present or the value of key just less than/greater than key
    void* lookup(Key_t key) {
        if (key <= curMin)
            return nullptr;
        auto t = idx->getThreadInfo();
        Key endKey;
        setKey(endKey, key);

        auto result = idx->lookupNext(endKey, t);
        return reinterpret_cast<void*>(result);
    }

    // Art segfaults if range operation is done when there are less than 2 keys
    bool isEmpty() {return (numInserts < 2);}
    uint32_t size() {return numInserts;}

};
//typedef SortedArray SearchLayer;
typedef ArtRowexIndex SearchLayer;
