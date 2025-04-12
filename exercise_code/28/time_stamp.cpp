#include "time_stamp.h"

#include <time.h>

TimeStamp::TimeStamp() { time_ = time(0); }

TimeStamp::TimeStamp(int64_t time) : time_(time) {}

TimeStamp TimeStamp::Now() { return TimeStamp(); }

time_t TimeStamp::ToInt() const { return time_; }

std::string TimeStamp::ToString() const {
    char buffer[32];
    struct tm* tm_time = localtime(&time_);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_time);
    return std::string(buffer);
}
