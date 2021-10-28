#pragma once

#define deadCode 0xDEADC0DE

struct Header {
	int size;
	int check = deadCode;
};

struct Footer {
	int reserved;
	int check = deadCode;
};



void* operator new(size_t size);
void operator delete(void* pMem);

