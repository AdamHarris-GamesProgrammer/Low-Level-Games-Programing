#pragma once
#include <iostream>
#define NAMELENGTH 16

struct Header;

class Heap
{
public:
	Heap(std::string name);
	void AllocateMemory(Header* header, int size);
	void DeallocateMemory(Header* header, int size);
	int GetAmountAllocated();
	std::string GetName() const { return _name; }

	void* operator new (size_t size);

	Header* pHead = NULL;

	void DisplayDebugInformation();
	void CheckIntegrity();

private:
	int _totalAllocated;
	int _peak;
	std::string _name;
};

