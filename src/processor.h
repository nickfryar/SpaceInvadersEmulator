#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "common.h"
#include "memory.h"


#include <iostream>

struct state
{
	u8 registers[8];	// B, C, D, E, H, L, _, A
	u16 pc;				// program counter
	u16 sp;				// stack pointer
	int status[5];		// status: S, Z, P, C, AC
	bool IE;

	bool INTE;
};
extern state i8080;

#define SP		i8080.sp
#define PC		i8080.pc
#define REGISTER		i8080.registers
#define STAT	i8080.status

extern u8 dipswitch_1;
extern u8 dipswitch_2;
extern u16 shift_register;
extern u16 shift_offset;

// Returns next byte of memory
inline u8 NextByte()
{
	return mem::Read(PC + 1);
}

// Returns next two bytes of memory
inline u16 NextShort()
{
	int value = ((mem::Read(PC + 1) << 8) | mem::Read(PC + 2));
	PC += 2;
	return value;
}

// Returns the next address in memory
inline u16 NextAddress()
{
	int address = ((mem::Read(PC + 2) << 8) | mem::Read(PC + 1));
	return address;
}

// Gets the status byte
inline u8 GetStatusByte()
{
	// S-Z-0-AC-0-P-1-C
	return ((STAT[0] << 7) | (STAT[1] << 6) | (STAT[4] << 4) | (STAT[2] << 2) | 0x2 | STAT[3]);
}

// Sets the status bits
inline void SetStatusBits(u8 status)
{
	// status: S, Z, P, C, AC
	STAT[0] = ((status & 0x80) >> 7);
	STAT[1] = ((status & 0x40) >> 6);
	STAT[2] = ((status & 0x04) >> 2);
	STAT[3] = (status & 0x01);
	STAT[4] = ((status & 0x10) >> 4);
}

// Push to the stack
inline void StackPush(u16 data)
{
	mem::Write(SP - 1, data & 0x00FF);
	mem::Write(SP - 2, (data & 0xFF00) >> 8);
	SP -= 2;
}

// Pop from the stack
inline u16 StackPop()
{
	u16 addr = ((mem::Read(SP) << 8) | mem::Read(SP + 1));
	SP += 2;
	return addr;
}

// Jump
inline void Jump()
{
	PC = NextAddress();
}

// Call
inline void Call()
{
	StackPush(PC + 3);
	Jump();
}

inline u8 Input(u8 port)
{
	switch (port)
	{
	case 1:
		return dipswitch_1;
	case 2:
		return dipswitch_2;
	case 3:
		// Shift register result
		return ((shift_register << shift_offset) >> 8);
		break;
	}

	return 0;
}

inline void Output(u8 port, u8 reg)
{
	switch (port)
	{
	case 2:
		// Shift register result offset
		shift_offset = (reg & 0x7);
		break;
	case 4:
		// Fill shift register
		shift_register = ((shift_register << 8) | reg);
		break;
	}
}

/********* Status Byte Functions *********/

inline void SetZero(u16 num)
{
	i8080.status[1] = (num == 0) ? 1 : 0;
}

inline void SetSign(u16 num)
{
	i8080.status[0] = (num >> 7);
}

inline void SetParity(u16 num)
{
	int parity = 0;
	while (num)
	{
		parity = !parity;
		num &= (num - 1);
	}
	i8080.status[2] = (parity) ? 0 : 1;
}

template <typename T>
inline void SetCarry(T num1, T num2)
{
	int res1 = num1 + num2;
	T res2 = num1 + num2;

	i8080.status[3] = (res1 == res2) ? 0 : 1;
}

template <typename T>
inline void SetAuxCarry(T num1, T num2)
{
	int byte1 = (num1 & 0xFF);
	T byte2 = ((num2 & 0xFF) & 0xFF);

	int res1 = byte1 + byte2;
	T res2 = byte1 + byte2;

	i8080.status[4] = (res1 == res2) ? 0 : 1;
}

template <typename T>
inline void SetBorrow(T num1, T num2)
{
	int res1 = num1 + num2;
	T res2 = num1 + num2;

	i8080.status[3] = (res1 == res2) ? 1 : 0;
}

template <typename T>
inline void SetAuxBorrow(T num1, T num2)
{
	int byte1 = (num1 & 0xFF);
	T byte2 = ((num2 & 0xFF) & 0xFF);

	int res1 = byte1 + byte2;
	T res2 = byte1 + byte2;

	i8080.status[4] = (res1 == res2) ? 1 : 0;
}

/********** Operand functions *********/

inline int ROMTA_Operand(int RP)
{
	if (RP == 6)
		return mem::Read(H_L);
	else
		return i8080.registers[RP];
}

/********** Other functions **********/

void InitializeCPU();
u16 GetPC();

#endif /*PROCESSOR_H*/
