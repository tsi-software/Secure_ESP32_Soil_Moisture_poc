#pragma once

//#include <concepts>
#include <cmath>
#include <iterator>
#include <limits>
#include <numeric>



//template<std::floating_point T, class Iter>
template<typename T, class Iter>
constexpr
T calculateMean(Iter begin, Iter end) {
    const auto size = std::distance(begin, end);

    if (size <= 0) {
        return std::numeric_limits<T>::quiet_NaN();
    }

    return std::accumulate<Iter, T>(begin, end, 0) / size;
}



/**
see
https://codereview.stackexchange.com/questions/185450/compute-mean-variance-and-standard-deviation-of-csv-number-file
https://codereview.stackexchange.com/questions/265753/mean-and-variance-for-math-library
*/
template<typename T>
struct VarianceResult {
    VarianceResult(T variance, T mean) : variance(variance), mean(mean) { }
    VarianceResult(const VarianceResult<T>& other) : variance(other.variance), mean(other.mean) { }
    T variance, mean;

    T standardDeviation() {
        return std::sqrt(variance);
    }
};

template<typename T, class Iter>
constexpr
VarianceResult<T> calculateVariance(Iter begin, Iter end) {
    const auto size = std::distance(begin, end);

    if (size <= 1) {
        return VarianceResult<T>( std::numeric_limits<T>::quiet_NaN(),
                                  std::numeric_limits<T>::quiet_NaN() );
    }

    const T mean = std::accumulate<Iter, T>(begin, end, 0) / size;

    auto const add_square = [mean](T sum, T i) {
        auto d = i - mean;
        return sum + d*d;
    };
    T total = std::accumulate<Iter, T>(begin, end, 0, add_square);

    return VarianceResult<T>( total/(size-1), mean );
}
