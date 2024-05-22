// main.cpp

#ifndef DEBUG
#define DEBUG
#endif

#include <chrono>
#include <iostream>
#include <fstream>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>
//#include <utility>

#include "emulated_system_calls.hpp"
#include "fast_array_average.hpp"
#include "lightweight_1p1c_queue.hpp"


// Mutex to locally protect std::cout << ...
std::mutex cout_mutex;

using namespace std;



long random_delay(int max_milliseconds)
{
    std::mt19937_64 eng{std::random_device{}()};
    std::uniform_int_distribution<> dist{1, max_milliseconds};
    auto delay = dist(eng);
    std::this_thread::sleep_for(std::chrono::milliseconds{delay});
    //std::this_thread::sleep_for(std::chrono::milliseconds{dist(eng)});
    return delay;
}



class Test1 {
public:
    Test1() {}

    Test1(std::string &&arg_a, std::string &&arg_b) {
        a = std::move(arg_a);
        b = std::move(arg_b);
    }

    Test1(Test1 &&arg) {
        a = std::move(arg.a);
        b = std::move(arg.b);
    }

    Test1& operator=(Test1&& other) {
        a = std::move(other.a);
        b = std::move(other.b);
        return *this;
    }

    std::string str() {
        stringstream stream;
        stream << "a='" << a << "', b='" << b << "'";
        return stream.str();
    }

    std::string a, b;
};


const unsigned char QUEUE_SIZE = 4;
using LightweightQueue = Lightweight_1P1C_Queue<Test1, QUEUE_SIZE>;


/***
int test_lightweight_1p1c_queue()
{
    cout << "Running test_lightweight_1p1c_queue()." << endl;

    // The producer starts with 'QUEUE_SIZE' elements available that can be pushed onto the queue.
    struct tskTaskControlBlock producerTask(QUEUE_SIZE, "producer");
    producerTask.indexToNotify = 1;

    // The consumer start with zero elements that can be popped from the queue.
    struct tskTaskControlBlock consumerTask(0, "consumer");
    consumerTask.indexToNotify = 2;

    //const UBaseType_t producerIndexToNotify = 1, consumerIndexToNotify = 1;

    // A Queue containing 'Test1' objects.
    //Lightweight_1P1C_Queue<Test1, 4>
    LightweightQueue test1_queue(
        &producerTask, producerTask.indexToNotify,
        &consumerTask, consumerTask.indexToNotify
    );
    Test1 test1a("abc", "123");
    cout << "test1a before producer_push_back(): " << test1a.str() << endl;
    test1_queue.producer_push_back(move(test1a));
    cout << "test1a after producer_push_back(): " << test1a.str() << endl;

    Test1 test1b;
    cout << "test1b before consumer_pop_front(): " << test1b.str() << endl;
    test1_queue.consumer_pop_front(move(test1b));
    //test1b = move(test1_queue.consumer_pop_front());
    cout << "test1b after consumer_pop_front(): " << test1b.str() << endl;

    // A Queue containing 'std::string' objects.
    Lightweight_1P1C_Queue<string, 4> queue(
        &producerTask, producerTask.indexToNotify,
        &consumerTask, consumerTask.indexToNotify
    );
    string a1 = "abc", b1 = "123";
    queue.producer_push_back(move(a1));
    queue.producer_push_back(move(b1));
    cout << "a1 = " << a1 << endl
         << "b1 = " << b1 << endl;

    string a2, b2;
    queue.consumer_pop_front(move(a2));
    queue.consumer_pop_front(move(b2));
    cout << "a2 = " << a2 << endl
         << "b2 = " << b2 << endl;

    cout << "test_lightweight_1p1c_queue() DONE." << endl;
    return 0;
}
***/



void producer(TaskHandle_t taskControlBlock, LightweightQueue &test1_queue)
{
    setTaskControlBlock(taskControlBlock);
    std::this_thread::sleep_for(2000ms);

    Test1 test1a("abc", "123");

    for (int i = 0; i < 10; ++i) {
        long delay = random_delay(1000);
        {   std::lock_guard<decltype(cout_mutex)> lock(cout_mutex);
            cout << "test1a before producer_push_back(): " << test1a.str() << endl;
        }
        test1_queue.producer_push_back(move(test1a));

        // ... for the next iteration.
        test1a.a = std::to_string(i);
        test1a.b = std::to_string(delay);
    }

    //------
    // Done
    //------
    test1a.a = "DONE";
    test1_queue.producer_push_back(move(test1a));
    {   std::lock_guard<decltype(cout_mutex)> lock(cout_mutex);
        cout << "Producer DONE." << endl;
    }
}



void consumer(TaskHandle_t taskControlBlock, LightweightQueue &test1_queue)
{
    std::string doneStr("DONE");
    setTaskControlBlock(taskControlBlock);
    std::this_thread::sleep_for(2000ms);

    Test1 test1b;
    int failsafe = 0;
    do {
        ++failsafe;
        random_delay(200);
        test1_queue.consumer_pop_front(move(test1b));
        {   std::lock_guard<decltype(cout_mutex)> lock(cout_mutex);
            cout << "test1b consumer_pop_front(): " << test1b.str() << endl;
        }
    } while(test1b.a != doneStr && failsafe < 16);

    {   std::lock_guard<decltype(cout_mutex)> lock(cout_mutex);
        cout << "Consumer DONE." << endl;
    }
}



int test_lightweight_queue()
{
    cout << "Starting test_lightweight_queue()." << endl;

    // The producer starts with 'QUEUE_SIZE' elements available that can be pushed onto the queue.
    struct tskTaskControlBlock producerTask(0, "Producer"); //(QUEUE_SIZE);
    producerTask.indexToNotify = 1;

    // The consumer start with zero elements that can be popped from the queue.
    struct tskTaskControlBlock consumerTask(0, "Consumer");
    consumerTask.indexToNotify = 2;

    LightweightQueue test1_queue(
        &producerTask, producerTask.indexToNotify,
        &consumerTask, consumerTask.indexToNotify
    );

    std::thread producer_thread(producer, &producerTask, std::ref(test1_queue));
    std::thread consumer_thread(consumer, &consumerTask, std::ref(test1_queue));

    {   std::lock_guard<decltype(cout_mutex)> lock(cout_mutex);
        cout << "Waiting for threads to complete." << endl;
    }
    producer_thread.join();
    consumer_thread.join();
    cout << "Queue Threads completed." << endl;

    return 0;
}



int test_fast_array_average()
{
    cout << endl << "Starting test_fast_array_average()." << endl;

    using LongAverageTest = FastArrayAverage<long, long long, 3>;
    using UShortAverageTest = FastArrayAverage<unsigned short, unsigned long, 3>;

    cout << "LongAverageTest" << endl;
    LongAverageTest long_avg(3);
    LongAverageTest::ValueArrayType long_sample_values, long_average_values;
    for (int count = 0; count <= 9; ++count) {
        long_sample_values[0] = 1;
        long_sample_values[1] = -count;
        long_sample_values[2] = count * 10;
        long_avg.add_array_values(long_sample_values);
        long_avg.debug_stream(cout);

        long_avg.get_average_values(long_average_values);
        cout << endl;
    }
    cout << endl;

    cout << "UShortAverageTest" << endl;
    UShortAverageTest ushort_avg(3);
    UShortAverageTest::ValueArrayType ushort_sample_values, ushort_average_values;
    for (int count = 0; count <= 9; ++count) {
        ushort_sample_values[0] = 1;
        ushort_sample_values[1] = count;
        ushort_sample_values[2] = count * 10;
        ushort_avg.add_array_values(ushort_sample_values);
        ushort_avg.debug_stream(cout);

        ushort_avg.get_average_values(ushort_average_values);
        cout << endl;
    }
    cout << endl;

    cout << "Finished test_fast_array_average()." << endl << endl;
    return 0;
}



int main()
{
    cout << "Run Snippet Tests." << endl;

    //test_lightweight_1p1c_queue();
    //test_lightweight_queue();
    test_fast_array_average();

    return 0;
}
