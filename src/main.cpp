#include <iostream>
#include <SDL2/SDL.h>
#undef main

#include "processor.h"
#include "opcodes.h"
#include "memory.h"

#define WIDTH 224
#define HEIGHT 256
#define SCALE 3

#define REG_PAIR ((opcode & 0x30) >> 4)
#define DDD ((opcode & 0x38) >> 3)
#define EXP ((opcode & 0x38) >> 3)
#define SSS (opcode & 0x07)


SDL_Window *window;
SDL_Surface *surface, *surface_native;

bool running = true;

bool Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO)) return false;
	window = SDL_CreateWindow("8080", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * SCALE, HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	if (window == NULL) return false;
	surface = SDL_GetWindowSurface(window);
	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));
	SDL_UpdateWindowSurface(window);
	surface_native = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);

	atexit(SDL_Quit);

	InitializeCPU();

	return true;
}

bool LoadRom()
{
	int size;
	char *buffer;

	FILE *f0 = fopen("invaders.h", "rb");
	FILE *f1 = fopen("invaders.g", "rb");
	FILE *f2 = fopen("invaders.f", "rb");
	FILE *f3 = fopen("invaders.e", "rb");
	if (f0 == NULL || f1 == NULL || f2 == NULL || f3 == NULL)
		return false;

	// Bank 0
	fseek(f0, 0, 2);
	size = ftell(f0);
	fseek(f0, 0, 0);
	buffer = new char[size];
	fread(buffer, sizeof(char), size, f0);
	for (int i = 0; i < 0x800; i++)
		mem::LoadROM(i, buffer[i]);

	// Bank 1
	fseek(f1, 0, 2);
	size = ftell(f1);
	fseek(f1, 0, 0);
	buffer = new char[size];
	fread(buffer, sizeof(char), size, f1);
	for (int i = 0; i < 0x800; i++)
		mem::LoadROM(i + 0x800, buffer[i]);

	// Bank 2
	fseek(f2, 0, 2);
	size = ftell(f2);
	fseek(f2, 0, 0);
	buffer = new char[size];
	fread(buffer, sizeof(char), size, f2);
	for (int i = 0; i < 0x800; i++)
		mem::LoadROM(i + 0x1000, buffer[i]);

	// Bank 3
	fseek(f3, 0, 2);
	size = ftell(f3);
	fseek(f3, 0, 0);
	buffer = new char[size];
	fread(buffer, sizeof(char), size, f3);
	for (int i = 0; i < 0x800; i++)
		mem::LoadROM(i + 0x1800, buffer[i]);

	return true;
}

inline void GetInput()
{
	SDL_Event e;
	if (SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT)
		running = false;

	const Uint8 *state = SDL_GetKeyboardState(NULL);

	if (state[SDL_SCANCODE_LEFT])	dipswitch_1 |= 0x20;
	else							dipswitch_1 &= 0xDF;

	if (state[SDL_SCANCODE_RIGHT])	dipswitch_1 |= 0x40;
	else							dipswitch_1 &= 0xBF;

}

int ExecuteInstruction()
{
	u8 opcode = mem::Read(GetPC());

	switch (opcode)
	{
		/* 00000000 */
	case 0x00: return NOP();
		/* 00xxxxxx */
	case 0x07: return RLC();
	case 0x0F: return RRC();
	case 0x17: return RAL();
	case 0x1F: return RAR();
	case 0x22: return SHLD();
	case 0x27: return DAA();
	case 0x2A: return LHLD();
	case 0x2F: return CMA();
	case 0x32: return STA();
	case 0x37: return STC();
	case 0x3A: return LDA();
	case 0x3F: return CMC();
		/* 01110110 */
	case 0x76: return HLT();
		/* 11xxxxxx */
	case 0xC0: return RNZ();
	case 0xC2: return JNZ();
	case 0xC3: return JMP();
	case 0xC4: return CNZ();
	case 0xC6: return ADI();
	case 0xC8: return RZ();
	case 0xC9: return RET();
	case 0xCA: return JZ();
	case 0xCC: return CZ();
	case 0xCE: return ACI();
	case 0xCD: return CALL();
	case 0xD0: return RNC();
	case 0xD2: return JNC();
	case 0xD3: return OUT();
	case 0xD4: return CNC();
	case 0xD6: return SUI();
	case 0xD8: return RC();
	case 0xDA: return JC();
	case 0xDB: return IN();
	case 0xDC: return CC();
	case 0xDE: return SBI();
	case 0xE0: return RPO();
	case 0xE2: return JPO();
	case 0xE3: return XTHL();
	case 0xE4: return CPO();
	case 0xE6: return ANI();
	case 0xE8: return RPE();
	case 0xE9: return PCHL();
	case 0xEA: return JPE();
	case 0xEB: return XCHG();
	case 0xEC: return CPE();
	case 0xEE: return XRI();
	case 0xF0: return RP();
	case 0xF2: return JP();
	case 0xF3: return DI();
	case 0xF4: return CP();
	case 0xF6: return ORI();
	case 0xF8: return RM();
	case 0xF9: return SPHL();
	case 0xFC: return CM();
	case 0xFA: return JM();
	case 0xFB: return EI();
	case 0xFE: return CPI();
		/* Other */
	default:
		switch ((opcode & 0xC0) >> 6) // Get two leftmost bits
		{
			/* 00xxxxxx */
		case 0:
			if ((opcode & 0x07) < 4) // Get three rightmost bits
			{
				switch (opcode & 0x0F)
				{
				case 1:		return LXI(REG_PAIR);	// 00xx0001
				case 2:		return STAX(REG_PAIR);	// 00xx0010
				case 3:		return INX(REG_PAIR);	// 00xx0011
				case 9:		return DAD(REG_PAIR);	// 00xx1001
				case 10:	return LDAX(REG_PAIR);	// 00xx1010
				case 11:	return DCX(REG_PAIR);	// 00xx1011
				}
			}
			else
			{
				switch (opcode & 0x07)
				{
				case 4:		return INR(DDD);	// 00xxx100
				case 5:		return DCR(DDD);	// 00xxx101
				case 6:		return MVI(DDD);	// 00xxx110
				}
			}
			break;

			/* 01xxxxxx */
		case 1: return MOV(SSS, DDD);

			/* 10xxxxxx */
		case 2:
			switch ((opcode & 0x38) >> 3)
			{
			case 0: return ADD(SSS);	// 10000xxx
			case 1: return ADC(SSS);	// 10001xxx
			case 2: return SUB(SSS);	// 10010xxx
			case 3: return SBB(SSS);	// 10011xxx
			case 4: return ANA(SSS);	// 10100xxx
			case 5: return XRA(SSS);	// 10101xxx
			case 6: return ORA(SSS);	// 10110xxx
			case 7: return CMP(SSS);	// 10111xxx
			}
			break;

			/* 11xxxxxx */
		case 3:
			switch (opcode & 0x07)
			{
			case 1: return POP(REG_PAIR);	// 11xxx001
			case 5: return PUSH(REG_PAIR);	// 11xxx101
			case 7: return RST(EXP);		// 11xxx111
			}
			break;
		}
	}

	running = false; // TODO: handle invalid opcodes
	return 4;
}

void Emulate8080(int cycles)
{
	int elapsed_cycles = 0;

	while (elapsed_cycles < cycles)
	{
		elapsed_cycles += ExecuteInstruction();
	}
}

void GenerateInterrupt(int addr)
{
	StackPush(i8080.pc);
	i8080.pc = addr;
}

void Draw()
{
	int display[224 * 256];
	int c = 0;
	for (int i = 0x2400; i < 0x4000; i++)
		for (int j = 0; j < 8; j++)
		{
			display[c] = (mem::Read(i) & (1 << j));
			c++;
		}

	c = 0;
	u32 *pixels = (u32*)surface_native->pixels;
	for (int i = 0; i < WIDTH; i++)
	{
		pixels += (i) ? 1 : 0;
		c++;
		for (int j = HEIGHT - 1; j > 0; j--)
			pixels[WIDTH * j] = (display[c++]) ? 0x00ffffff : 0;
	}

	SDL_BlitScaled(surface_native, NULL, surface, NULL);
	SDL_UpdateWindowSurface(window);
}

int main(int argc, char *argv[])
{
	if (Initialize() & LoadRom())
	{
		int interrupt_switch = 0;
		while (running)
		{
			Emulate8080((2000000 / 60) / 2);
			if (i8080.INTE)
			{
				GenerateInterrupt((interrupt_switch) ? 0x10 : 0x08);
				interrupt_switch = ~interrupt_switch;
				i8080.INTE = 0;
			}
			Draw();
		}
	}
	else
		std::cout << "Error.\n";

	return 0;
}
