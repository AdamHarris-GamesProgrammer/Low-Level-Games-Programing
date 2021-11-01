#include "HeapFactory.h"

Heap* HeapFactory::GetDefaultHeap()
{
	if (defaultHeap == nullptr) defaultHeap = new Heap("DefaultHeap");
	return defaultHeap;
}

Heap* HeapFactory::CreateHeap(std::string name)
{
	Heap* heap = new Heap(name.c_str());
	heapMap.insert(std::make_pair(name, heap));
	return heap;
}

Heap* HeapFactory::GetHeap(std::string name)
{
	return heapMap[name];
}

Heap* HeapFactory::defaultHeap = nullptr;
std::unordered_map<std::string, Heap*> HeapFactory::heapMap;

