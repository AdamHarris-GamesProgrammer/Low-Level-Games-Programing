#include "Heap.h"
#include "MemoryManager.h"
#include <iostream>
#include <typeinfo>

Heap::Heap(std::string name) : _totalAllocated(0), _peak(0)
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

void Heap::DisplayDebugInformation()
{
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
		std::cout << "Total Allocated: " << _totalAllocated << std::endl;
	}



}

