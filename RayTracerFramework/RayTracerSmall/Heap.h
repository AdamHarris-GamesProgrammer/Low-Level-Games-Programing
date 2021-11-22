#pragma once
#include <iostream>
#include <mutex>
#define NAMELENGTH 16

struct Header;

enum ConsoleColor {
	WHITE,
	RED,
	YELLOW
};

class Heap
{
public:
	Heap(std::string name);
	void AllocateMemory(Header* header, int size);
	void DeallocateMemory(Header* header, int size);
	int GetAmountAllocated();
	std::string GetName() const { return _name; }

	void* operator new (size_t size);

	Header* pHead = NULL;

	void DisplayDebugInformation();
	void CheckIntegrity();

	Heap(const Heap&) = default;
	Heap& operator=(const Heap&) = default;
private:
	void SetConsoleColor(ConsoleColor color);

	int _totalAllocated;
	int _peak;
	std::string _name;
};

