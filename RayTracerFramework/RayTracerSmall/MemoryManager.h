#pragma once
#include "Heap.h"
#include "HeapManager.h"

#define deadCode 0xDEADC0DE

struct Header {
	int size;
	int check = deadCode;
	Heap* pHeap;
	Header* pPrevious = NULL;
	Header* pNext = NULL;
};

struct Footer {
	int reserved;
	int check = deadCode;
};



void* operator new(size_t size, Heap* heap);
void* operator new(size_t size);
void operator delete(void* pMem);

