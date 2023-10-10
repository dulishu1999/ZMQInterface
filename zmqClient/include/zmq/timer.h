// Copyright (c) fp Corporation. All rights reserved.
// Licensed under the MIT License.



#ifndef TIMER_H
#define TIMER_H

// System headers
#include <iostream>
#include <chrono>
#include <ctime>
#include <cmath>

// Library headers

// Project headers
namespace fp
{

class Timer
{
public:
    Timer()=default;

    ~Timer()=default;

    void Start();
    
    void Stop();
    
    double ElapsedMilliseconds();
    
    double ElapsedSeconds();
    
private:
    std::chrono::time_point<std::chrono::system_clock> m_startTime;
    std::chrono::time_point<std::chrono::system_clock> m_endTime;
    bool                                               m_bRunning = false;
};

}

#endif