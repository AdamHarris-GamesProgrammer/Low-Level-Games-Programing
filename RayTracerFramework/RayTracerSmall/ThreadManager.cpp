#include "ThreadManager.h"

#if defined _WIN32
std::vector<std::thread*> ThreadManager::_threads;
#else 
std::vector<pid_t> ThreadManager::_threads;
#endif

void ThreadManager::CreateTask(std::function<void()> task)
{
#if defined _WIN32
	std::thread* newThread = new std::thread(task);
	_threads.emplace_back(newThread);
#else
	pid_t newThread = vfork();
	if (newThread < 0) {
		printf("Error: Couldn't create fork");
}
	else if (newThread == 0) {
		_threads.emplace_back(newThread);
		task();
		_exit(0);
	}
	else {

	}
#endif
}
void ThreadManager::WaitForAllThreads()
{
#ifdef _WIN32
	for (auto& t : _threads) {
		t->join();
		delete t;
		t = nullptr;
	}

	_threads.clear();
#else
	int status;
	for (auto& t : _threads) {
		if (wait(&status) == -1) {

		}
		else if (WIFEXITED(status)) {

		}
	}

	_threads.clear();
#endif
}
