#include "HeapManager.h"

Heap& HeapManager::GetDefaultHeap()
{
	return defaultHeap;
}

Heap* HeapManager::CreateHeap(std::string name)
{
	Heap* heap = new Heap(name);
	heapMap.insert(std::make_pair(name, heap));
	return heap;
}

Heap* HeapManager::GetHeap(std::string name)
{
	return heapMap[name];
}

void HeapManager::CleanHeaps()
{
	heapMap.clear();

	//No need to delete defaultHeap or heapMap as they are created on the stack and therefore
	//deleted on program shutdown
}

void HeapManager::DebugAll()
{
	std::unordered_map<std::string, Heap*>::const_iterator it;
	for (it = heapMap.begin(); it != heapMap.end(); it++) {
		it->second->DisplayDebugInformation();
		it->second->CheckIntegrity();
	}

	defaultHeap.DisplayDebugInformation();
	defaultHeap.CheckIntegrity();
}

Heap HeapManager::defaultHeap = Heap("DefaultHeap");
std::unordered_map<std::string, Heap*> HeapManager::heapMap;

