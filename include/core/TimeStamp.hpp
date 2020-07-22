/*
 * File: TimeStamp.hpp
 * -------------------
 * @author: Fu Wei
 *
 */

#ifndef HYDROGENWEB_TIMESTAMP_HPP
#define HYDROGENWEB_TIMESTAMP_HPP

#include <chrono>
#include <string>

class TimeStamp
{
    typedef std::chrono::system_clock::time_point TimePoint;

public:
    TimeStamp() = default;

    explicit TimeStamp(TimePoint t) :
        _timePoint(t)
    {
    }

    auto toMicroSec()
    {
        using namespace std::chrono;
        return duration_cast<microseconds>(_timePoint.time_since_epoch()).count();
    }

    auto toMilliSec()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(_timePoint.time_since_epoch()).count();
    }

    auto toSec()
    {
        using namespace std::chrono;
        return duration_cast<seconds>(_timePoint.time_since_epoch()).count();
    }

    std::string toString();

    static TimeStamp now()
    {
        using namespace std::chrono;
        return TimeStamp(system_clock::now());
    }

    friend bool operator<(const TimeStamp& lhs, const TimeStamp& rhs);
    friend bool operator==(const TimeStamp& lhs, const TimeStamp& rhs);
    friend bool operator<=(const TimeStamp& lhs, const TimeStamp& rhs);

    friend TimeStamp operator+(const TimeStamp& lhs, double sec);

    static const int microSecsPerSecond = 1000000;

private:
    TimePoint _timePoint;
};

#endif //HYDROGENWEB_TIMESTAMP_HPP
