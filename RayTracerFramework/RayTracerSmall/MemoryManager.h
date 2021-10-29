#pragma once
#include "Heap.h"

#define deadCode 0xDEADC0DE

struct Header {
	int size;
	int check = deadCode;
	Heap* pHeap;
};

struct Footer {
	int reserved;
	int check = deadCode;
};



void* operator new(size_t size, Heap* heap);
void operator delete(void* pMem);

