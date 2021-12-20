#pragma once
#include "Heap.h"
#include "HeapManager.h"

#define deadCode 0xDEADC0DE

struct Header {
	int size;
#if DEBUG
	int check = deadCode;
#endif

	Heap* pHeap;
	Header* pPrevious = NULL;
	Header* pNext = NULL;
};

struct Footer {
	int reserved;

#ifdef DEBUG
	int check = deadCode;
#endif
};



void* operator new(size_t size, Heap* heap);
void* operator new[](size_t size, Heap* heap);
void* operator new(size_t size);
void operator delete(void* pMem);

