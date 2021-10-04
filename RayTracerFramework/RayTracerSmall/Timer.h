#pragma once
#include <chrono>

using namespace std::chrono;

class Timer {
public:
	Timer() : last(steady_clock::now()) {}

	float Mark() {
		const auto old = last;
		last = steady_clock::now();
		const duration<float> frameTime = last - old;
		return frameTime.count();
	}

	float Peek() {
		return duration<float>(steady_clock::now() - last).count();
	}

private:
	steady_clock::time_point last;
};

