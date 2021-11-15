#pragma once
#include <vector>
#include <thread>
#include <functional>

class ThreadManager
{
public:
	std::thread* CreateTask(std::function<void()> task) {
		std::thread* newThread = new std::thread(task);
		_threads.emplace_back(newThread);
		return newThread;
	}

	void WaitForAllThreads() {
		for (auto& t : _threads) {
			t->join();
			delete t;
			t = nullptr;
		}

		_threads.clear();
	}

private:
	std::vector<std::thread*> _threads;
};

