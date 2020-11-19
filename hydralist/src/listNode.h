#ifndef _LISTNODE_H
#define _LISTNODE_H

#include <vector>
#include "bitset.h"
#include "VersionedLock.h"
#include "common.h"
#include <mutex>

#define MAX_ENTRIES 64
class ListNode
{
private:
    Key_t min;
    volatile Key_t max;
    uint8_t numEntries;
    ListNode* next;
    ListNode* prev;
    bool deleted;
    hydra::bitset bitMap;
    uint8_t fingerPrint[MAX_ENTRIES];
    VersionedLock verLock;

    //Key_t keyArray[MAX_ENTRIES];
    //Val_t valueArray[MAX_ENTRIES];
    std::pair<Key_t, Val_t> keyArray[MAX_ENTRIES];

    uint64_t lastScanVersion;
    std::mutex pLock;
    uint8_t permuter[MAX_ENTRIES];

    ListNode* split(Key_t key, Val_t val, uint8_t keyHash);
    ListNode* mergeWithNext();
    ListNode* mergeWithPrev();
    uint8_t getKeyInsertIndex(Key_t key);
    int getKeyIndex(Key_t key, uint8_t keyHash);
    int getFreeIndex(Key_t key, uint8_t keyHash);
    bool insertAtIndex(std::pair<Key_t,Val_t> key, int index, uint8_t keyHash);
    bool updateAtIndex(Val_t value, int index);
    bool removeFromIndex(int index);
    int lowerBound(Key_t key);
    uint8_t getKeyFingerPrint(Key_t key);
    void generatePermuter(uint64_t writeVersion);
    int permuterLowerBound(Key_t key);

public:
    ListNode();
    bool insert(Key_t key, Val_t value);
    bool update(Key_t key, Val_t value);
    bool remove(Key_t key);
    bool probe(Key_t key); //return True if key exists
    bool lookup(Key_t key, Val_t &value);
    bool scan(Key_t startKey, int range, std::vector<Val_t> &rangeVector, uint64_t writeVersion);
    void print();
    bool checkRange(Key_t key);
    bool checkRangeLookup(Key_t key);
    version_t readLock();
    version_t writeLock();
    bool readUnlock(version_t);
    void writeUnlock();
    void setNext(ListNode* next);
    void setPrev(ListNode* prev);
    void setMin(Key_t key);
    void setMax(Key_t key);
    void setNumEntries(int numEntries);
    void setDeleted(bool deleted);
    ListNode* getNext();
    ListNode* getPrev();
    int getNumEntries();
    Key_t getMin();
    Key_t getMax();
    std::pair<Key_t, Val_t>* getKeyArray();
    std::pair<Key_t, Val_t>* getValueArray();
    hydra::bitset *getBitMap();
    uint8_t *getFingerPrintArray();
    bool getDeleted();

};
#endif
