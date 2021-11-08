#pragma once
#include "Heap.h"
#include <unordered_map>

class HeapManager
{
public:
	static Heap& GetDefaultHeap();
	static Heap* CreateHeap(std::string name);
	static Heap* GetHeap(std::string name);

private:
	static Heap defaultHeap;
	static std::unordered_map<std::string, Heap*> heapMap;
};

