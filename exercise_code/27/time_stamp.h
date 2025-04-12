#pragma once

#include <iostream>
#include <string>

class TimeStamp {
private:
    time_t time_;

public:
    TimeStamp();
    TimeStamp(int64_t time);
    static TimeStamp Now();
    time_t ToInt() const;
    std::string ToString() const;
};