#pragma once
#define NAMELENGTH 16

class Heap
{
public:
	Heap(const char* name);
	void AllocateMemory(int size);
	void DeallocateMemory(int size);
	int GetAmountAllocated();
	const char* GetName() const { return _name; }

	void* operator new (size_t size);
private:
	int _totalAllocated;
	const char* _name;
};

