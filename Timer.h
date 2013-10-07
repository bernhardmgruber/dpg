#pragma once

//#include <windows.h>

class Timer
{
public:
    Timer();
    void tick();
    double getTime();

    float tps;      // Ticks per second
    double interval; // The time passed since the last call of Tick()

private:
    //LARGE_INTEGER HPF;
};
