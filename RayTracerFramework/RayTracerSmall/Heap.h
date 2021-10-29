#pragma once
class Heap
{
public:
	Heap() : _totalAllocated(0) {}
	void AllocateMemory(int size);
	void DeallocateMemory(int size);
	int GetAmountAllocated();
private:
	int _totalAllocated;
};

