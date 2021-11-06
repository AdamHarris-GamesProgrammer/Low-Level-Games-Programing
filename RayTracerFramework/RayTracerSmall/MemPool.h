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
		_pMemBlock = malloc(_poolSize); //Allocate a memory block.
		if (_pMemBlock)
		{
			for (unsigned long i = 0; i < noOfChunks; i++)
			{
				struct Chunk* pCurUnit = (struct Chunk*)((char*)_pMemBlock + i * (sizeOfChunks + sizeof(struct Chunk)));

				pCurUnit->pPrev = nullptr;
				pCurUnit->pNext = _pFreeMemBlock;    //Insert the new unit at head.

				if (nullptr != _pFreeMemBlock)
				{
					_pFreeMemBlock->pPrev = pCurUnit;
				}
				_pFreeMemBlock = pCurUnit;
			}
		}
	}

	~MemPool()
	{
		free(_pMemBlock);
	}

	// Allocate memory unit If memory pool can`t provide proper memory unit,
	// It will call system function.
	void* Alloc(unsigned long requestedBytes)
	{
		if (requestedBytes > _chunkSize ||
			_pMemBlock == nullptr || _pFreeMemBlock == nullptr)
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
			free(p);
		}
	}

private:
	//This is essentially a doubly linked list
	struct Chunk
	{
		struct Chunk* pPrev, * pNext;
	};

	// Manage all Chunks with linked lists
	struct Chunk* _pAllocatedMemBlock; //Pointer to the start of the allocated memory block
	struct Chunk* _pFreeMemBlock;      //Pointer to the start of the free memory block

	void* _pMemBlock;  //The start address of the memory pool.

	unsigned long    _chunkSize;
	unsigned long    _poolSize; //Memory pool size. Memory pool is make of memory unit.
};
