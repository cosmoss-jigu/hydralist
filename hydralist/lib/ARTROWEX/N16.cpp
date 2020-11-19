#include <assert.h>
#include <algorithm>
#include "N.h"
#include <emmintrin.h> // x86 SSE intrinsics

namespace ART_ROWEX {

    bool N16::insert(uint8_t key, N *n) {
        if (compactCount == 16) {
            return false;
        }
        keys[compactCount].store(flipSign(key), std::memory_order_release);
        children[compactCount].store(n, std::memory_order_release);
        compactCount++;
        count++;
        return true;
    }

    template<class NODE>
    void N16::copyTo(NODE *n) const {
        for (unsigned i = 0; i < compactCount; i++) {
            N *child = children[i].load();
            if (child != nullptr) {
                n->insert(flipSign(keys[i]), child);
            }
        }
    }

    void N16::change(uint8_t key, N *val) {
        auto childPos = getChildPos(key);
        assert(childPos != nullptr);
        return childPos->store(val, std::memory_order_release);
    }

    std::atomic<N *> *N16::getChildPos(const uint8_t k) {
        __m128i cmp = _mm_cmpeq_epi8(_mm_set1_epi8(flipSign(k)),
                                     _mm_loadu_si128(reinterpret_cast<const __m128i *>(keys)));
        unsigned bitfield = _mm_movemask_epi8(cmp) & ((1 << compactCount) - 1);
        while (bitfield) {
            uint8_t pos = ctz(bitfield);

            if (children[pos].load() != nullptr) {
                return &children[pos];
            }
            bitfield = bitfield ^ (1 << pos);
        }
        return nullptr;
    }

    N *N16::getChild(const uint8_t k) const {
        __m128i cmp = _mm_cmpeq_epi8(_mm_set1_epi8(flipSign(k)),
                                     _mm_loadu_si128(reinterpret_cast<const __m128i *>(keys)));
        unsigned bitfield = _mm_movemask_epi8(cmp) & ((1 << 16) - 1);
        while (bitfield) {
            uint8_t pos = ctz(bitfield);

            N *child = children[pos].load();
            if (child != nullptr && keys[pos].load() == flipSign(k)) {
                return child;
            }
            bitfield = bitfield ^ (1 << pos);
        }
        return nullptr;
    }

    bool N16::remove(uint8_t k, bool force) {
        if (count == 3 && !force) {
            return false;
        }
        auto leafPlace = getChildPos(k);
        assert(leafPlace != nullptr);
        leafPlace->store(nullptr, std::memory_order_release);
        count--;
        assert(getChild(k) == nullptr);
        return true;
    }

    N *N16::getAnyChild() const {
        N *anyChild = nullptr;
        for (int i = 0; i < 16; ++i) {
            N *child = children[i].load();
            if (child != nullptr) {
                if (N::isLeaf(child)) {
                    return child;
                }
                anyChild = child;
            }
        }
        return anyChild;
    }
    
    N *N16::getAnyChildReverse() const {
        N *anyChild = nullptr;
        for (int i = 15; i >= 0; --i) {
            N *child = children[i].load();
            if (child != nullptr) {
                if (N::isLeaf(child)) {
                    return child;
                }
                anyChild = child;
            }
        }
        return anyChild;
    }

    void N16::deleteChildren() {
        for (std::size_t i = 0; i < compactCount; ++i) {
            if (children[i].load() != nullptr) {
                N::deleteChildren(children[i]);
                N::deleteNode(children[i]);
            }
        }
    }

    void N16::getChildren(uint8_t start, uint8_t end, std::tuple<uint8_t, N *> *&children,
                          uint32_t &childrenCount) const {
        childrenCount = 0;
        for (int i = 0; i < compactCount; ++i) {
            uint8_t key = flipSign(this->keys[i]);
            if (key >= start && key <= end) {
                N *child = this->children[i].load();
                if (child != nullptr) {
                    children[childrenCount] = std::make_tuple(key, child);
                    childrenCount++;
                }
            }
        }
        std::sort(children, children + childrenCount, [](auto &first, auto &second) {
            return std::get<0>(first) < std::get<0>(second);
        });
    }
    N *N16::getSmallestChild(uint8_t start) const {
        N *smallestChild = nullptr;
        uint8_t minKey = 255;
        for (int i = 0; i < compactCount; ++i) {
            uint8_t key = flipSign(this->keys[i]);
            if (key >= start && key <= minKey) {
                N *child = this->children[i].load();
                if (child != nullptr) {
                    minKey = key;
                    smallestChild = child;
                }
            }
        }
        return smallestChild;
    }
    N *N16::getLargestChild(uint8_t end) const {
        N *largestChild = nullptr;
        uint8_t maxKey = 0;
        for (int i = 0; i < compactCount; ++i) {
            uint8_t key = flipSign(this->keys[i]);
            if (key <= end && key >= maxKey) {
                N *child = this->children[i].load();
                if (child != nullptr) {
                    maxKey = key;
                    largestChild = child;
                }
            }
        }
        return largestChild;
    }
}
