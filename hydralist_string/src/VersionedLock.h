#ifndef _VERSIONEDLOCK_H
#define _VERSIONEDLOCK_H
#include <atomic>
#include <iostream>

#define PAUSE asm volatile("pause\n": : :"memory");
typedef unsigned long version_t;

class VersionedLock {
private:
    std::atomic<version_t> version;

public:
    VersionedLock() : version(2) {}

    version_t read_lock() {
        version_t ver = version.load(std::memory_order_acquire);
        if ((ver & 1) != 0) {
            return 0;
        }
        return ver;
    }

    version_t write_lock() {
        version_t ver = version.load(std::memory_order_acquire);
            if((ver & 1) == 0 && version.compare_exchange_weak(ver, ver+1)) {
                return ver;
            }else {
                return 0;
            }
    }

    bool read_unlock(version_t old_version) {
        std::atomic_thread_fence(std::memory_order_acquire);
        version_t new_version = version.load(std::memory_order_acquire);

        return new_version == old_version;
    }

    void write_unlock() {
        version.fetch_add(1, std::memory_order_release);
        return;
    }
};
#endif //_VERSIONEDLOCK_H
