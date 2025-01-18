#pragma once

#include <chrono>

class FPSCounter
{
public:
    FPSCounter()
    { 
        avgTimePoint = curTimePoint = std::chrono::high_resolution_clock::now();
    }

    void onPresent()
    {
        avgFrameCounter++;

        auto now = std::chrono::high_resolution_clock::now();

        auto avgDuration = std::chrono::duration<double>(now - avgTimePoint).count();
        auto curDuration = std::chrono::duration<double>(now - curTimePoint).count();

        if (avgDuration >= 1.0)
        {
            avgFps          = static_cast<float>(avgFrameCounter) / avgDuration;
            avgFrameCounter = 0;
            avgTimePoint    = now;
        }

        curTimePoint = now;
        curFps       = 1.0 / curDuration;
    }

    float avgfps() const { return avgFps; }

    float curfps() const { return curFps; }

private:
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    float avgFps = 0.01;
    float curFps = 0.01;

    uint32_t avgFrameCounter = 0;

    TimePoint avgTimePoint;
    TimePoint curTimePoint;
};
