#pragma once
#include "Heap.h"
#include "HeapFactory.h"

#define deadCode 0xDEADC0DE

struct Header {
	int size;
	int check = deadCode;
	int allocNumSize;
	Heap* pHeap;
	Header* previous;
	Header* next;
};

struct Footer {
	int reserved;
	int check = deadCode;
};



void* operator new(size_t size, Heap* heap);
void* operator new(size_t size);
void operator delete(void* pMem);

