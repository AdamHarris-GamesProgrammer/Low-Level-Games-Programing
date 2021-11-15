#pragma once
#include <iostream>

class MemoryPool
{
public:
	MemoryPool(Heap* heap, unsigned long noOfChunks, unsigned long sizeOfChunks /* Recommended 2**n */) :
		_pMemBlock(nullptr),
		_pAllocatedMemBlock(nullptr),
		_pFreeMemBlock(nullptr),
		_poolSize(noOfChunks * (sizeOfChunks + sizeof(Node))),
		_blockSize(sizeOfChunks)
	{
		//Calculate required bytes 
		size_t requestedBytes = _poolSize + sizeof(Header) + sizeof(Footer);
		//Allocate memory
		char* pMem = (char*)malloc(requestedBytes);

		//Header is at the start of the memory block
		Header* pHeader = (Header*)pMem;
		pHeader->pHeap = heap;

		//Allocates the memory to the heap mem counter and inserts the pool into the linked list
		heap->AllocateMemory(pHeader, _poolSize);

		//Setting the start of the memory block
		_pMemBlock = pMem + sizeof(Header);
		if (_pMemBlock)
		{
			for (unsigned i = 0; i < noOfChunks; i++)
			{
				//Linked list for the blocks of memory
				Node* pCurrentNode = (Node*)((char*)_pMemBlock + i * (sizeOfChunks + sizeof(Node)));

				//Sets the default values 
				pCurrentNode->pPrev = nullptr;
				pCurrentNode->pNext = _pFreeMemBlock;  

				//Check that the free memory block has been initialized
				if (_pFreeMemBlock != nullptr)
					//Set the previous node to the current node
					_pFreeMemBlock->pPrev = pCurrentNode;

				//Sets the free mem block to the current node, allowing us to setup the next chunk 
				_pFreeMemBlock = pCurrentNode;
			}
		}

		//Sets up the footer pointer
		void* pFooterAddr = ((char*)pMem + sizeof(Header) + _poolSize);
		Footer* pFooter = (Footer*)pFooterAddr;
		pFooter->check = deadCode;
	}

	~MemoryPool()
	{
		//Calls the global delete override
		delete _pMemBlock;
	}

	// Allocate memory unit If memory pool can`t provide proper memory unit,
	// It will call system function.
	void* Alloc(unsigned long requestedBytes)
	{
		//std::cout << "Allocating " << requestedBytes << std::endl;

		Node* pCurUnit = _pFreeMemBlock;
		_pFreeMemBlock = pCurUnit->pNext;  //Get a unit from free linked list.
		if (_pFreeMemBlock != nullptr)
		{
			_pFreeMemBlock->pPrev = nullptr;
		}

		pCurUnit->pNext = _pAllocatedMemBlock;

		if (_pAllocatedMemBlock != nullptr)
		{
			_pAllocatedMemBlock->pPrev = pCurUnit;
		}
		_pAllocatedMemBlock = pCurUnit;

		return (void*)((char*)pCurUnit + sizeof(Node));
	}

	//  To free a memory unit. If the pointer of parameter point to a memory unit,
	//  then insert it to "Free linked list". Otherwise, call system function "free".
	void Free(void* p)
	{
		//check that the memory we wish to delete is in the pool
		//((char*)_pMemBlock + _poolSize. Pointer arithmetic to get the memory address at the end of the pool
		if (p > _pMemBlock && p < ((char*)_pMemBlock + _poolSize))
		{
			//std::cout << "Setting memory free starting at: " << p << std::endl;

			Node* pCurUnit = (Node*)((char*)p - sizeof(Node));

			Node* pPrev = pCurUnit->pPrev;
			Node* pNext = pCurUnit->pNext;
			if (pPrev == nullptr) {
				_pAllocatedMemBlock = pCurUnit->pNext;
			}
			else {
				pPrev->pNext = pNext;
			}
			if (pNext != nullptr)
			{
				pNext->pPrev = pPrev;
			}

			pCurUnit->pNext = _pFreeMemBlock;
			if (_pFreeMemBlock != nullptr)
			{
				_pFreeMemBlock->pPrev = pCurUnit;
			}

			_pFreeMemBlock = pCurUnit;
		}
	}
private:
	//This is a doubly linked list
	struct Node
	{
		Node* pPrev, * pNext;
	};

	// Manage all blocks of memory with linked lists
	Node* _pAllocatedMemBlock; //Linked list of allocated memory blocks
	Node* _pFreeMemBlock;      //linked list of free memory blocks

	void* _pMemBlock;  //The start address of the memory pool.

	unsigned long _blockSize;//Size of one memory chunk
	unsigned long _poolSize; //Memory pool size. Memory pool is make of memory unit.
};
