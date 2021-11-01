#include "Heap.h"
#include "MemoryManager.h"
#include <iostream>
#include <typeinfo>

Heap::Heap(const char* name) : _totalAllocated(0), _peak(0)
{
	_name = name;
}

void Heap::AllocateMemory(int size)
{
	_totalAllocated += size;
	if (_totalAllocated > _peak) _peak = _totalAllocated;
}

void Heap::DeallocateMemory(int size)
{
	_totalAllocated -= size;
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

void Heap::DisplaySizes()
{
	Header* pCurrent = pHead;
	while (pCurrent != NULL)
	{
		
		std::cout << pCurrent->size << " ";
		pCurrent = pCurrent->pNext;
	}

	std::cout << std::endl;
}

void Heap::DisplayDebugInformation()
{
	std::cout << "Name: " << _name << std::endl;
	std::cout << "____________________________________________" << std::endl;
	std::cout << "Current memory: " << _totalAllocated << "\tPeak memory: " << _peak << std::endl;
	std::cout << "____________________________________________" << std::endl;
	std::cout << "TYPE\t\tSIZE" << std::endl;
	std::cout << "____________________________________________" << std::endl;

	Header* pCurrent = pHead;
	size_t hSize = sizeof(Header);
	while (pCurrent != NULL)
	{

		auto& startMem = *(pCurrent + hSize);
		std::cout << typeid(startMem).name() << "\t\t" << pCurrent->size << std::endl;
		
		pCurrent = pCurrent->pNext;
	}

	std::cout << std::endl;

}

