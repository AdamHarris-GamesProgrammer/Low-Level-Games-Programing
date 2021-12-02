#include "ThreadManager.h"

#if defined _WIN32
std::vector<std::thread*> ThreadManager::_threads;

void ThreadManager::CreateTask(std::function<void()> task)
{
	std::thread* newThread = new std::thread(task);
	_threads.emplace_back(newThread);
	//return newThread;
}
void ThreadManager::WaitForAllThreads()
{
	for (auto& t : _threads) {
		t->join();
		delete t;
		t = nullptr;
	}

	_threads.clear();
}
#else
std::vector<pid_t> ThreadManager::_threads;

void ThreadManager::CreateTask(std::function<void()> task)
{
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
}
void ThreadManager::WaitForAllThreads()
{
	int status;
	for (auto& t : _threads) {
		if (wait(&status) == -1) {

		}
		else if (WIFEXITED(status)) {

		}
	}

	_threads.clear();
}
#endif