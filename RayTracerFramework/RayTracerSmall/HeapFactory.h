#pragma once
#include "Heap.h"
#include <unordered_map>

class HeapFactory
{
public:
	static void Init();
	static Heap* GetDefaultHeap();
	static void CreateHeap(std::string name);
	static Heap* GetHeap(std::string name);

private:
	static Heap* defaultHeap;
	static std::unordered_map<std::string, Heap*> _heaps;
};

