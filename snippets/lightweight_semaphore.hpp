// lightweight_semaphore.hpp

#ifndef _LIGHTWEIGHT_SEMAPHORE_HPP_
#define _LIGHTWEIGHT_SEMAPHORE_HPP_

#include <mutex>
#include <condition_variable>
//#include <stdint.h>
#include <sstream>


// based on:
// https://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads
class LightweightSemaphore {
public:
    // MUST BE SOME FORM OF UNSIGNED INTEGER!
    using CountType = unsigned; //uint32_t;

    LightweightSemaphore(CountType count = 0, std::string semaphoreName = "???") :
        count_(count),
        semaphoreName(semaphoreName)
    { }

    // Increase the semaphore count so that the next take(), or currently blocked take(), will un-block.
    // a.k.a. release, notify, ...
    void give(const CountType count = 1) {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        //TODO: implement and test for a maximum 'count' value.
        //TODO: test for (count < 1)
        count_ += count;
        condition_.notify_one();
    }

    // Decrement the semaphore count.
    // If the count was at zero then block until some other thread has called give(), which increments the count.
    // a.k.a. aquire, wait, ...
    CountType take() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);

        // Alternate syntax:
        //condition_.wait(lock, [&]{ return count_ > 0; });
        //
        // Handle spurious wake-ups.
        while(count_ == 0) {
            condition_.wait(lock);
        }

        CountType previous_count = count_;
        --count_;
        return previous_count;
    }

    // TODO: uncomment the following only if it is really needed.
    // a.k.a. try_aquire, try_wait, ...
    // Non-blocking take.
    // bool try_take() {
    //     std::lock_guard<decltype(mutex_)> lock(mutex_);
    //     if(count_ > 0) {
    //         --count_;
    //         return true;
    //     }
    //     return false;
    // }

    std::string debug_str() {
        std::stringstream stream;
        {
            std::lock_guard<decltype(mutex_)> lock(mutex_);
            stream << "semaphore '" << semaphoreName << "', count=" << count_;
        }
        return stream.str();
    }

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::string semaphoreName;
    CountType count_;
};


#endif // _LIGHTWEIGHT_SEMAPHORE_HPP_
