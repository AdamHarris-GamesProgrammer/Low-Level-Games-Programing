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
	//Adds memory to total allocated
	_totalAllocated += size;
	//if total allocated is now greater than the peak, then set the new peak
	if (_totalAllocated > _peak) _peak = _totalAllocated;

	//Set the size of this allocation
	header->size = size;
#ifdef _DEBUG
	//Set the check code in debug mode only
	header->check = deadCode;
#endif // DEBUG

	//Setup the previous and next elements in the doubly linked list
	header->pPrevious = NULL;
	header->pNext = pHead;
	if (pHead != NULL)
		pHead->pPrevious = header;
	pHead = header;
}

void Heap::DeallocateMemory(Header* header, int size)
{
	//Remove the total allocation
	_totalAllocated -= size;

	//Find what elements need to be changed around to allow this element to be removed in the doubly linked list
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
	//Allocate the amount of size required for the memory block
	void* pMem = (void*)malloc(size);
	return pMem;
}

void Heap::DisplayDebugInformation()
{
	SetConsoleColor(WHITE);

	//Outputs the name of this heap
	std::cout << "Name: " << _name << std::endl;
	std::cout << "____________________________________________" << std::endl;

	//if we have a head
	if (pHead != NULL) {
		//Output the total allocated and the peak memory
		//Outputs a nice layout for the debug information
		std::cout << "Current memory: " << _totalAllocated << "\tPeak memory: " << _peak << std::endl;
		std::cout << "____________________________________________" << std::endl;
		std::cout << "ADDRESS\t\t\tTYPE\t\tSIZE" << std::endl;
		std::cout << "____________________________________________" << std::endl;

		//Stores the size of the header
		size_t hSize = sizeof(Header);

		//Gets the current head of the list
		Header* pCurrent = pHead;
		//While the current exists
		while (pCurrent != NULL)
		{
			if(pCurrent == NULL) break;
			auto& startMem = *(pCurrent + hSize);
			std::cout << &startMem << "\t" << typeid(startMem).name() << "\t" << pCurrent->size << std::endl;
			if (pCurrent->pNext == NULL) {
				break;
			}

			//Cycle through to the next element of the list
			pCurrent = pCurrent->pNext;
		}

		std::cout << std::endl;
	}
	else {
		//outputs the total and peak memory allocated if we do not have a head
		std::cout << "Current memory: " << _totalAllocated << "\tPeak memory: " << _peak << std::endl;
	}

	std::cout << std::endl;
}

void Heap::CheckIntegrity()
{
#ifdef _DEBUG
	std::cout << "Checking integrity of " << _name << std::endl;

	bool errorFound = false;
	int totalErrors = 0;

	if (pHead != NULL) {
		//Start at the current head of the list
		Header* pCurrent = pHead;

		//while our current head is not null
		while (pCurrent != NULL) {
			//Check we have the correct check code
			if (pCurrent->check != deadCode) {
				SetConsoleColor(RED);
				std::cout << "[ERROR: Heap::CheckIntegrity]: Header check code does not match" << std::endl;
				errorFound = true;
				totalErrors++;
			}

			//Get the address of the footer
			void* pFooterAddr = ((char*)pCurrent + sizeof(Header) + pCurrent->size);
			Footer* pFooter = (Footer*)pFooterAddr;
			//Check we have the correct check code in
			if (pFooter->check != deadCode) {
				SetConsoleColor(RED);
				std::cout << "[ERROR: Heap::CheckIntegrity]: Footer check code does not match" << std::endl;
				errorFound = true;
				totalErrors++;
			}

			//Set current to the next element in the linked list
			pCurrent = pCurrent->pNext;
		}
	}

	if (errorFound) {
		//Set console text color to red and print a error if an error has been found
		SetConsoleColor(RED);
		std::cout << "[ERROR: Heap::CheckIntegrity]: Error(s) found: " << totalErrors << std::endl;
	}
	else {
		//Set console text color to yellow and print a statement if no error has been found
		SetConsoleColor(YELLOW);
		std::cout << "[MESSAGE: Heap::CheckIntegrity]: No Errors found in " << _name << std::endl;
	}

	//Revert console color back to white
	SetConsoleColor(WHITE);
#endif
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

