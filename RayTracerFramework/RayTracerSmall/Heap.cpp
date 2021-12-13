#include "Heap.h"
#include "MemoryManager.h"
#include <iostream>
#include <typeinfo>

#if defined __linux__

#else 
#include <Windows.h>
#endif

Heap::Heap(std::string name) : _totalAllocated(0), _peak(0)
{
	_name = name;
}

void Heap::AllocateMemory(Header* header, int size)
{
	_totalAllocated += size;
	if (_totalAllocated > _peak) _peak = _totalAllocated;


	header->size = size;
	header->check = deadCode;

	header->pPrevious = NULL;
	header->pNext = pHead;
	if (pHead != NULL)
		pHead->pPrevious = header;
	pHead = header;
}

void Heap::DeallocateMemory(Header* header, int size)
{
	_totalAllocated -= size;

	if (pHead == header) {
		pHead = header->pNext;
	}
	if (header->pNext != NULL) {
		header->pNext->pPrevious = header->pPrevious;
	}
	if (header->pPrevious != NULL) {
		header->pPrevious->pNext = header->pNext;
	}
}

int Heap::GetAmountAllocated()
{
	return _totalAllocated;
}

void* Heap::operator new(size_t size)
{
	void* pMem = (void*)malloc(size);
	return pMem;
}

void Heap::DisplayDebugInformation()
{
	SetConsoleColor(WHITE);

	std::cout << "Name: " << _name << std::endl;
	std::cout << "____________________________________________" << std::endl;

	if (pHead != NULL) {
		std::cout << "Current memory: " << _totalAllocated << "\tPeak memory: " << _peak << std::endl;
		std::cout << "____________________________________________" << std::endl;
		std::cout << "ADDRESS\t\t\tTYPE\t\tSIZE" << std::endl;
		std::cout << "____________________________________________" << std::endl;


		size_t hSize = sizeof(Header);

		Header* pCurrent = pHead;
		while (pCurrent != NULL)
		{
			if(pCurrent == NULL) break;
			auto& startMem = *(pCurrent + hSize);
			std::cout << &startMem << "\t" << typeid(startMem).name() << "\t" << pCurrent->size << std::endl;
			if (pCurrent->pNext == NULL) {
				break;
			}
			pCurrent = pCurrent->pNext;
		}

		std::cout << std::endl;
	}
	else {
		std::cout << "Current memory: " << _totalAllocated << "\tPeak memory: " << _peak << std::endl;
	}

	std::cout << std::endl;
}

void Heap::CheckIntegrity()
{
	std::cout << "Checking integrity of " << _name << std::endl;

	bool errorFound = false;
	int totalErrors = 0;

	#if defined _WIN32
	int red = 12;
	int yellow = 14;
	int brightWhite = 15;
	#endif

	if (pHead != NULL) {
		Header* pCurrent = pHead;

		while (pCurrent != NULL) {
			if (pCurrent->check != deadCode) {
				SetConsoleColor(RED);
				std::cout << "[ERROR: Heap::CheckIntegrity]: Header check code does not match" << std::endl;
				errorFound = true;
				totalErrors++;
			}

			void* pFooterAddr = ((char*)pCurrent + sizeof(Header) + pCurrent->size);
			Footer* pFooter = (Footer*)pFooterAddr;
			if (pFooter->check != deadCode) {
				SetConsoleColor(RED);
				std::cout << "[ERROR: Heap::CheckIntegrity]: Footer check code does not match" << std::endl;
				errorFound = true;
				totalErrors++;
			}

			pCurrent = pCurrent->pNext;
		}
	}

	if (errorFound) {
		SetConsoleColor(RED);
		std::cout << "[ERROR: Heap::CheckIntegrity]: Error(s) found: " << totalErrors << std::endl;
	}
	else {
		SetConsoleColor(YELLOW);
		std::cout << "[MESSAGE: Heap::CheckIntegrity]: No Errors found in " << _name << std::endl;
	}

	SetConsoleColor(WHITE);
}

void Heap::SetConsoleColor(ConsoleColor color)
{
#if defined _WIN32
	HANDLE console;
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	int colorCode = 0;

	switch (color)
	{
	case WHITE:
		colorCode = 15;
		break;
	case RED:
		colorCode = 12;
		break;
	case YELLOW:
		colorCode = 14;
		break;
	}
	SetConsoleTextAttribute(console, colorCode);
#else
	switch (color) {
	case WHITE:
		printf("\033[0m");
		break;
	case RED:
		printf("\033[31m");
		break;
	case YELLOW:
		printf("\033[33m");
		break;
	}

#endif
}

