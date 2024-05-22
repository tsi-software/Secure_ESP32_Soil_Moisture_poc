// fast_array_average.hpp

#ifndef _FAST_ARRAY_AVERAGE_HPP_
#define _FAST_ARRAY_AVERAGE_HPP_

#include <array>
#include <ostream>



template<class T, class S, std::size_t array_size_>
class FastArrayAverage {
public:
    using AccumulatorType = S;
    using ValueType = T;
    using ValueArrayType = std::array<T, array_size_>;

    /*
    # of bits   value (2^n)
        0          1
        1          2
        2          4
        3          8
        4         16
        5         32
        6         64
        7        128
        8        256
        9        512
       10       1024
    **/

    static const std::size_t array_size = array_size_;
    const unsigned number_of_bits;
    const unsigned sample_size;


    FastArrayAverage(unsigned number_of_bits) :
        number_of_bits(number_of_bits),
        sample_size(1 << number_of_bits) //TODO: EXPLAIN!
    {
        //TODO: number_of_bits > 0 and number_of_bits < 2^(sizeof(S))
        reset();
    }


    void reset() {
        // for (std::size_t index = 0; index < array_size; ++index) {
        //     accumulator[index] = 0;
        // }
        for (auto &value : accumulator) {
            value = 0;
        }
        array_sample_count = 0;
    }


    void add_values(const ValueArrayType& values) {
        for (std::size_t index = 0; index < array_size; ++index) {
            accumulator[index] += values[index];
        }
        ++array_sample_count;
    }


    // 'array_values' must be an array of size 'array_size'!
    // void add_array_values(ValueType *array_values) {
    //     if (array_sample_count >= sample_size) {
    //         //TODO: throw an error.
    //         return;
    //     }
    //
    //     for (std::size_t index = 0; index < array_size; ++index) {
    //         accumulator[index] += array_values[index];
    //     }
    //     ++array_sample_count;
    // }


    bool is_average_ready() {
        return array_sample_count == sample_size;
    }


    // 'result' must be an array of size 'array_size'!
    void get_average_values(ValueArrayType& result) {
        if (!is_average_ready()) {
            //TODO: log a warning message that the average values cannot yet be computed.
            for (auto &rslt : result) {
                rslt = 0;
            }
            return;
        }

        for (std::size_t index = 0; index < array_size; ++index) {

            //TODO:...
            // if (std::is_unsigned<AccumulatorType>::value) {
            //     //
            // }

            if (accumulator[index] < 0) {
                result[index] = -(-accumulator[index] >> number_of_bits);

            } else {
                result[index] = accumulator[index] >> number_of_bits;
            }
        }

        reset();
    }


#ifdef DEBUG
    void debug_stream(std::ostream &stream) {
        stream << "array_sample_count:" << array_sample_count
               << ", number_of_bits: " << number_of_bits
               << ", sample_size: " << sample_size
               << std::endl;
        stream << "Accumulator:" << std::endl;
        for (std::size_t index = 0; index < array_size; ++index) {
            if (index != 0) {
                stream << ", ";
            }
            stream << accumulator[index];
        }
        stream << std::endl;

        if (is_average_ready()) {
            stream << "AVERAGE:" << std::endl;
            for (std::size_t index = 0; index < array_size; ++index) {
                if (index != 0) {
                    stream << ", ";
                }
                if (accumulator[index] < 0) {
                    stream << -(-accumulator[index] >> number_of_bits);
                } else {
                    stream << (accumulator[index] >> number_of_bits);
                }
            }
            stream << std::endl;
        } else {
            stream << "* average not yet ready." << std::endl;
        }
    }
#endif


private:
    unsigned array_sample_count = 0;
    //AccumulatorType accumulator[array_size];
    std::array<AccumulatorType, array_size> accumulator;
};



#endif // _FAST_ARRAY_AVERAGE_HPP_
