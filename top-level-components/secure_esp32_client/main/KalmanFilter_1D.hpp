#pragma once

#include <ostream>
#include <sstream>


/*
--------------
Values:
--------------
Original Estimate

Previous Estimate

Measured Value (Data Input)

Error in Estimate

Error in Data (Measurement)


--------------
Calculations:
--------------
1. Calculate the Kalman Gain.

2. Calculate Current Estimate.

3. Calculate new Error in Estimate.

*/
class KalmanFilter_1D {
public:
    using ValueType = float; // ?? double ??
    KalmanFilter_1D() { }
    virtual ~KalmanFilter_1D() { }

    void setInitialValues(
            ValueType initialEstimate,
            ValueType initialEstimateError,
            ValueType initialMeasurementError
    );

    // inline ValueType getCurrentEstimate() const {
    //     return current.estimate;
    // }

    void processNewMeasurement(ValueType newMeasurement);

    std::ostringstream formatCsvHeader() const { return current.formatCsvHeader(); }
    std::ostringstream formatCsv() const       { return current.formatCsv(); }
    std::ostringstream formatDebug() const;

    static void test1();


private:
    class KalmanValues {
    public:
        ValueType estimate,
                  estimateError,
                  measurement, //TODO: 'measurement' is transitory and only used in one calculation.
                  measurementError,
                  kalmanGain;

        KalmanValues() :
            estimate(0), estimateError(0),
            measurement(0), measurementError(0),
            kalmanGain(0)
        { }

        KalmanValues(ValueType estimate, ValueType estimateError,
                     ValueType measurement, ValueType measurementError,
                     ValueType kalmanGain
        ) :
            estimate(estimate), estimateError(estimateError),
            measurement(measurement), measurementError(measurementError),
            kalmanGain(kalmanGain)
        { }

        KalmanValues(const KalmanValues& other) :
            estimate(other.estimate), estimateError(other.estimateError),
            measurement(other.measurement), measurementError(other.measurementError),
            kalmanGain(other.kalmanGain)
        { }

        // see https://en.cppreference.com/w/cpp/language/operators
        KalmanValues& operator=(const KalmanValues& other) {
            if (this == &other) {
                return *this;
            }
            estimate = other.estimate;
            estimateError = other.estimateError;
            measurement = other.measurement;
            measurementError = other.measurementError;
            kalmanGain = other.kalmanGain;
            return *this;
        }

        std::ostringstream formatCsvHeader() const;
        std::ostringstream formatCsv() const;
        std::ostringstream formatDebug() const;


        ValueType calculateKalmanGain() const {
            return estimateError / (estimateError + measurementError);
        }


        // Note: the new 'kalmanGain' MUST be passed in as a function argument because
        //       this function is called on the previous kalman values.
        ValueType newEstimate(ValueType kalmanGain, ValueType newMeasurement) const {
            return estimate + kalmanGain * (newMeasurement - estimate);
        }


        // Note: the new 'kalmanGain' MUST be passed in as a function argument because
        //       this function is called on the previous kalman values.
        ValueType newEstimateError(ValueType kalmanGain) const {
            return ((ValueType)1 - kalmanGain) * estimateError;
        }


        void processNewMeasurement(ValueType newMeasurement, const KalmanValues& previous) {
            kalmanGain = previous.calculateKalmanGain();
            estimate = previous.newEstimate(kalmanGain, newMeasurement);

            estimateError = previous.newEstimateError(kalmanGain);

            measurement = newMeasurement;
            measurementError = previous.measurementError;
        }
    };


    KalmanValues current, previous;
};



std::ostream& operator<< (std::ostream& stream, const KalmanFilter_1D& obj);
