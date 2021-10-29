#include "HeapFactory.h"

void HeapFactory::Init()
{
	CreateHeap("DefaultHeap");
	defaultHeap = GetHeap("DefaultHeap");
}

Heap* HeapFactory::GetDefaultHeap()
{
	return defaultHeap;
}

void HeapFactory::CreateHeap(std::string name)
{
	Heap* heap = new Heap(name.c_str());
	_heaps.insert(std::make_pair(name, heap));
}

Heap* HeapFactory::GetHeap(std::string name)
{
	return _heaps[name];
}

Heap* HeapFactory::defaultHeap;

std::unordered_map<std::string, Heap*> HeapFactory::_heaps;
