#include "timer.h"
#include <time.h>

#define TPS_UPDATE_INTERVAL 0.2f

Timer::Timer()
{
    //if (!QueryPerformanceFrequency(&HPF))
    //    HPF.QuadPart = 0; //set 0 if not suppored

    Tick();
}

void Timer::Tick()
{
    static double lastTime = 0.0;

    double currentTime = GetTime();
    interval = currentTime - lastTime;

    lastTime = currentTime;

    static int tickCounter = 0;
    static double lastTimeFPSUpdate = 0.0;

    tickCounter++;

    if (currentTime - lastTimeFPSUpdate > TPS_UPDATE_INTERVAL)
    {
        tps = (float)((double)tickCounter / (currentTime - lastTimeFPSUpdate));

        lastTimeFPSUpdate = currentTime;
        tickCounter = 0;
    }
}

double Timer::GetTime()
{
    /*if (HPF.QuadPart) //g_HPF.QuadPart is 0, if not supported
    {
        LARGE_INTEGER PerformanceCounter;
        QueryPerformanceCounter(&PerformanceCounter);
        return (double)PerformanceCounter.QuadPart / (double)HPF.QuadPart;
    }
    else*/
        return (double)clock() / (double)CLOCKS_PER_SEC;
}
