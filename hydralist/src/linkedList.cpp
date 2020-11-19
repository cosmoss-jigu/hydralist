#include <climits>
#include <iostream>
#include <cassert>
#include "linkedList.h"
#include <chrono>
#include <thread>
#define cpu_relax() asm volatile("pause\n" : : : "memory")
//std::atomic<int> numSplits;


ListNode* LinkedList::initialize() {
    ListNode* head = new(ListNode);
    ListNode* tail = new(ListNode);

    head->insert(0,0);
    head->setNext(tail);
    head->setPrev(nullptr);
    head->setMin(0);
    head->setMax(ULLONG_MAX);

    tail->insert(ULLONG_MAX, 0);
    tail->setNumEntries(MAX_ENTRIES); //This prevents merge of tail
    tail->setNext(nullptr);
    tail->setPrev(tail);
    tail->setMin(ULLONG_MAX);
    this->head = head;

    return head;
}

bool LinkedList::insert(Key_t key, Val_t value, ListNode* head) {
    int retryCount = 0;
    restart:
    ListNode* cur = head;

    while (1) {
        if (cur->getMin() > key) {
            cur = cur->getPrev();
            continue;
        }
        if (!cur->checkRange(key)) {
            cur = cur->getNext();
            continue;
        }
        break;
    }

    if (!cur->writeLock()){
	if (retryCount > 20){
	    for (int i = 0; i < retryCount*5; i++) {
	        cpu_relax();
	        std::atomic_thread_fence(std::memory_order_seq_cst);
	    }
	}
	retryCount++;
        goto restart;
    }
    if (cur->getDeleted()) {
        cur->writeUnlock();
        goto restart;
    }
    if (!cur->checkRange(key)) {
        cur->writeUnlock();
        goto restart;
    }
    bool ret = cur->insert(key, value);
    cur->writeUnlock();
    return ret;
}

bool LinkedList::update(Key_t key, Val_t value, ListNode* head) {
    int retryCount = 0;
    restart:
    ListNode* cur = head;

    while (1) {
        if (cur->getMin() > key) {
            cur = cur->getPrev();
            continue;
        }
        if (!cur->checkRange(key)) {
            cur = cur->getNext();
            continue;
        }
        break;
    }

    if (!cur->writeLock()){
	if (retryCount > 20){
	    for (int i = 0; i < retryCount*7; i++) {
	        cpu_relax();
	        std::atomic_thread_fence(std::memory_order_seq_cst);
	    }
	}
	retryCount++;
        goto restart;
    }
    if (cur->getDeleted()) {
        cur->writeUnlock();
        goto restart;
    }
    if (!cur->checkRange(key)) {
        cur->writeUnlock();
        goto restart;
    }
    bool ret = cur->update(key, value);
    cur->writeUnlock();
    return ret;
}

bool LinkedList::remove(Key_t key, ListNode *head) {
    restart:
    ListNode* cur = head;

    while (1) {
        if (cur->getMin() > key) {
            cur = cur->getPrev();
            continue;
        }
        if (!cur->checkRange(key)) {
            cur = cur->getNext();
            continue;
        }
        break;
    }
    if (!cur->writeLock()) {
        if (head->getPrev() != nullptr) head = head->getPrev();
        goto restart;
    }
    if (cur->getDeleted()) {
        if (head->getPrev() != nullptr) head = head->getPrev();
        cur->writeUnlock();
        goto restart;
    }
    if (!cur->checkRange(key)) {
        if (head->getPrev() != nullptr) head = head->getPrev();
        cur->writeUnlock();
        goto restart;
    }

    bool ret = cur->remove(key);
    cur->writeUnlock();
    return ret;
}

bool LinkedList::probe(Key_t key, ListNode *head) {
    restart:
    ListNode* cur = head;
    //int count = 0;
    //if(cur->getMin() > key)
    //    std::atomic_fetch_add(&numSplits, 1);

    while (1) {
        if (cur->getMin() > key) {
            cur = cur->getPrev();
            continue;
        }
        if (!cur->checkRange(key)) {
            cur = cur->getNext();
            continue;
        }
        break;
    }

    version_t readVersion = cur->readLock();
    //Concurrent Update
    if (!readVersion) {
        goto restart;
    }
    if (cur->getDeleted()){
        goto restart;
    }
    if (!cur->checkRange(key)) {
        goto restart;
    }
    bool ret = false;
    ret = cur->probe(key);
    if (!cur->readUnlock(readVersion))
        goto restart;
    return ret;
}

bool LinkedList::lookup(Key_t key, Val_t &value, ListNode *head) {
    int retryCount = 0;
    restart:
    ListNode* cur = head;
    int count = 0;
    //if(cur->getMin() > key)
    //    std::atomic_fetch_add(&numSplits, 1);

    while (1) {
        if (cur->getMin() > key) {
            cur = cur->getPrev();
            continue;
        }
        if (!cur->checkRange(key)) {
            cur = cur->getNext();
            continue;
        }
        break;
    }

    version_t readVersion = cur->readLock();
    //Concurrent Update
    if (!readVersion){
	if (retryCount > 20){
	    for (int i = 0; i < retryCount*5; i++) {
	        cpu_relax();
	        std::atomic_thread_fence(std::memory_order_seq_cst);
	    }
	}
	retryCount++;
        goto restart;
    }
    if (cur->getDeleted()){
        goto restart;
    }
    if (!cur->checkRange(key)) {
        goto restart;
    }
    bool ret = false;
    ret = cur->lookup(key, value);
    if (!cur->readUnlock(readVersion))
        goto restart;
    return ret;
}

void LinkedList::print(ListNode *head) {
    ListNode* cur = head;
    while (cur->getNext() != nullptr) {
        cur->print();
        cur = cur->getNext();
    }
    std::cout << "\n";
    return;
}

uint32_t LinkedList::size(ListNode *head) {
    ListNode* cur = head;
    int count = 0;
    while (cur->getNext() != nullptr) {
        count++;
        cur = cur->getNext();
    }
    return count;
}

ListNode *LinkedList::getHead() {
    return head;
}

uint64_t LinkedList::scan(Key_t startKey, int range, std::vector<Val_t> &rangeVector, ListNode *head) {
    restart:
    ListNode* cur = head;
    rangeVector.clear();
    // Find the start Node
    while (1) {
        if (cur->getMin() > startKey) {
            cur = cur->getPrev();
            continue;
        }
        if (!cur->checkRange(startKey)) {
            cur = cur->getNext();
            continue;
        }
        break;
    }
    bool end = false;
    assert(rangeVector.size() == 0);
    while (rangeVector.size() < range && !end) {
        version_t readVersion = cur->readLock();
        //Concurrent Update
        if (!readVersion)
            goto restart;
        if (cur->getDeleted()){
            goto restart;
        }
        end = cur->scan(startKey, range, rangeVector, readVersion);
        if(!cur->readUnlock(readVersion))
            goto restart;
        cur = cur->getNext();
    }
    return rangeVector.size();
}

