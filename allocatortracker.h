#include <cstdio>
#include <typeinfo>
#include <vector>
#include <cstring>
#include <map>
#include <string>
#include <cstdlib>
#include <iostream>

#ifndef ALLOCATOR_TRACKER_H
#define ALLOCATOR_TRACKER_H

template<typename ValueType> 
/**
 * Custom allocator for all the indexes except array index.
 */
class AllocatorTracker : public std::allocator<ValueType> {
public:
  typedef typename std::allocator<ValueType> BaseAllocator;
  typedef typename BaseAllocator::pointer pointer;
  typedef typename BaseAllocator::size_type size_type;

  // that's what we passed through copy constructor
  // all the stl and other stuff only use copy constructor
  int64_t *memory_size;

  // This shouldn't be called. We should call the constructor with pointer to the memory_size.
  AllocatorTracker() throw() : BaseAllocator() {}

  AllocatorTracker(int64_t* m_ptr) throw() : BaseAllocator() {
    memory_size = m_ptr;
  }
  AllocatorTracker(const AllocatorTracker& allocator) throw() : BaseAllocator(allocator) {
    memory_size = allocator.memory_size;
  }
  template <class U> AllocatorTracker(const AllocatorTracker<U>& allocator) throw(): BaseAllocator(allocator) {
    memory_size = allocator.memory_size;
  }

  ~AllocatorTracker() {}

  template<class U> struct rebind {
    typedef AllocatorTracker<U> other;
  };

  pointer allocate(size_type size) {
    pointer dataPtr = BaseAllocator::allocate(size);
    *memory_size += size * sizeof(ValueType);
    //VOLT_TRACE("allocate +++++++ %p %lu.\n", dataPtr, size * sizeof(ValueType));
    //VOLT_TRACE("%s\n", typeid(ValueType).name());
    return dataPtr;
  }

  pointer allocate(size_type size, void* ptr) {
    pointer dataPtr = BaseAllocator::allocate(size, ptr);
    *memory_size += size * sizeof(ValueType);
    //VOLT_TRACE("allocate +++++++ %p %lu.\n", dataPtr, size * sizeof(ValueType));
    //VOLT_TRACE("%s\n", typeid(ValueType).name());
    return dataPtr;
  }

  pointer allocate(size_type size, pointer ptr) {
    pointer dataPtr = BaseAllocator::allocate(size, ptr);
    *memory_size += size * sizeof(ValueType);
    //VOLT_TRACE("allocate +++++++ %p %lu.\n", dataPtr, size * sizeof(ValueType));
    //VOLT_TRACE("%s\n", typeid(ValueType).name());
    return dataPtr;
  }

  void deallocate(pointer ptr, size_type size) throw() {
    BaseAllocator::deallocate(ptr, size);
    *memory_size -= size * sizeof(ValueType);
  }
  /*
  void construct(pointer __ptr, const ValueType& __val) {
    new(__ptr) ValueType(__val);
    //VOLT_TRACE("construct +++++++ %p %lu.\n", __ptr, sizeof(ValueType));
    //VOLT_TRACE("%s\n", typeid(ValueType).name());
    *memory_size += sizeof(ValueType);
  }

  void destroy(pointer __ptr) {
    //VOLT_TRACE("-+-+-+- %08x.\n", __ptr);
    __ptr->~ValueType();
    *memory_size -= sizeof(ValueType);
  }
  */
};

#endif
