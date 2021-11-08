#include "HeapFactory.h"

Heap* HeapManager::GetDefaultHeap()
{
	if (defaultHeap == nullptr) defaultHeap = new Heap("DefaultHeap");
	return defaultHeap;
}

Heap* HeapManager::CreateHeap(std::string name)
{
	Heap* heap = new Heap(name.c_str());
	heapMap.insert(std::make_pair(name, heap));
	return heap;
}

Heap* HeapManager::GetHeap(std::string name)
{
	return heapMap[name];
}

Heap* HeapManager::defaultHeap = nullptr;
std::unordered_map<std::string, Heap*> HeapManager::heapMap;

