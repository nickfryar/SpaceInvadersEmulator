#ifndef OPCODES_H
#define OPCODES_H

#include "common.h"
#include "processor.h"

/********** Carry Bit Instructions **********/
// Compliment Carry
inline int CMC()
{
	CARRY = ~CARRY;
	PC++;
	return 4;
}
// Set Carry
inline int STC()
{
	CARRY = 1;
	PC++;
	return 4;
}



/********** Single Register Instructions **********/
// Increment Register or Memory
inline int INR(int DDD)
{
	if (DDD == 6)
	{
		// Increment memory
		u8 num = mem::Read(H_L);
		mem::Increment(H_L);
		PC++;

		// Status bits
		SetZero(mem::Read(H_L));
		SetSign(mem::Read(H_L));
		SetParity(mem::Read(H_L));
		SetAuxCarry(num, (u8)1);
	}
	else
	{
		// Increment register
		u8 num = i8080.registers[DDD];
		i8080.registers[DDD]++;
		PC++;

		// Status bits
		SetZero(i8080.registers[DDD]);
		SetSign(i8080.registers[DDD]);
		SetParity(i8080.registers[DDD]);
		SetAuxCarry(num, (u8)1);
	}

	PC++;
	return (DDD == 6) ? 10 : 5;
}
// Decrement Register or Memory
inline int DCR(int DDD)
{
	u8 num;
	if (DDD == 6)
	{
		// Decrement memory
		num = mem::Read(H_L);
		mem::Decrement(H_L);

		// Status bits
		SetZero(mem::Read(H_L));
		SetSign(mem::Read(H_L));
		SetParity(mem::Read(H_L));
		SetAuxBorrow(num, (u8)1);
	}
	else
	{
		// Decrement register
		num = i8080.registers[DDD];
		i8080.registers[DDD]--;

		// Status bits
		SetZero(i8080.registers[DDD]);
		SetSign(i8080.registers[DDD]);
		SetParity(i8080.registers[DDD]);
		SetAuxBorrow(num, (u8)1);
	}

	PC++;
	return (DDD == 6) ? 10 : 5;
}
// Complement Accumulator
inline int CMA()
{
	ACC = ~ACC;
	PC++;
	// No status.
	return 4;
}
// Decimal Adjust Accumulator
inline int DAA()
{
	if ((ACC & 0x0F) > 0x9 || AUX_CARRY == 1)	// step 1
	{
		ACC += 0x6;
		AUX_CARRY = 1;
	}
	else
		AUX_CARRY = 0;

	if ((ACC & 0xF0) > 0x9 || CARRY == 1)		// step 2
	{
		ACC = ((((ACC & 0xF0) >> 2) + 0x6) << 4) | (ACC & 0x0F);
		CARRY = 1;
	}
	else
		CARRY = 0;

	// Status bits
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);

	PC++;

	return 4;
}



/********** NOP Instruction **********/
// No Operation
inline int NOP()
{
	PC++;
	// No status.
	return 4;
}



/********** Data Transfer Instructions **********/
// Move Byte from Src to Dst
inline int MOV(int SSS, int DDD)
{
	// Get src
	int src;
	if (SSS == 6)
		src = mem::Read(H_L);
	else
		src = i8080.registers[SSS];

	// Get dst
	if (DDD == 6)
		mem::Write(H_L, src);
	else
		i8080.registers[DDD] = src;

	PC++;
	// No status.
	return (SSS == 6 || DDD == 6) ? 7 : 5;
}
// Store Accumulator
inline int STAX(int X)
{
	int upper_index = (X) ? 2 : 0;
	mem::Write((i8080.registers[upper_index] << 8) | i8080.registers[upper_index + 1], ACC);
	PC++;
	// No status.
	return 7;
}
// Load Accumulator
inline int LDAX(int X)
{
	int upper_index = (X) ? 2 : 0;
	ACC = mem::Read((i8080.registers[upper_index] << 8) | i8080.registers[upper_index + 1]);
	PC++;
	// No status.
	return 7;
}



/*********** Register or Memory to Accumulator Instructions **********/
// Add Register or Memory to Accumulator
inline int ADD(int RP)
{
	u8 a = ACC;
	u8 b = (RP == 6) ? mem::Read(H_L) : i8080.registers[RP];
	ACC += b;

	// Status bits
	SetZero(ACC);
	SetSign(ACC);
	SetCarry(a, b);
	SetAuxCarry(a, b);

	PC++;
	return (RP == 6) ? 7 : 4;
}
// Add Register or Memory to Accumulator with Carry
inline int ADC(int RP)
{
	u8 a = ACC;
	u8 b = (RP == 6) ?
		mem::Read(H_L) + CARRY : i8080.registers[RP] + CARRY;
	ACC += b;

	// Status bits
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);
	SetCarry(a, b);
	SetAuxCarry(a, b);

	PC++;
	return (RP == 6) ? 7 : 4;
}
// Subtract Register or Memoryfrom Accumulator
inline int SUB(int RP)
{
	u8 a = ACC;
	u8 b = (RP == 6) ? -mem::Read(H_L) : -i8080.registers[RP];
	ACC += b;

	// Status bits
	SetZero(ACC);
	SetSign(ACC);
	SetBorrow(a, b);
	SetAuxBorrow(a, b);

	PC++;
	return (RP == 6) ? 7 : 4;
}
// Subtract Register or Memory from Accumulator with Carry
inline int SBB(int RP)
{
	u8 a = ACC;
	u8 b = (RP == 6) ?
		-mem::Read(H_L) + CARRY : -i8080.registers[RP] + CARRY;
	ACC += b;

	// Status bits
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);
	SetBorrow(a, b);
	SetAuxBorrow(a, b);

	PC++;
	return (RP == 6) ? 7 : 4;
}
// Logical AND Register or Memory with Accumulator
inline int ANA(int RP)
{
	ACC &= (RP == 6) ? mem::Read(H_L) : i8080.registers[RP];

	// Status bits
	CARRY = 0;
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);

	PC++;
	return (RP == 6) ? 7 : 4;
}
// Logical XOR Register or Memory with Accumulator
inline int XRA(int RP)
{
	ACC ^= (RP == 6) ? mem::Read(H_L) : i8080.registers[RP];

	// Status bits
	CARRY = 0;
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);
	// TODO: aux carry?

	PC++;
	return (RP == 6) ? 7 : 4;
}
// Logical OR Register with Accumulator
inline int ORA(int RP)
{
	ACC |= (RP == 6) ? mem::Read(H_L) : i8080.registers[RP];

	// Status bits
	CARRY = 0;
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);

	PC++;
	return (RP == 6) ? 7 : 4;
}
// Compare Register or Memory with Accumulator
inline int CMP(int RP)
{
	u8 num = (RP == 6) ? mem::Read(H_L) : i8080.registers[RP];
	u8 result = ACC - num;

	// Status bits
	SetZero(result);
	SetSign(result);
	SetParity(result);
	SetBorrow(ACC, num);
	SetAuxBorrow(ACC, num);

	PC++;
	return (RP == 6) ? 7 : 4;
}


	
/********** Rotate Accumulator Instructions **********/
// Rotate Accumulator Left
inline int RLC()
{
	CARRY = ((ACC & 0x80) >> 7);
	ACC = ((ACC << 1) | CARRY);
	PC++;
	return 4;
}
// Rotate Accumulator Right
inline int RRC()
{
	CARRY = ((ACC & 0x01) << 7);
	ACC = ((ACC >> 1) | CARRY);
	PC++;
	return 4;
}
// Rotate Accumulator Left Through Carry
inline int RAL()
{
	int temp = CARRY;
	CARRY = ((ACC & 0x80) >> 7);
	ACC = ((ACC << 1) | temp);
	PC++;
	return 4;
}
// Rotate Accumulator Right Through Carry
inline int RAR()
{
	int temp = CARRY;
	CARRY = ((ACC & 0x01) << 7);
	ACC = ((ACC >> 1) | CARRY);
	PC++;
	return 4;
}



/********** Register Pair Instructions **********/
// Push
inline int PUSH(int RP)
{
	if (RP == 3)
	{
		// Push (PSW)
		mem::Write(SP - 1, ACC);
		mem::Write(SP - 2, GetStatusByte());
		SP -= 2;
	}
	else
	{
		int upper_index;
		switch (RP)
		{
		case 0: upper_index = 0; break;
		case 1: upper_index = 2; break;
		case 2: upper_index = 4; break;
		}

		// Push (register pair)
		mem::Write(SP - 1, i8080.registers[upper_index]);
		mem::Write(SP - 2, i8080.registers[upper_index + 1]);
		SP -= 2;
	}

	PC++;
	return 11;
}
// Pop
inline int POP(int RP)
{
	if (RP == 3)
	{
		// Pop (PSW)
		SetStatusBits(mem::Read(SP));
		ACC = mem::Read(SP + 1);
		SP += 2;
	}
	else
	{
		int upper_index;
		switch (RP)
		{
		case 0: upper_index = 0; break;
		case 1: upper_index = 2; break;
		case 2: upper_index = 4; break;
		}

		// Pop (register pair)
		i8080.registers[upper_index] = mem::Read(SP + 1);
		i8080.registers[upper_index + 1] = mem::Read(SP);
		SP += 2;
	}

	PC++;
	return 10;
}
// Double Add
inline int DAD(int RP)
{
	if (RP == 3)
	{
		// TODO: SP !!
	}
	else
	{
		int upper_index;
		switch (RP)
		{
		case 0: upper_index = 0; break;
		case 1: upper_index = 2; break;
		case 2: upper_index = 4; break;
		}

		u16 RR =
			((i8080.registers[upper_index] << 8) | i8080.registers[upper_index + 1]);
		u16 HL = ((i8080.registers[4] << 8) | i8080.registers[5]);
		u16 temp = RR + HL;
		i8080.registers[4] = ((temp & 0xFF00) >> 8);
		i8080.registers[5] = (temp & 0x00FF);

		// Status bits
		SetCarry(RR, HL);
	}

	PC++;
	return 10;
}
// Increment Register Pair
inline int INX(int RP)
{
	if (RP == 3)
	{
		// TODO: SP !!
	}
	else
	{
		int upper_index;
		switch (RP)
		{
		case 0: upper_index = 0; break;
		case 1: upper_index = 2; break;
		case 2: upper_index = 4; break;
		}

		u16 temp = // TODO: signed short?
			((i8080.registers[upper_index] << 8) | i8080.registers[upper_index + 1]) + 1;
		i8080.registers[upper_index] = ((temp & 0xFF00) >> 8);
		i8080.registers[upper_index + 1] = (temp & 0x00FF);
	}

	PC++;
	// No status.
	return 5;
}
// Decrement Register Pair
inline int DCX(int RP)
{
	if (RP == 3)
	{
		// TODO: SP !!
	}
	else
	{
		int upper_index;
		switch (RP)
		{
		case 0: upper_index = 0; break;
		case 1: upper_index = 2; break;
		case 2: upper_index = 4; break;
		}

		u16 temp = // TODO: signed short?
			((i8080.registers[upper_index] << 8) | i8080.registers[upper_index + 1]) - 1;
		i8080.registers[upper_index] = (temp & 0xFF00);
		i8080.registers[upper_index + 1] = (temp & 0x00FF);
	}

	PC++;
	// No status.
	return 5;
}
// Exchange Registers
inline int XCHG()
{
	// Exchange HL and DE register pairs
	u8 t1, t2;
	t1 = i8080.registers[2];
	t2 = i8080.registers[3];
	i8080.registers[2] = i8080.registers[4];
	i8080.registers[3] = i8080.registers[5];
	i8080.registers[4] = t1;
	i8080.registers[5] = t2;

	PC++;
	// No status.
	return 5;
}
// Exchange Stack
inline int XTHL()
{
	// Exchange HL register pair with stack
	u8 t1, t2;
	t1 = i8080.registers[4];
	t2 = i8080.registers[5];
	i8080.registers[4] = mem::Read(SP + 1);
	i8080.registers[5] = mem::Read(SP);
	mem::Write(SP + 1, t1);
	mem::Write(SP, t2);

	PC++;
	// No status.
	return 18;
}
// Load SP from H and L
inline int SPHL()
{
	SP = H_L;
	PC++;
	// No status.
	return 5;
}



/********** Immediate Instructions **********/
// Load Immediate Data
inline int LXI(int RP)
{
	if (RP == 3)
	{
		SP = NextAddress();
		PC += 3;
	}
	else
	{
		int upper_index; // TODO: inline
		switch (RP)
		{
		case 0: upper_index = 0; break;
		case 1: upper_index = 2; break;
		case 2: upper_index = 4; break;
		}

		i8080.registers[upper_index + 1] = NextByte();
		PC++;
		i8080.registers[upper_index] = NextByte();
		PC += 2;
	}
	// No status.
	return 10;
}
// Move Immedate Data
inline int MVI(int REG)
{
	int cycles;

	if (REG == 3)
	{
		mem::Write(H_L, NextByte());
		cycles = 10;
	}
	else
	{
		i8080.registers[REG] = NextByte();
		cycles = 7;
	}

	PC += 2;
	// No status.
	return cycles;
}
// Add Immediate to Accumulator
inline int ADI()
{
	u8 a = ACC;
	u8 b = mem::Read(++PC);
	ACC += b;

	// Status bits
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);
	SetCarry(a, b);
	SetAuxCarry(a, b);

	PC++;
	return 7;
}
// Add Immediate to Accumulator with Carry
inline int ACI()
{
	u8 a = ACC;
	u8 b = mem::Read(++PC) + CARRY;
	ACC += b;

	// Status bits
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);
	SetCarry(a, b);
	SetAuxCarry(a, b);

	PC++;
	return 7;
}
// Subtract Immediate from Accumulator
inline int SUI()
{
	u8 a = ACC;
	u8 b = -mem::Read(++PC);
	ACC += b;

	// Status bits
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);
	SetBorrow(a, b);
	SetAuxBorrow(a, b);

	PC++;
	return 7;
}
// Subtract Immediate from Accumulator with Borrow
inline int SBI()
{
	u8 a = ACC;
	u8 b = -mem::Read(++PC) + CARRY;
	ACC += b;

	// Status bits
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);
	SetBorrow(a, b);
	SetAuxBorrow(a, b);

	PC++;
	return 7;
}
// AND Immediate with Accumulator
inline int ANI()
{
	ACC &= NextByte();

	// Status bits
	CARRY = 0;
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);

	PC += 2;
	return 7;
}
// XOR Immediate with Accumulator
inline int XRI()
{
	ACC ^= NextByte();

	// Status bits
	CARRY = 0;
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);

	PC += 2;
	return 7;
}
// OR Immediate with Accumulator
inline int ORI()
{
	ACC |= NextByte();

	// Status bits
	CARRY = 0;
	SetZero(ACC);
	SetSign(ACC);
	SetParity(ACC);

	PC += 2;
	return 7;
}
// Compare Immediate with Accumulator
inline int CPI()
{
	u8 num = NextByte();
	u8 result = ACC - num;

	// Status bits
	SetZero(result);
	SetSign(result);
	SetParity(result);
	SetBorrow(ACC, num);
	SetAuxBorrow(ACC, num);

	PC += 2;
	return 7;
}



/********** Direct Addressing Instructions **********/
// Store Accumulator Direct
inline int STA()
{
	mem::Write(NextAddress(), ACC);

	PC += 3;
	// No status.
	return 13;
}
// Load Accumulator Direct
inline int LDA()
{
	ACC = mem::Read(NextAddress());

	PC += 3;
	// No status.
	return 13;
}
// Store H and L Direct
inline int SHLD()
{
	u16 addr = NextAddress();
	mem::Write(addr, i8080.registers[5]);
	mem::Write(addr + 1, i8080.registers[4]);

	PC += 3;
	// No status.
	return 16;
}
// Load H and L Direct
inline int LHLD()
{
	u16 addr = NextAddress();
	i8080.registers[5] = mem::Read(addr);
	i8080.registers[4] = mem::Read(addr + 1);

	PC += 3;
	// No status.
	return 16;
}



/********** Jump Instructions **********/
// Load Program Counter
inline int PCHL()
{
	PC = ((i8080.registers[4] << 8) | i8080.registers[5]);
	// No status.
	return 5;
}
// Jump
inline int JMP()
{
	Jump();
	// No status.
	return 10;
}
// Jump if Carry
inline int JC()
{
	if (CARRY)
		Jump();
	else
		PC += 3;
	// No status.
	return 10;
}
// Jump if No Carry
inline int JNC()
{
	if (CARRY)
		PC += 3;
	else
		Jump();
	// No status.
	return 10;
}
// Jump if Zero
inline int JZ()
{
	if (ZERO)
		Jump();
	else
		PC += 3;
	// No status.
	return 10;
}
// Jump if Not Zero
inline int JNZ()
{
	if (ZERO != 0)
		PC += 3;
	else
		Jump();
	// No status.
	return 10;
}
// Jump if Minus
inline int JM()
{
	if (SIGN)
		Jump();
	else
		PC += 3;
	// No status.
	return 10;
}
// Jump if Positive
inline int JP()
{
	if (SIGN)
		PC += 3;
	else
		Jump();
	// No status.
	return 10;
}
// Jump if Parity Even
inline int JPE()
{
	if (PARITY)
		Jump();
	else
		PC += 3;
	// No status.
	return 10;
}
// Jump if Parity Odd
inline int JPO()
{
	if (PARITY)
		PC += 3;
	else
		Jump();
	// No status.
	return 10;
}



/********** Call Subroutine Instructions **********/
// Call
inline int CALL()
{
	Call();
	return 17;
}
// Call if Carry
inline int CC()
{
	if (CARRY)
		Call();
	else
		PC += 3;
	return 17;
}
// Call if No Carry
inline int CNC()
{
	if (CARRY)
		PC += 3;
	else
		Call();
	return 17;
}
// Call if Zero
inline int CZ()
{
	if (ZERO)
		Call();
	else
		PC += 3;
	return 17;
}
// Call if Not Zero
inline int CNZ()
{
	if (ZERO)
		PC += 3;
	else
		Call();
	return 17;
}
// Call if Minus
inline int CM()
{
	if (SIGN)
		Call();
	else
		PC += 3;
	return 17;
}
// Call if Plus
inline int CP()
{
	if (SIGN)
		PC += 3;
	else
		Call();
	return 17;
}
// Call if Parity Even
inline int CPE()
{
	if (PARITY)
		Call();
	else
		PC += 3;
	return 17;
}
// Call if Parity Odd
inline int CPO()
{
	if (PARITY)
		PC += 3;
	else
		Call();
	return 17;
}



/********** Return from Subroutine Instructions **********/
// Return
inline int RET()
{
	PC = StackPop();
	return 10;
}
// Return if Carry
inline int RC()
{
	if (CARRY)
		PC = StackPop();
	else
		PC++;
	return 11;
}
// Return if No Carry
inline int RNC()
{
	if (CARRY)
		PC++;
	else
		PC = StackPop();
	return 11;
}
// Return if Zero
inline int RZ()
{
	if (ZERO)
		PC = StackPop();
	else
		PC++;
	return 11;
}
// Return if Not Zero
inline int RNZ()
{
	if (ZERO)
		PC++;
	else
		PC = StackPop();
	return 11;
}
// Return if Minus
inline int RM()
{
	if (SIGN)
		PC = StackPop();
	else
		PC++;
	return 11;
}
// Return if Plus
inline int RP()
{
	if (SIGN)
		PC++;
	else
		PC = StackPop();
	return 11;
}
// Return if Parity Even
inline int RPE()
{
	if (PARITY)
		PC = StackPop();
	else
		PC++;
	return 11;
}
// Return if Parity Odd
inline int RPO()
{
	if (PARITY)
		PC++;
	else
		PC = StackPop();
	return 11;
}



/* RST Instruction */
// Restart
inline int RST(int EXP)
{
	StackPush(PC + 1);
	PC = (EXP << 3);
	return 11;
}



/********** Interrupt Flip-flop Instructions **********/
// Enable Interrupts
inline int EI()
{
	i8080.INTE = 1;
	PC++;
	return 4;
}
// Disable Interrupts
inline int DI()
{
	i8080.INTE = 0;
	PC++;
	return 4;
}



/********** I/O Instructions **********/
// Input
inline int IN()
{
	ACC = Input(NextByte());
	PC += 2;
	return 10;
}
// Output
inline int OUT()
{
	Output(NextByte(), ACC);
	PC += 2;
	return 10;
}



/* HLT Halt Instruction */
// Halt
inline int HLT()
{
	// unnecessary

	return 7;
}

#endif /*OPCODES_H*/
