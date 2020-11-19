//
// Created by ajit on 3/7/19.
//

#ifndef HYDRALIST_COMMON_H
#define HYDRALIST_COMMON_H
#include <cstdint>
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>

#define MAX_NUMA 8
#define KEYLENGTH 32
#define WORKER_THREAD_PER_NUMA 1
//#define HYDRALIST_ENABLE_STATS

template <std::size_t keySize>
class StringKey {
private:
    char data[keySize];
    size_t keyLength = 0;
public:
    StringKey() { memset(data, 0x00, keySize);}
    StringKey(const StringKey &other) {
        memcpy(data, other.data, keySize);
    }
    StringKey(const char bytes[]) {set(bytes, strlen(bytes));}
    StringKey(int k) {
        setFromString(std::to_string(k));
    }
    inline StringKey &operator=(const StringKey &other) {
        memcpy(data, other.data, keySize);
        return *this;
    }
    inline bool operator<(const StringKey<keySize> &other) { return strcmp(data, other.data) < 0;}
    inline bool operator>(const StringKey<keySize> &other) { return strcmp(data, other.data) > 0;}
    inline bool operator==(const StringKey<keySize> &other) { return strcmp(data, other.data) == 0;}
    inline bool operator!=(const StringKey<keySize> &other) { return !(*this == other);}
    inline bool operator<=(const StringKey<keySize> &other) { return !(*this > other);}
    inline bool operator>=(const StringKey<keySize> &other) {return !(*this < other);}

    size_t size() const {
        if (keyLength)
            return keyLength;
        else
            return strlen(data);
    }

    inline void setFromString(std::string key) {
        memset(data, 0, keySize);
        if (key.size() >= keySize) {
            memcpy(data, key.c_str(), keySize - 1);
            data[keySize - 1] = '\0';
            keyLength = keySize;
        } else {
            strcpy(data, key.c_str());
            keyLength = key.size();
        }
        return;
    }

    inline void set(const char bytes[], const std::size_t length) {
        memset(data, 0, keySize);
        //assert(length <= keySize);
        memcpy(data, bytes, length);
    }
    const char* getData() const { return data;}
    //friend ostream & operator << (ostream &out, const StringKey<keySize> &k);
};

typedef StringKey<KEYLENGTH> Key_t;
typedef uint64_t Val_t;

class OpStruct {
public:
    enum Operation {insert, remove};
    Operation op;
    Key_t key;
    uint8_t hash;
    void* listNodePtr;
    uint64_t ts;
    bool operator< (const OpStruct& ops) const {
        return (ts < ops.ts);
    }
};

#endif //HYDRALIST_COMMON_H
