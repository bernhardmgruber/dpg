#include "timer.h"
#include <ctime>

#define TPS_UPDATE_INTERVAL 0.2f

Timer::Timer() {
	if (!QueryPerformanceFrequency(&frequency))
		frequency.QuadPart = 0; //set 0 if not suppored

	tickCounter = 0;
	lastTimeFPSUpdate = 0.0;

	tick();
}

void Timer::tick() {
	static double lastTime = 0.0;

	double currentTime = getTime();
	interval = currentTime - lastTime;
	lastTime = currentTime;

	tickCounter++;

	if (currentTime - lastTimeFPSUpdate > TPS_UPDATE_INTERVAL) {
		tps = static_cast<float>(static_cast<double>(tickCounter) / (currentTime - lastTimeFPSUpdate));

		lastTimeFPSUpdate = currentTime;
		tickCounter = 0;
	}
}

double Timer::getTime() {
	if (frequency.QuadPart) //g_HPF.QuadPart is 0, if not supported
	{
		LARGE_INTEGER PerformanceCounter;
		QueryPerformanceCounter(&PerformanceCounter);
		return (double)PerformanceCounter.QuadPart / (double)frequency.QuadPart;
	} else
		return static_cast<double>(clock()) / static_cast<double> CLOCKS_PER_SEC;
}
