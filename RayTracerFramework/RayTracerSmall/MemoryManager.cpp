#include "MemoryManager.h"

#include<iostream>

void* operator new(size_t size) {
	return ::operator new(size, &HeapManager::GetDefaultHeap());
}

void* operator new(size_t size, Heap* heap) {
	size_t requestedBytes = size + sizeof(Header) + sizeof(Footer);
	char* pMem = (char*)malloc(requestedBytes);

	Header* pHeader = (Header*)pMem;
	pHeader->size = size;
	pHeader->check = deadCode;
	pHeader->pHeap = heap;

	pHeader->pPrevious = NULL;
	pHeader->pNext = heap->pHead;
	if (heap->pHead != NULL)
		heap->pHead->pPrevious = pHeader;
	heap->pHead = pHeader;

	pHeader->pHeap->AllocateMemory(size);

	//Get the location of the footer start position
	//pMem (start of mem block) + sizeof(header) (offset) + size of the mem block. Will give the memory position of the footer
	void* pFooterAddr = pMem + sizeof(Header) + size;
	Footer* pFooter = (Footer*)pFooterAddr;
	pFooter->check = deadCode;

	//TODO: Setup reserving memory 

	void* pStartMemBlock = pMem + sizeof(Header);
	return pStartMemBlock;
}



void operator delete(void* pMem) {
	Header* pHeader = (Header*)((char*)pMem - sizeof(Header));
	if (pHeader->check != deadCode) {
		std::cout << "[ERROR: MemoryManager::delete]: Header check code does not match" << std::endl;
	}

	Footer* pFooter = (Footer*)((char*)pMem + pHeader->size);
	if (pFooter->check != deadCode) {
		std::cout << "[ERROR: MemoryManager::delete]: Footer check code does not match" << std::endl;
	}

	pHeader->pHeap->DeallocateMemory(pHeader->size);

	
	if (pHeader->pNext != NULL) {
		if(pHeader->pPrevious != NULL) pHeader->pPrevious->pNext = pHeader->pNext;
		pHeader->pNext->pPrevious = pHeader->pPrevious;
	}
	else if(pHeader->pPrevious != NULL) {
		pHeader->pPrevious->pNext = NULL;
	}

	if (pHeader->pHeap->pHead == pHeader) {
		pHeader->pHeap->pHead = pHeader->pNext;
	}

	free(pHeader);
}
