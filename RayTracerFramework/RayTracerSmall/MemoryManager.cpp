#include "MemoryManager.h"

#include<iostream>

void* operator new(size_t size, Heap* heap) {
	size_t requestedBytes = size + sizeof(Header) + sizeof(Footer);
	char* pMem = (char*)malloc(requestedBytes);

	Header* pHeader = (Header*)pMem;
	pHeader->size = size;
	pHeader->check = deadCode;
	pHeader->pHeap = heap;

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

void* operator new(size_t size) {
	return ::operator new(size, HeapFactory::GetDefaultHeap());
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

	free(pHeader);
}
