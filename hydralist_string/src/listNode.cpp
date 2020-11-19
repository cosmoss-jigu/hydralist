#include <cstring>
#include <iostream>
#include "Oplog.h"
#include "listNode.h"
#pragma GCC target("avx")
#include <xmmintrin.h>
#include <immintrin.h>

ListNode :: ListNode(){
    numEntries = 0;
    deleted = false;
    bitMap.clear();
    lastScanVersion = 0;
}


void ListNode::setNext(ListNode *next) {
    this->next = next;
}

void ListNode::setPrev(ListNode *prev) {
    this->prev = prev;
}

void ListNode::setMin(Key_t key) {
    this->min = key;
}

void ListNode::setPrefixLength(uint8_t pl) {
    this->prefixLength = pl;
}

void ListNode::setNumEntries(int numEntries) {
    this->numEntries = numEntries;
}

void ListNode::setDeleted(bool deleted) {
    this->deleted = deleted;
}

ListNode *ListNode::getNext() {
    return next;
}

ListNode *ListNode::getPrev() {
    return prev;
}

version_t ListNode::readLock() {
    return verLock.read_lock();
}

version_t ListNode::writeLock() {
    return verLock.write_lock();
}

bool ListNode::readUnlock(version_t oldVersion) {
    return verLock.read_unlock(oldVersion);
}

void ListNode::writeUnlock() {
    return verLock.write_unlock();
}

int ListNode :: getNumEntries()
{
    return this->numEntries;
}

Key_t* ListNode :: getKeyArray()
{
    return this->keyArray;
}

Val_t* ListNode :: getValueArray()
{
    return this->valueArray;
}

Key_t &ListNode :: getMin()
{
    return this->min;
}

uint8_t ListNode::getPrefixLength()
{
	return this->prefixLength;
}

bool ListNode::getDeleted() {
    return deleted;
}

uint8_t LCPlength(const char *str1, const char *str2) {
	int ret = 0;
	for (uint8_t i = 0; i < strlen(str1) && i < strlen(str2); i++) {
		if (str1[i] == str2[i]) ret++;
		else break;
	}
	return ret;
}

ListNode* ListNode :: split(Key_t &key, Val_t val, uint8_t keyHash)
{
    //Find median
    Key_t copyArray[MAX_ENTRIES];
    std::copy(std::begin(keyArray), std::end(keyArray), std::begin(copyArray));
    std::sort(std::begin(copyArray), std::end(copyArray));
    Key_t median = copyArray[MAX_ENTRIES/2];
    int newNumEntries = 0;
    Key_t newMin = median;
    ListNode* newNode = new(ListNode);
    int removeIndex = -1;
    for(int i = 0; i < MAX_ENTRIES; i++) {
        if(keyArray[i] >= median) {
            newNode->insertAtIndex(keyArray[i], valueArray[i], newNumEntries, fingerPrint[i]);
            newNumEntries++;
            removeFromIndex(i);
            if(removeIndex == -1) removeIndex = i;
        }
    }
    if (key < newMin) {
            if (removeIndex != -1)
                insertAtIndex(key, val, removeIndex, keyHash);
            else
                insertAtIndex(key, val, numEntries, keyHash);
    }
    else newNode->insertAtIndex(key, val, newNumEntries, keyHash);
    newNode->setMin(newMin);
    newNode->setNext(next);
    newNode->setDeleted(false);
    newNode->setPrev(this);
	setPrefixLength(LCPlength(newMin.getData(), min.getData()));
	newNode->setPrefixLength(LCPlength(newMin.getData(), next->getMin().getData()));

    next = newNode;
    return newNode;
}
ListNode* ListNode :: mergeWithNext()
{
    ListNode* deleteNode = next;
    if (!deleteNode->writeLock()) return nullptr;

    int cur = 0;
    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (deleteNode->getBitMap()->test(i)) {
            for (int j = cur; j < MAX_ENTRIES; j++) {
                if (!bitMap[j]) {
                    uint8_t keyHash = deleteNode->getFingerPrintArray()[i];
                    Key_t key = deleteNode->getKeyArray()[i];
                    Val_t val = deleteNode->getValueArray()[i];
                    insertAtIndex(key, val, j, keyHash);
                    deleteNode->removeFromIndex(i);
                    cur = j + 1;
                    break;
                }
            }
        }
    }
	setPrefixLength(LCPlength(next->getMin().getData(), min.getData()));
    next = deleteNode->getNext();
    next->setPrev(this);
    deleteNode->setDeleted(true);

    //deleteNode->writeUnlock();
    return deleteNode;
}

ListNode* ListNode :: mergeWithPrev()
{
    ListNode* deleteNode = this;
    ListNode* mergeNode = prev;
    if (!prev->writeLock()) return nullptr;

    int cur = 0;
    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (deleteNode->getBitMap()->test(i)) {
            for (int j = cur; j < MAX_ENTRIES; j++) {
                if (!mergeNode->getBitMap()->test(j)) {
                    uint8_t keyHash = deleteNode->getFingerPrintArray()[i];
                    Key_t key = deleteNode->getKeyArray()[i];
                    Val_t val = deleteNode->getValueArray()[i];
                    mergeNode->insertAtIndex(key, val, j, keyHash);
                    deleteNode->removeFromIndex(i);
                    cur = j + 1;
                    break;
                }
            }
        }
    }
	mergeNode->setPrefixLength(LCPlength(mergeNode->getMin().getData(), deleteNode->min.getData()));
    mergeNode->setNext(deleteNode->getNext());
    next->setPrev(mergeNode);
    deleteNode->setDeleted(true);
    mergeNode->writeUnlock();
    //deleteNode->writeUnlock();
    return deleteNode;
}
bool ListNode :: insertAtIndex(const Key_t &key, Val_t value, int index, uint8_t keyHash)
{
    keyArray[index] = key;
    valueArray[index] = value;
    fingerPrint[index] = keyHash;
    bitMap.set(index);
    numEntries++;
    return true;
}

bool ListNode :: updateAtIndex(Val_t value, int index)
{
    valueArray[index] = value;
    return true;
}

bool ListNode :: removeFromIndex(int index)
{
    bitMap.reset(index);
    numEntries--;
    return true;
}

#if MAX_ENTRIES < 8 
int ListNode :: lowerBound(Key_t key)
{
    int i;
    for(i = 0; i < numEntries; i++) {
        if (keyArray[i] >= key)
            return i;
    }
    return i;
}
#else
int ListNode :: lowerBound(Key_t key)
{
   int lower = 0;
   int upper = numEntries;
   do {
          int mid = ((upper-lower)/2) + lower;
          if (key < keyArray[mid]) {
                  upper = mid;
          } else if (key > keyArray[mid]) {
                  lower = mid + 1;
          } else {
                  return mid;
          }
   } while (lower < upper);
   return lower;
}
#endif

uint8_t ListNode :: getKeyInsertIndex(const Key_t &key)
{

    for (uint8_t i = 0; i < MAX_ENTRIES; i++) {
        if (!bitMap[i]) return i;
    }
}
#if MAX_ENTRIES == 64
int ListNode:: getKeyIndex(const Key_t &key, uint8_t keyHash) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)fingerPrint);
    __m512i v2 = _mm512_set1_epi8((int8_t)keyHash);
    uint64_t mask = _mm512_cmpeq_epi8_mask(v1, v2);
    uint64_t bitMapMask = bitMap.to_ulong();
    uint64_t posToCheck_64 = (mask) & bitMapMask;
    uint32_t posToCheck = (uint32_t) posToCheck_64;
    while(posToCheck) {
            int pos;
            asm("bsrl %1, %0" : "=r"(pos) : "r"((uint32_t)posToCheck));
            //printf("pos: %d key: %d\n", pos, key);
            if (memcmp(&keyArray[pos], key.getData(), 32) == 0)
                return pos;
            posToCheck = posToCheck & (~(1 << pos));
    }
    posToCheck = (uint32_t) (posToCheck_64 >> 32);
    while(posToCheck) {
            int pos;
            asm("bsrl %1, %0" : "=r"(pos) : "r"((uint32_t)posToCheck));
            //printf("xxxpos: %d key: %d\n", pos, key);
            if (memcmp(&keyArray[pos+32], key.getData(), 32) == 0)
            //if (keyArray[pos + 32] == key)
                return pos + 32;
            posToCheck = posToCheck & (~(1 << pos));
    }
    return -1;

}

int ListNode:: getFreeIndex(Key_t &key, uint8_t keyHash) {
    int freeIndex;
    if (numEntries != 0 && getKeyIndex(key, keyHash) != -1) return -1;
    
    uint64_t bitMapMask = bitMap.to_ulong();
    if (!(~bitMapMask)) return -2;
    
    uint64_t freeIndexMask = ~(bitMapMask);
    if ((uint32_t)freeIndexMask)
        asm("bsf %1, %0" : "=r"(freeIndex) : "r"((uint32_t)freeIndexMask));
    else {
        freeIndexMask = freeIndexMask >> 32;
        asm("bsf %1, %0" : "=r"(freeIndex) : "r"((uint32_t)freeIndexMask));
        freeIndex += 32;
    }
    return freeIndex;

}
#elif MAX_ENTRIES == 128
int ListNode:: getKeyIndex(const Key_t &key, uint8_t keyHash) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)fingerPrint);
    __m512i v2 = _mm512_set1_epi8((int8_t)keyHash);
    uint64_t mask = _mm512_cmpeq_epi8_mask(v1, v2);
    uint64_t bitMapMask = bitMap.to_ulong(0);
    uint64_t posToCheck_64 = (mask) & bitMapMask;
    uint32_t posToCheck = (uint32_t) posToCheck_64;
    while(posToCheck) {
            int pos;
            asm("bsrl %1, %0" : "=r"(pos) : "r"((uint32_t)posToCheck));
            //printf("pos: %d key: %d\n", pos, key);
            if (keyArray[pos] == key)
                return pos;
            posToCheck = posToCheck & (~(1 << pos));
    }
    posToCheck = (uint32_t) (posToCheck_64 >> 32);
    while(posToCheck) {
            int pos;
            asm("bsrl %1, %0" : "=r"(pos) : "r"((uint32_t)posToCheck));
            //printf("xxxpos: %d key: %d\n", pos, key);
            if (keyArray[pos + 32] == key)
                return pos + 32;
            posToCheck = posToCheck & (~(1 << pos));
    }
    v1 = _mm512_loadu_si512((__m512i*)&fingerPrint[64]);
    mask = _mm512_cmpeq_epi8_mask(v1, v2);
    bitMapMask = bitMap.to_ulong(1);
    posToCheck_64 = mask & bitMapMask;
    posToCheck = (uint32_t) posToCheck_64;
    while(posToCheck) {
            int pos;
            asm("bsrl %1, %0" : "=r"(pos) : "r"((uint32_t)posToCheck));
            //printf("pos: %d key: %d\n", pos, key);
            if (keyArray[pos + 64] == key)
                return pos + 64;
            posToCheck = posToCheck & (~(1 << pos));
    }
    posToCheck = (uint32_t) (posToCheck_64 >> 32);
    while(posToCheck) {
            int pos;
            asm("bsrl %1, %0" : "=r"(pos) : "r"((uint32_t)posToCheck));
            //printf("xxxpos: %d key: %d\n", pos, key);
            if (keyArray[pos + 96] == key)
                return pos + 96;
            posToCheck = posToCheck & (~(1 << pos));
    }
    return -1;

}

int ListNode:: getFreeIndex(Key_t &key, uint8_t keyHash) {
    int freeIndex;
    if (numEntries != 0 && getKeyIndex(key, keyHash) != -1) return -1;
    
    uint64_t bitMapMask_lower = bitMap.to_ulong(0);
    uint64_t bitMapMask_upper = bitMap.to_ulong(1);
    if ((~(bitMapMask_lower) == 0x0) && (~(bitMapMask_upper) == 0x0)) return -2;
    else if (~(bitMapMask_lower) != 0x0) {
    
        uint64_t freeIndexMask = ~(bitMapMask_lower);
        if ((uint32_t)freeIndexMask)
            asm("bsf %1, %0" : "=r"(freeIndex) : "r"((uint32_t)freeIndexMask));
        else {
            freeIndexMask = freeIndexMask >> 32;
            asm("bsf %1, %0" : "=r"(freeIndex) : "r"((uint32_t)freeIndexMask));
            freeIndex += 32;
        }

        return freeIndex;
    } else {
        uint64_t freeIndexMask = ~(bitMapMask_upper);
        if ((uint32_t)freeIndexMask) {
            asm("bsf %1, %0" : "=r"(freeIndex) : "r"((uint32_t)freeIndexMask));
            freeIndex += 64;
        }
        else {
            freeIndexMask = freeIndexMask >> 32;
            asm("bsf %1, %0" : "=r"(freeIndex) : "r"((uint32_t)freeIndexMask));
            freeIndex += 96;
        }
        
        return freeIndex;
    }

}
#else
int ListNode:: getKeyIndex(const Key_t &key, uint8_t keyHash) {
    int count = 0;
    for (uint8_t i = 0; i < MAX_ENTRIES; i++) {
        if (bitMap[i] && keyHash == fingerPrint[i] && keyArray[i] == key)
            return (int) i;
        if (bitMap[i]) count++;
        if (count == numEntries) break;
    }
    return -1;

}

int ListNode:: getFreeIndex(Key_t &key, uint8_t keyHash) {
    int freeIndex = -2;
    int count = 0;
    bool keyCheckNeeded = true;
    for (uint8_t i = 0; i < MAX_ENTRIES; i++) {
        if (keyCheckNeeded && bitMap[i] && keyHash == fingerPrint[i] && keyArray[i] == key)
            return -1;
        if (freeIndex == -2 && !bitMap[i]) freeIndex = i;
        if (bitMap[i]) {
           count++;
           if (count == numEntries)
               keyCheckNeeded = false;
        }
    }
    return freeIndex;

}
#endif

bool ListNode::insert(Key_t &key, Val_t value) {
    uint8_t keyHash = getKeyFingerPrint(key);
    int index = getFreeIndex(key, keyHash);
    if (index == -1) return false; // Key exitst
    if (index == -2) { //No free index
        ListNode* newNode = split(key, value, keyHash);
        ListNode* nextNode = newNode->getNext();
        nextNode->setPrev(newNode);
        Oplog::enqPerThreadLog(OpStruct::insert, newNode->getMin(), keyHash, reinterpret_cast<void*>(newNode));
        return true;
    }
    if (!insertAtIndex(key, value, (uint8_t)index, keyHash))
        return false;
    return true;
}



bool ListNode::update(Key_t &key, Val_t value) {
    uint8_t keyHash = getKeyFingerPrint(key);
    int index = getKeyIndex(key, keyHash);
    if (index == -1) return false; // Key Does not exit
    if (!updateAtIndex(value, (uint8_t)index))
        return false;
    return true;
}

bool ListNode :: remove(Key_t &key)
{
    uint8_t keyHash = getKeyFingerPrint(key);
    int index = getKeyIndex(key, keyHash);
    if (index == -1)
        return false;
    if (!removeFromIndex(index))
        return false;
    if (numEntries + next->getNumEntries() < MAX_ENTRIES/2) {
        ListNode* delNode = mergeWithNext();
        if (delNode != nullptr)
            Oplog::enqPerThreadLog(OpStruct::remove, delNode->getMin(), keyHash, reinterpret_cast<void*>(delNode));
        return true;
    }
    if (prev != NULL && numEntries + prev->getNumEntries() < MAX_ENTRIES/2) {
        ListNode* delNode = mergeWithPrev();
        if (delNode != nullptr)
            Oplog::enqPerThreadLog(OpStruct::remove, delNode->getMin(), keyHash, reinterpret_cast<void*>(delNode));
    }
    return true;
}

int ListNode :: checkRange(Key_t &key)
{
	if (prefixLength < 4) {
		if (min > key) 
			return -1;
		else if (key >= next->min)
			return 1;
		else 
			return 0;
	}
	else {
		int cmp = memcmp(key.getData(), min.getData(), prefixLength);
		if (cmp == 0) {
			int ret1 = strcmp(&(key.getData()[prefixLength]), &(min.getData()[prefixLength]));
			if (ret1 == 0) return 0;
			if (ret1 < 0) return -1;
			else {
				int ret2 = strcmp(&(key.getData()[prefixLength]), &(next->min.getData()[prefixLength]));
				if (ret2 >= 0) return 1;
				else return 0;
			}
		}
		else return cmp > 0 ? 1 : -1;
	}
}

bool ListNode :: checkRangeLookup(Key_t &key)
{
    int ne = numEntries;
    return min <= key && key <= keyArray[ne-1];
}

bool ListNode::probe(Key_t &key) {
    int keyHash = getKeyFingerPrint(key);
    int index = getKeyIndex(key, keyHash);
    if (index == -1)
        return false;
    else
        return true;
}

bool ListNode::lookup(Key_t &key, Val_t &value) {
    int keyHash = getKeyFingerPrint(key);
    int index = getKeyIndex(key, keyHash);
    if (index == -1)
        return false;
    else {
        value = valueArray[index];
        return true;
    }
}

void ListNode::print() {
    for(int i = 0; i < numEntries; i++)
        //std::cout << keyArray[i] << ", ";
    std::cout << "::";
}

bool ListNode::scan(Key_t &startKey, int range, std::vector<Val_t> &rangeVector, uint64_t writeVersion) {

    if (next == nullptr)
        return true;

    int todo = static_cast<int>(range - rangeVector.size());
    if (todo < 1) assert(0 && "ListNode:: scan: todo < 1");
    if (writeVersion > lastScanVersion) {
        generatePermuter(writeVersion);
    }
    uint8_t startIndex = 0;
    if (startKey > min) startIndex = permuterLowerBound(startKey);
    for (uint8_t i = startIndex; i < numEntries && todo > 0; i++) {
        rangeVector.push_back(valueArray[permuter[i]]);
        todo--;
    }
    return rangeVector.size() == range;
}

uint8_t ListNode::getKeyFingerPrint(const Key_t &key) {
   /* #if typeid(Key_t) == typeid(uint64_t))
        key = (~key) + (key << 18); // key = (key << 18) - key - 1;
        key = key ^ (key >> 31);
        key = (key + (key << 2)) + (key << 4);
        key = key ^ (key >> 11);
        key = key + (key << 6);
        key = key ^ (key >> 22);
        return (uint8_t) (key);
    #elsif*/

   uint32_t length = key.size();
   uint32_t hash = length;
   const char* str = key.getData();

   for (uint32_t i = 0; i < length; ++str, ++i) {
       hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
   }
   return (uint8_t) (hash & 0xFF);

}

hydra::bitset *ListNode::getBitMap() {
    return &bitMap;
}

uint8_t *ListNode::getFingerPrintArray() {
    return fingerPrint;
}
bool compare (std::pair<Key_t, uint8_t> &x, std::pair<Key_t, uint8_t> &y) {
    return x.first < y.first;
}
void ListNode::generatePermuter(uint64_t writeVersion) {
    pLock.lock();
    if (writeVersion == lastScanVersion) {
	pLock.unlock();
	return;
    }
    std::vector<std::pair<Key_t, uint8_t>> copyArray;
    for (uint8_t i = 0; i < MAX_ENTRIES; i++) {
        if (bitMap[i]) copyArray.push_back(std::make_pair(keyArray[i], i));
    }
    std::sort(copyArray.begin(), copyArray.end(), compare);
    for (uint8_t i = 0; i < copyArray.size(); i++) {
        permuter[i] = copyArray[i].second;
    }
    lastScanVersion = writeVersion;
    pLock.unlock();
    return;
}

int ListNode :: permuterLowerBound(Key_t &key)
{
    int lower = 0;
    int upper = numEntries;
    do {
        int mid = ((upper-lower)/2) + lower;
        int actualMid = permuter[mid];
        if (key < keyArray[actualMid]) {
            upper = mid;
        } else if (key > keyArray[actualMid]) {
            lower = mid + 1;
        } else {
            return mid;
        }
    } while (lower < upper);
    return (uint8_t) lower;
}
