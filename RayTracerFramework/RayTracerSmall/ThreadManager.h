#pragma once
#include <vector>
#include <functional>

#if defined __windows__
#include <thread>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#endif

class ThreadManager
{
public:
#if defined __windows
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
#else
	pid_t& CreateTask(std::function<void()> task) {
		pid_t newThread = fork();
		if(newThread < 0) {

		}else {
			task();
			_threads.emplace_back(newThread);
		}
		return newThread;
	}

	void WaitForAllThreads() {
		int status;
		for (auto& t : _threads) {
			if(wait(&status) == -1) {

			}
			else if(WIFEXITED(status)) {
				
			}
		}

		_threads.clear();
	}
#endif

private:
#if defined __windows__
	std::vector<std::thread*> _threads;
#else
	std::vector<pid_t> _threads;
#endif

};

