#include "KalmanFilter_1D.hpp"

#include <iostream>
#include <sstream>



//------------------------------------------------------------------------------
void KalmanFilter_1D::setInitialValues(
        ValueType initialEstimate,
        ValueType initialEstimateError,
        ValueType initialMeasurementError
) {
    current = KalmanValues(initialEstimate, initialEstimateError, 0, initialMeasurementError, 0);
}


void KalmanFilter_1D::processNewMeasurement(ValueType newMeasurement)
{
    previous = current;
    current.processNewMeasurement(newMeasurement, previous);
}


std::ostringstream KalmanFilter_1D::formatDebug() const
{
    std::ostringstream stream;
    stream << "Current: " << current.formatDebug().str() << std::endl
           << "Previous: " << previous.formatDebug().str() << std::endl
           ;
    return stream;
}



//------------------------------------------------------------------------------
std::ostringstream KalmanFilter_1D::KalmanValues::formatCsvHeader() const
{
    std::ostringstream stream;
    stream << "estimate,estimate_error,measurement,measurement_error,kalman_gain";
    return stream;
}


std::ostringstream KalmanFilter_1D::KalmanValues::formatCsv() const
{
    std::ostringstream stream;
    stream << estimate
           << "," << estimateError
           << "," << measurement
           << "," << measurementError
           << "," << kalmanGain
           ;
    return stream;
}


std::ostringstream KalmanFilter_1D::KalmanValues::formatDebug() const
{
    std::ostringstream stream;
    stream << "estimate=" << estimate
           << ", estimate error=" << estimateError
           << ", measurement=" << measurement
           << ", measurement error=" << measurementError
           << ", kalman gain=" << kalmanGain
           ;
    return stream;
}



//------------------------------------------------------------------------------
/**
see:
https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading
*/
std::ostream& operator<<(std::ostream& stream, const KalmanFilter_1D& obj)
{
    stream << obj.formatCsv().str();
    return stream;
}



//------------------------------------------------------------------------------
/**
Expected Test Output:

Initial Values:
Current: estimate=68, estimate error=2, measurement=0, measurement error=4
Previous: estimate=0, estimate error=0, measurement=0, measurement error=0

Measurement:75
processNewMeasurement(...): Kalman Gain=0.333333
Current: estimate=70.3333, estimate error=1.33333, measurement=75, measurement error=4
Previous: estimate=68, estimate error=2, measurement=0, measurement error=4

Measurement:71
processNewMeasurement(...): Kalman Gain=0.25
Current: estimate=70.5, estimate error=1, measurement=71, measurement error=4
Previous: estimate=70.3333, estimate error=1.33333, measurement=75, measurement error=4

Measurement:70
processNewMeasurement(...): Kalman Gain=0.2
Current: estimate=70.4, estimate error=0.8, measurement=70, measurement error=4
Previous: estimate=70.5, estimate error=1, measurement=71, measurement error=4

Measurement:74
processNewMeasurement(...): Kalman Gain=0.166667
Current: estimate=71, estimate error=0.666667, measurement=74, measurement error=4
Previous: estimate=70.4, estimate error=0.8, measurement=70, measurement error=4
*/
void KalmanFilter_1D::test1()
{
    KalmanFilter_1D::ValueType measurement;
    KalmanFilter_1D kalmanFilter;
    kalmanFilter.setInitialValues(68, 2, 4);
    std::cout << "Initial Values:" << std::endl
              << kalmanFilter.formatDebug().str() << std::endl;

    measurement = 75;
    std::cout << "Measurement:" << measurement << std::endl;
    kalmanFilter.processNewMeasurement(measurement);
    std::cout << kalmanFilter.formatDebug().str() << std::endl;

    measurement = 71;
    std::cout << "Measurement:" << measurement << std::endl;
    kalmanFilter.processNewMeasurement(measurement);
    std::cout << kalmanFilter.formatDebug().str() << std::endl;

    measurement = 70;
    std::cout << "Measurement:" << measurement << std::endl;
    kalmanFilter.processNewMeasurement(measurement);
    std::cout << kalmanFilter.formatDebug().str() << std::endl;

    measurement = 74;
    std::cout << "Measurement:" << measurement << std::endl;
    kalmanFilter.processNewMeasurement(measurement);
    std::cout << kalmanFilter.formatDebug().str() << std::endl;
}
