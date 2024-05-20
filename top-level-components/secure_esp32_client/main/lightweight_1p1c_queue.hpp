// lightweight_pc_queue.hpp

//TODO: ?? create mediumweight_pc_queue.hpp using C++ Semaphores. ??


#ifndef _LIGHTWEIGHT_1P1C_QUEUE_HPP_
#define _LIGHTWEIGHT_1P1C_QUEUE_HPP_

#include <utility>

#ifdef EMULATE_SYSTEM_CALLS
#  include "emulated_system_calls.hpp"
#else
#  include "freertos/FreeRTOS.h"
#  include "freertos/task.h"
#endif


/*
Required Features:
 - Light Weight
 - Fixed Size
 - Thread Safe
 - Single Producer
 - Single Consumer
 - std::move assignment only
 - queue size MUST be less than 256

NOTE:
 - <T> must conform to std::move constructor and assignment,
   and default constructor.

References:
https://www.freertos.org/Inter-Task-Communication.html
https://www.freertos.org/xTaskNotifyGive.html
https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html
https://www.freertos.org/taskENTER_CRITICAL_taskEXIT_CRITICAL.html

https://www.freertos.org/Documentation/RTOS_book.html
https://github.com/FreeRTOS/FreeRTOS-Kernel-Book/blob/main/toc.md
*/
template<class T, unsigned char n>
class Lightweight_1P1C_Queue {
private:
    class Node {
    public:
        Node() {}

        Node(T&& arg_data) noexcept {
            //is_empty = false;
            data = std::move(arg_data);
        }

        Node(Node&& arg) noexcept {
            //is_empty = std::move(arg.is_empty);
            data = std::move(arg.data);
        }

        // Node(T&& arg_data) : is_empty(false), data(std::move(arg_data)) { }
        // Node(Node&& arg) : is_empty(std::move(arg.is_empty)), data(std::move(arg.data)) { }

        Node& operator=(Node&& other) noexcept {
            data = std::move(other.data);
            //is_empty = std::move(other.is_empty);
            return *this;
        }

        T&& move_data() noexcept {
            return std::move(data);
        }

    private:
        //TODO: ?? rename to 'has_value' ??
        volatile bool is_empty = true;
        T data;
    };

    using IndexT = unsigned char;
    IndexT front_index = 0, back_index = 0;
    Node queue[n];

    const TaskHandle_t producerTask, consumerTask;
    const UBaseType_t producerIndexToNotify, consumerIndexToNotify;


    void increment(IndexT &index) {
        if (index >= n-1) {
            index = 0;
        } else {
            ++index;
        }
    }


public:
    Lightweight_1P1C_Queue(
        const TaskHandle_t producerTask,
        const UBaseType_t producerIndexToNotify,
        const TaskHandle_t consumerTask,
        const UBaseType_t consumerIndexToNotify
    ) :
        producerTask(producerTask),
        producerIndexToNotify(producerIndexToNotify),
        consumerTask(consumerTask),
        consumerIndexToNotify(consumerIndexToNotify)
    {
        BaseType_t err;

        //TODO: assert - FAIL if producerTask and consumerTask are the same task!!!

        // Set the Direct Task Notifications to the required initial states.
        // The Producer needs to know how much space is available in the queue
        //  so initialize it to 'n'.
        err = xTaskNotifyIndexed(producerTask, producerIndexToNotify, n, eSetValueWithoutOverwrite);
        if (err != pdPASS) {
            // TODO: handle error state!
        }

        // The Consumer needs to know how many items are currently in the queue
        //  so initialize it to zero.
        err = xTaskNotifyIndexed(consumerTask, consumerIndexToNotify, 0, eSetValueWithoutOverwrite);
        if (err != pdPASS) {
            // TODO: handle error state!
        }
    }


    /*
    This function MUST be called from the Task specified by this->producerTask.
    Blocks infinitely when the queue is full.
    Argument must conform to std::move.
    */
    void producer_push_back(T&& data) {
        uint32_t available_space;

        do {
            // NOTE: deadlock will occur here if the producer and the consumer or running in the same task!
            // This function MUST have been called from the Task specified by this->producerTask.
            //
            // If there is no space left in the queue then this system call will block until
            //  the consumer has popped something off the queue, making space available.
            //
            // available_space will be value of the task's notification value before it is decremented.
            // ie. how much space was available in the queue when this call unblocked.
            available_space = ulTaskNotifyTakeIndexed(producerIndexToNotify, pdFALSE, portMAX_DELAY);
            if (available_space <= 0) {
                // Either the system call just above timed out, or something bad happened.
                // TODO: handle the error state!
                // TODO: log and error or warning message here.
            }
        } while(available_space <= 0);

        Node node(std::move(data));
        queue[back_index] = std::move(node);
        increment(back_index);

        // Notify the Consumer to "Unblock" if it was blocked waiting on an empty queue.
        xTaskNotifyGiveIndexed(consumerTask, consumerIndexToNotify);
    }


    /*
    This function MUST be called from the Task specified by this->consumerTask.
    Blocks infinitely when the queue is empty.
    Argument must conform to std::move.
    */
    void consumer_pop_front(T&& data) {
        uint32_t queue_count;

        do {
            // NOTE: deadlock will occur here if the producer and the consumer or running in the same task!
            // This function MUST have been called from the Task specified by this->consumerTask.
            //
            // If the queue is empty then this system call will block until 
            //  the producer has pushed something onto the queue.
            //
            // queue_count will be value of the task's notification value before it is decremented.
            // ie. how many items were in the queue when this call unblocked.
            queue_count = ulTaskNotifyTakeIndexed(consumerIndexToNotify, pdFALSE, portMAX_DELAY);
            if (queue_count <= 0) {
                // Either the system call just above timed out, or something bad happened.
                // TODO: handle the error state!
                // TODO: log and error or warning message here.
            }
        } while(queue_count <= 0);

        Node node = std::move(queue[front_index]);
        increment(front_index);
        data = std::move(node.move_data());

        // Notify the Producer to "Unblock" if it was blocked waiting for available space
        //  to push a new item on to the queue.
        xTaskNotifyGiveIndexed(producerTask, producerIndexToNotify);
    }

};



#endif // _LIGHTWEIGHT_1P1C_QUEUE_HPP_
