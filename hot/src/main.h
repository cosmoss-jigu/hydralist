#ifndef __MAIN_H__
#define __MAIN_H__

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>

class HOTidx {
public:
	bool insert( uint64_t key, uint64_t val);

	bool remove( uint64_t key);

	bool lookup( uint64_t key);

	void scan (uint64_t key, uint64_t range);

};

class HOTidxString {
public:
	bool insert( char* key, size_t keylen, uint64_t val);

	bool remove(char* key);

	bool lookup(char* key);

	void scan (char* key, uint64_t range);

};

#endif
