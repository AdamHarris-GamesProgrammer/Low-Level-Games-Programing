#pragma once
#include <iostream>
#define NAMELENGTH 16

struct Header;

class Heap
{
public:
	Heap(const char* name);
	void AllocateMemory(int size);
	void DeallocateMemory(int size);
	int GetAmountAllocated();
	const char* GetName() const { return _name; }

	void* operator new (size_t size);

	Header* pHead = NULL;

	void DisplaySizes();
	void DisplayDebugInformation();

private:
	int _totalAllocated;
	int _peak;
	const char* _name;
};

