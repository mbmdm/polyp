#pragma once

#include <chrono>

class FPSCounter
{
public:
    FPSCounter()
    { 
        avg.prevFrame    = std::chrono::high_resolution_clock::now();
        avg.fps          = kInitialFPS;
        avg.frameCounter = 0;

        cur.fps          = kInitialFPS;
        cur.pevFrame     = avg.prevFrame;
    }

    void onPresent()
    {
        avg.frameCounter++;

        auto now = std::chrono::high_resolution_clock::now();

        auto avgDuration = std::chrono::duration<double>(now - avg.prevFrame).count();
        auto curDuration = std::chrono::duration<double>(now - cur.pevFrame).count();

        if (avgDuration >= 1.0)
        {
            avg.fps          = static_cast<float>(avg.frameCounter) / avgDuration;
            avg.frameCounter = 0;
            avg.prevFrame    = now;
        }

        cur.pevFrame = now;
        cur.fps = 1.0 / curDuration;
    }

    float avgfps() const { return avg.fps; }

    float fps() const { return cur.fps; }

private:
    using TymePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    const double kInitialFPS = 25.0;

    struct {
        float     fps;
        TymePoint prevFrame;
        uint32_t  frameCounter;
    } avg;

    struct {
        float     fps;
        TymePoint pevFrame;
    } cur;
};
