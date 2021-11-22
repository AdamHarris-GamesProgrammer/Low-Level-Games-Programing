#pragma once
#include <vector>
#include <functional>

#if defined _WIN32
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
#if defined _WIN32
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
	void CreateTask(std::function<void()> task) {
		pid_t newThread = vfork();
		if(newThread < 0) {
			printf("Error: Couldn't create fork");
		}else if(newThread == 0){
			_threads.emplace_back(newThread);
			task();
			_exit(0);
		}else{

		}
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
#if defined _WIN32
	std::vector<std::thread*> _threads;
#else
	std::vector<pid_t> _threads;
#endif

};

