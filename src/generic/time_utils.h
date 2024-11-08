#pragma once

#include <chrono>
#include <ctime>
#include <iostream>

namespace polyp {

inline double getTimeSpand()
{
    using namespace std::chrono;
    auto time = system_clock::now().time_since_epoch();
    return duration_cast<duration<double, std::deci>>(time).count();
}

}
