#pragma once
#include <thread>
#include <chrono>
#include <string>

#define TEST_ENTITY_COMPONENTS 0
#define TEST_WINDOW 0
#define TEST_RENDERER 1

class Test {
	virtual bool Initialize() = 0;
	virtual void Run() = 0;
	virtual void Shutdown() = 0;
};
#if _WIN64
#include <Windows.h>
class Timer {
public:
	using Clock = std::chrono::high_resolution_clock;
	using TimeStamp = std::chrono::steady_clock::time_point;

	void Begin() 
	{
		start = Clock::now();
	}

	void End() 
	{
		auto dt = Clock::now() - start;
		msAvg += ((float)std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() - msAvg) / (float)counter;
		++counter;

		if (std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - seconds).count() >= 1) 
		{
			OutputDebugStringA("Avg. frame (ms):");
			OutputDebugStringA(std::to_string(msAvg).c_str());
			OutputDebugStringA((" " + std::to_string(counter)).c_str());
			OutputDebugStringA(" fps");
			OutputDebugStringA("\n");
			msAvg = 0.f;
			counter = 1;
			seconds = Clock::now();
		}
	}
private:
	float msAvg{ 0.f };
	int counter{ 1 };
	TimeStamp start;
	TimeStamp seconds{ Clock::now() };
};
#endif