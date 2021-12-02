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
	static void CreateTask(std::function<void()> task);

	static void WaitForAllThreads();
#else
	static void CreateTask(std::function<void()> task);

	static void WaitForAllThreads();
#endif

private:
#if defined _WIN32
	static std::vector<std::thread*> _threads;
#else
	static std::vector<pid_t> _threads;
#endif

};

