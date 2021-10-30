#pragma once
#include "Heap.h"
#include <unordered_map>

class HeapFactory
{
public:
	static void Init();
	static Heap* GetDefaultHeap();

private:
	static Heap* defaultHeap;
};

