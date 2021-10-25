#include "MemoryManager.h"
#include<iostream>

void* operator new(size_t size) {
	char* pMem = (char*)malloc(size);
	void* pMemBlock = pMem;
	return pMemBlock;
}
void operator delete(void* pMem) {
	free(pMem);
}
