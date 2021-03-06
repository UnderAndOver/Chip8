// Chip8.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include "chip8.h"
Chip8 MyChip8;
void Chip8::initialize() {
	//initialize registers and memory once
	pc     = 0x200; //program counter starts at 0x200
	opcode = 0; //resets current opcode
	I      = 0; //resets index register
	sp     = 0; //resets stack pointer

	// Clear display
	// Clear stack
	// Clear registers V0-VF
	// Clear memory

	//load fontset
	for (int i = 0; i < 80; ++i) {
		memory[i] = chip8_fontset[i];
	}
	
	// Reset timers
}
void Chip8::emulateCycle() {
	//Fetch Opcode
	opcode = memory[pc] << 8 | memory[pc + 1]; // each opcode is 2bytes long, since memory is char array combining 2 elements is 1 opcode.
	//Decode Opcode-checks opcode table
	unsigned char a, b, c, x, y;// opcode is XXXX
	a = opcode & 0xFFF;//12-bit address operand Xnnn
	b = opcode & 0xFF; // 8-bit constant XXNN
	c = opcode & 0xF;  // 4-bit constant XXXn
	// x and y register operands
	x = opcode >> 8 & 0xF;
	y = opcode >> 4 & 0xF;
	//Execute Opcode - not implemented: 0NNN
	switch (opcode & 0xF000)//checks first nibble
	{
		case 0x0000: 
			switch (opcode & 0x00F) //checks last nibble
			{
				case 0x0000: // 0x00E0: Clear screen
					for (int i=0;i<64*32;i++)
						gfx[i] = 0;
					MyChip8.drawFlag = true;
					break;

				case 0x000E: // 0x00EE return from subroutine
					pc = stack[sp]-2;//adjust for pc+=2
					--sp;
					break;
				default:
					printf("Unkown opcode [0x0000]: 0x%X\n", opcode);
			}
			break;
		case 0x1000: //1NNN - JMP to NNN - pc=NNN
			pc = a;
			pc -= 2; //adjust for pc+=2
			break;
		case 0x2000: //2NNN - calls subroutine at NNN *(0xNNN)()
			stack[sp] = pc;
			++sp;
			pc = a;
			pc -= 2;//to adjust for pc+=2 at the end
			break;
		case 0x3000: //3XNN - skips next instruction if Vx == NN
			if (V[x] == b)
				pc += 2;
			break;
		case 0x4000: //4XNN - skips next instruction if Vx != NN
			if (V[x] != b)
				pc += 2;
			break;
		case 0x5000: //5XY0 - skips next instruction if Vx == Vy
			if (V[x] == V[y])
				pc += 2;
			break;
		case 0x6000: //6XNN - Vx = NN
			V[x] = b;
			break;
		case 0x7000: //7XNN - Vx += NN
			V[x] += b;
			break;
		case 0x8000: //8XYN
			switch (opcode & 0x00F)
			{
				case 0x0000: //8XY0 Vx = Vy
					V[x] = V[y];
					break;
				case 0x0001: //8XY1 Vx |= Vy
					V[x] |= V[y];
					break;
				case 0x0002: //8XY2 Vx &= Vy
					V[x] &= V[y];
					break;
				case 0x0003: //8XY3 Vx = Vx XOR Vy
					V[x] ^= V[y];
					break;
				case 0x0004: //8XY4 Vx += Vy
					if (V[x] + V[y] > 0xFF)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					V[x] += V[y];
					break;
				case 0x0005: //8XY5 Vx -= Vy
					if (V[x] > V[y])
						V[0xF] = 1;
					else
						V[0xF] = 0;
					V[x] -= V[y];
					break;
				case 0x0006: //8XY6 Vx = Vy >> 1
					V[0xF] = V[y] & 0xF;
					V[x] = V[y]>>1;
					break;
				case 0x0007: //8XY4 Vx = Vy -Vx
					V[x] = V[y]-V[x];
					break;
				case 0x000E: //8XY4 Vx = Vy << 1
					V[0xF] = (V[y] >> 12) & 0xF;
					V[x] = V[y]<<1;
					break;
				default:
					printf("Unkown opcode [0x8000]: 0x%X\n", opcode);
			}
			break;
		case 0x9000: //9XY0 - skips next instruction if Vx != Vy
			if (V[x] != V[y])
				pc += 2;
			break;
		case 0xA000: //ANNN - I = NNN
			I = a;
			break;
		case 0xB000: //BNNN -jumps to address NNN + V0
			pc = V[0] + a;
			pc -= 2; // adjust for pc+=2
			break;
		case 0xC000: //CXNN -  Vx = rand()&NN
			V[x] = rand()&b;
			break;
		case 0xD000: //DXYN - draw at x,y N height
			//x = V[(opcode & 0x0F00) >> 8];
			//y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for (int xline = 0; xline < 8; xline++)
				{
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (gfx[(x + xline + ((y + yline) * 64))] == 1)
							V[0xF] = 1;
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}

			drawFlag = true;
			break;
		case 0xE000: //E000 instructions
			switch (opcode & 0x00F)
			{
				case 0x000E: //EX9E - skips if key()==Vx
					if (key[V[x]] != 0)
						pc += 2;
					break;
				case 0x0001: //EXA1 - skips if key()!=Vx
					if (key[V[x]] == 0)
						pc += 2;
					break;
				default:
					printf("Unkown opcode [0xE000]: 0x%X\n", opcode);
			}
			break;
		case 0xF000: //F000 instructions
			switch (opcode & 0xFF)
			{
				case 0x0007: //FX07 - sets vx to value of the delay timer
					V[x] = delay_timer;
					break;
				case 0x000A: //FX0A - vx = get_key() waits for key and stores in vx
					V[x]=MyChip8.getKey();
					break;
				case 0x0015: //FX15 - sets delay timer to vx
					delay_timer = V[x];
					break;
				case 0x0018: //FX18 - sets sound timer to vx
					sound_timer = V[x];
					break;
				case 0x001E: //FX1E - adds VX to I
					I += V[x];
					break;
				case 0x0029: //FX29 - sets I to location of sprite for the character in VX 5 bytes per character
					I = (5 * V[x]) & 0xFFF;
					break;
				case 0x0033: //FX33 - BCD
					memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
					break;
				case 0x0055: //FX55 - stores V0-VX in memory starting at address I, I++ for each entry
					for (int i = 0; i <= x; ++i) {
						memory[I] = V[i];
						I += 1;
					}
					break;
				case 0x0065: //FX65 - fills V0-VX with values from memory starting at I, I++ for each entry
					for (int i = 0; i <= x; ++i) {
						V[i] = memory[I];
						I += 1;
					}
					break;
				default:
					printf("Unkown opcode [0xF000]: 0x%X\n", opcode);
			}
			break;
		default:
			printf("Unkown opcode [0x]: 0x%X\n", opcode);
	}
	pc += 2; //increase by 2 for next opcode
	//Update Timers
	if (delay_timer > 0)
		delay_timer -= 1;
	if (sound_timer > 0)
		sound_timer -= 1;
	if (sound_timer == 0)
		//play sound
}
void Chip8::loadGame(char* ROM) {
	FILE *f = fopen(ROM, "rb");
	if (f == NULL) {
		printf("ERROR: Couldn't Open File");
		exit(1);
	}
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);
	unsigned char *buffer = (unsigned char*)malloc(fsize);//size of file and 2-byte offset
	fread(buffer, fsize, 1, f);
	fclose(f);
	for (int i = 0; i < fsize; ++i)
		memory[i + 0x200] = buffer[i];
}
void Chip8::getKey()
{
}
void Chip8::setKeys()
{
}
int main(int argc,char** argv)
{
	setupGraphics();
	setupInput();
	//Initialize Chip8 and load game into memory
	MyChip8.initialize();
	MyChip8.loadGame(argv[1]);
	//Emulation Loop
	for (;;) {
		//emulates one cycle
		MyChip8.emulateCycle();

		//update screen if flag is set
		if (MyChip8.drawFlag)
			drawGraphics();

		//Store key press state
		MyChip8.setKeys();
	}
    return 0;
}

