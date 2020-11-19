#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H
#include "listNode.h"

enum Operation  {lt, gt};

class LinkedList {
private:
    ListNode* head;

public:
    ListNode* initialize();
    bool insert(Key_t key, Val_t value, ListNode* head);
    bool update(Key_t key, Val_t value, ListNode* head);
    bool remove(Key_t key, ListNode* head);
    bool probe(Key_t key, ListNode* head);
    bool lookup(Key_t key, Val_t &value, ListNode* head);
    uint64_t scan(Key_t startKey, int range, std::vector<Val_t> &rangeVector, ListNode *head);
    void print(ListNode *head);
    uint32_t size(ListNode* head);
    ListNode* getHead();

};


#endif //_LINKEDLIST_H
