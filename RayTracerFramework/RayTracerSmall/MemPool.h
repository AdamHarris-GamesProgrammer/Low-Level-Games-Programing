#pragma once
#include <iostream>

class MemPool
{
public:
	MemPool(unsigned long noOfChunks, unsigned long sizeOfChunks /* Recommended 2**n */) :
		_pMemBlock(nullptr),
		_pAllocatedMemBlock(nullptr),
		_pFreeMemBlock(nullptr),
		_poolSize(noOfChunks* (sizeOfChunks + sizeof(struct Chunk))),
		_chunkSize(sizeOfChunks)
	{
		_pMemBlock = malloc(_poolSize); //Allocates the memory block.
		//Can fail if the pool size is too large

		if (_pMemBlock)
		{
			for (unsigned long i = 0; i < noOfChunks; i++)
			{
				//Linked list 
				Chunk* pCurUnit = (Chunk*)((char*)_pMemBlock + i * (sizeOfChunks + sizeof(Chunk)));

				pCurUnit->pPrev = nullptr;
				pCurUnit->pNext = _pFreeMemBlock;  

				if (_pFreeMemBlock != nullptr)
				{
					_pFreeMemBlock->pPrev = pCurUnit;
				}
				_pFreeMemBlock = pCurUnit;
			}
		}
	}

	~MemPool()
	{
		std::cout << "Deallocating all bytes" << std::endl;
		//free all pooled memory
		free(_pMemBlock);
	}

	// Allocate memory unit If memory pool can`t provide proper memory unit,
	// It will call system function.
	void* Alloc(unsigned long requestedBytes)
	{
		std::cout << "Allocating " << requestedBytes << std::endl;

		//if the requested bytes is larger than what we can allocate
		if (requestedBytes > _chunkSize || _pMemBlock == nullptr || _pFreeMemBlock == nullptr)
		{
			return malloc(requestedBytes);
		}

		// Now FreeList isn`t empty
		struct Chunk* pCurUnit = _pFreeMemBlock;
		_pFreeMemBlock = pCurUnit->pNext;  //Get a unit from free linkedlist.
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

		return (void*)((char*)pCurUnit + sizeof(struct Chunk));
	}

	//  To free a memory unit. If the pointer of parameter point to a memory unit,
	//  then insert it to "Free linked list". Otherwise, call system function "free".
	void Free(void* p)
	{
		if (_pMemBlock < p && p < (void*)((char*)_pMemBlock + _poolSize))
		{
			struct Chunk* pCurUnit = (struct Chunk*)((char*)p - sizeof(struct Chunk));

			struct Chunk* pPrev = pCurUnit->pPrev;
			struct Chunk* pNext = pCurUnit->pNext;
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
		else
		{
			std::cout << "Deallocating bytes" << std::endl;
			free(p);
		}
	}

private:
	//This is a doubly linked list
	struct Chunk
	{
		struct Chunk* pPrev, * pNext;
	};

	// Manage all Chunks with linked lists
	struct Chunk* _pAllocatedMemBlock; //Linked list of allocated memory blocks
	struct Chunk* _pFreeMemBlock;      //linked list of free memory blocks

	void* _pMemBlock;  //The start address of the memory pool.

	unsigned long _chunkSize;//Size of one memory chunk
	unsigned long _poolSize; //Memory pool size. Memory pool is make of memory unit.
};
