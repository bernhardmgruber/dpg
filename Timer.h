#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

//#include <windows.h>

class Timer
{
public:
    Timer();
    void Tick();
    double GetTime();

    float tps;      // Ticks per second
    double interval; // The time passed since the last call of Tick()

private:
    //LARGE_INTEGER HPF;
};

#endif // TIMER_H_INCLUDED
