#pragma once
#include <thread>

class Test {
	virtual bool Initialize() = 0;
	virtual void Run() = 0;
	virtual void Shutdown() = 0;
};