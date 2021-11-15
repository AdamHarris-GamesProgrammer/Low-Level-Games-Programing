#include "Heap.h"
#include "MemoryManager.h"
#include <iostream>
#include <typeinfo>
#include <Windows.h>

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

	//Edge case: The header we are trying to deallocate is the head of the heap's linked list
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
	HANDLE console;
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console, 15); //15 = Bright White

	std::cout << "Name: " << _name << std::endl;
	std::cout << "____________________________________________" << std::endl;

	if (pHead != NULL) {
		std::cout << "Current memory: " << _totalAllocated << "\tPeak memory: " << _peak << std::endl;
		std::cout << "____________________________________________" << std::endl;
		std::cout << "ADDRESS\t\t\tTYPE\t\tSIZE" << std::endl;
		std::cout << "____________________________________________" << std::endl;

		Header* pCurrent = pHead;
		size_t hSize = sizeof(Header);
		while (pCurrent != NULL)
		{
			auto& startMem = *(pCurrent + hSize);
			std::cout << &startMem << "\t" << typeid(startMem).name() << "\t" << pCurrent->size << std::endl;

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

	HANDLE console;
	console = GetStdHandle(STD_OUTPUT_HANDLE);
	int red = 12;
	int yellow = 14;
	int brightWhite = 15;

	if (pHead != NULL) {
		Header* pCurrent = pHead;

		while (pCurrent != NULL) {

			
			if (pCurrent->check != deadCode) {
				SetConsoleTextAttribute(console, red);
				std::cout << "[ERROR: Heap::CheckIntegrity]: Header check code does not match" << std::endl;
				errorFound = true;
				totalErrors++;
			}

			void* pFooterAddr = ((char*)pCurrent + sizeof(Header) + pCurrent->size);
			Footer* pFooter = (Footer*)pFooterAddr;
			if (pFooter->check != deadCode) {
				SetConsoleTextAttribute(console, red);
				std::cout << "[ERROR: Heap::CheckIntegrity]: Footer check code does not match" << std::endl;
				errorFound = true;
				totalErrors++;
			}

			pCurrent = pCurrent->pNext;
		}
	}

	if (errorFound) {
		SetConsoleTextAttribute(console, red);
		std::cout << "[ERROR: Heap::CheckIntegrity]: Error(s) found: " << totalErrors << std::endl;
	}
	else {
		SetConsoleTextAttribute(console, yellow);
		std::cout << "[MESSAGE: Heap::CheckIntegrity]: No Errors found in " << _name << std::endl;
	}

	SetConsoleTextAttribute(console, brightWhite); 
}

