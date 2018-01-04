#include "processor.h"

#include <iostream>
#include <cstring>

struct state i8080;

u8 dipswitch_1 = 0x00;
u8 dipswitch_2 = 0x00;
u16 shift_register = 0x0000;
u16 shift_offset = 0x0000;

void InitializeCPU()
{
	memset(&i8080, 0, sizeof(i8080));
}

u16 GetPC()
{
	return PC;
}

u8 GetReg(int index)
{
	return i8080.registers[index];
}

u8 GetDipSwitch(int num)
{
	if (num == 0)
		return dipswitch_1;
	else
		return dipswitch_2;
}
