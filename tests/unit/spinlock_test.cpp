#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "core/spinlock.h"

TEST(SpinLockTest, SpinlockBasic) {
    SpinLock lock;

    {
        Guard guard(lock);
        EXPECT_FALSE(lock.try_lock());
        EXPECT_TRUE(lock.is_locked());
    }
    EXPECT_FALSE(lock.is_locked());
    EXPECT_TRUE(lock.try_lock());
    lock.unlock();
    EXPECT_TRUE(lock.try_lock());
}

TEST(SpinLockTest, SpinlockMultithread) {
    SpinLock lock;

    std::vector<std::thread> threads;

    long count = 0;

    {
        Guard guard(lock);

        auto fn = [&]() {
            for (int i = 0; i < 1000000; i++) {
                {
                    Guard guard_inner(lock);
                    count++;
                }
            }
        };

        threads.push_back(std::thread(fn));
        threads.push_back(std::thread(fn));
    }

    for (auto thread = threads.begin(); thread != threads.end(); thread++) {
        thread->join();
    }

    EXPECT_EQ(count, 2000000);
}
