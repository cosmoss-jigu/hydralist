#include "main.h"
#include <hot/rowex/HOTRowex.hpp>
#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/OptionalValue.hpp>



typedef struct IntKeyVal {
    uint64_t key;
    uintptr_t value;
} IntKeyVal;

template<typename ValueType = IntKeyVal *>
class IntKeyExtractor {
    public:
    typedef uint64_t KeyType;

    inline KeyType operator()(ValueType const &value) const {
	        return value->key;
	    }
};

typedef struct HOTKey {
	uint64_t value;
	size_t key_len;
	uint8_t fkey[];

	inline HOTKey *make_leaf(char *key, size_t key_len, uint64_t value);

	inline size_t getKeyLen() const;
} HOTKey;

inline HOTKey *HOTKey::make_leaf(char *key, size_t key_len, uint64_t value)
{
	void *aligned_alloc;
	posix_memalign(&aligned_alloc, 64, sizeof(HOTKey) + key_len);
	HOTKey *k = reinterpret_cast<HOTKey *> (aligned_alloc);

	k->value = value;
	k->key_len = key_len;
	memcpy(k->fkey, key, key_len);

	return k;
}
inline size_t HOTKey::getKeyLen() const { return key_len; }
template<typename ValueType = HOTKey *>
class KeyExtractor {
	public:
		typedef char const * KeyType;

		inline KeyType operator()(ValueType const &value) const {
			return (char const *)value->fkey;
		}
};



class HOTimpl {
	using TrieType = hot::rowex::HOTRowex<IntKeyVal *, IntKeyExtractor>;
	TrieType mTrie;
	public:
		inline bool insert(uint64_t key, uint64_t val) {
			IntKeyVal *key1;
			posix_memalign((void **)&key1, 64, sizeof(IntKeyVal));
			key1->key = key;
			key1->value = (uintptr_t)val;
			return mTrie.insert(key1);
		}


		inline bool lookup(uint64_t key) {
			idx::contenthelpers::OptionalValue<IntKeyVal *> result = mTrie.lookup(key);
			if (result.mIsValid) {
				return 1;
			} else return 0;
		}

		void scan(uint64_t key, uint64_t range) {
			mTrie.scan(key, range);
			return;
		}
};

class HOTimplString {
	using TrieType = hot::rowex::HOTRowex<HOTKey *, KeyExtractor>;
	TrieType mTrie;
	public:
		inline bool insert(char* key, int keylen, uint64_t val) {
			HOTKey *key1 = key1->make_leaf((char *) key, keylen, val);
			return mTrie.insert(key1);
		}


		inline bool lookup(char* key) {
			idx::contenthelpers::OptionalValue<HOTKey *> result = mTrie.lookup(key);
			if (result.mIsValid) {
				return 1;
			} else return 0;
		}

		void scan(char* key, uint64_t range) {
			mTrie.scan(key, range);
			return;
		}
};
HOTimpl xxx;
HOTimplString yyy;
bool HOTidx::insert(uint64_t key, uint64_t val) {
	return xxx.insert(key, val);
}

bool HOTidx::remove(uint64_t key){
	return true;
}

bool HOTidx::lookup(uint64_t key) {
	return xxx.lookup(key);
}

void HOTidx::scan(uint64_t key, uint64_t range) {
	 xxx.scan(key,range);
	 return;
}

bool HOTidxString::insert(char* key, size_t keylen, uint64_t val) {
	return yyy.insert(key, keylen, val);
}

bool HOTidxString::remove(char* key){
	return true;
}

bool HOTidxString::lookup(char* key) {
	return yyy.lookup(key);
}

void HOTidxString::scan(char* key, uint64_t range) {
	 yyy.scan(key,range);
	 return;
}

