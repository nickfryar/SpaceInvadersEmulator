#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

#include <iostream>

namespace mem
{
	extern u8 memory[0xFFFF];

	inline u8 Read(u16 address)
	{
		return memory[address];
	}

	inline void LoadROM(u16 addr, u8 data)
	{
		memory[addr] = data;
	}

	inline void Write(u16 address, u8 data)
	{
		if (address < 0x2000)
		{
			std::cout << "ERROR: Cannot overwrite ROM.\n";
		}
		else if (address > 0x4000)
			memory[address - 0x2000] = data;
		else
			memory[address] = data;
	}

	inline void Increment(u16 address)
	{
		memory[address]++; // TODO: overflow? status?
	}

	inline void Decrement(u16 address)
	{
		memory[address]--; // TODO: overflow? status?
	}
}

#endif /*MEMORY_H*/
