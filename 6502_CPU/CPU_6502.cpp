#include <iostream>
#include <string>
#include <vector>
#include "olcPixelGameEngine.h"
#include "CPU_6502.h"

using byte = unsigned char;
using word = unsigned short;

CPU_6502::CPU_6502()
{
	RAM = new byte[0x10000];

	RAM[0xFFFC] = 0x00;
	RAM[0xFFFD] = 0x02;
}

CPU_6502::~CPU_6502()
{
	delete[] RAM;
}

void CPU_6502::mon_clear()
{
	for (int i = 0x4000; i < 0x8400; i++)
		RAM[i] = 0;

	RAM[0xC] = 0;
}

void CPU_6502::init()
{
	rSP = 0xFF;
	setFlag(fI, 0);

	rPC = getWord(0xFFFC);
	
	RAM[0xE] = 0;
	RAM[0XD] = 0;
}

void CPU_6502::start()
{
	/*byte hex[] = {0xa9, 0x20, 0x85, 0x0a, 0xa9, 0x42, 0x38, 0xe5, 0x0a, 0x00};
	for (int i = 0; i < sizeof(hex); i++)
	{
		RAM[i] = hex[i];
	}*/

	int i = 0;

	while (execute() && i < 5000)
	{
		std::printf("%3d. PC: %02x A: %02x X: %02x Y: %02x SP: %02x P: ", i, rPC, rA, rX, rY, rSP);
		for (int i = 0; i < 8; i++)
		{
			if(i != f_ && ((rS >> i) & 1) == 1)
				switch (i)
				{
				case fC:
					printf("C ");
					break;
				case fZ:
					printf("Z ");
					break;
				case fI:
					printf("I ");
					break;
				case fD:
					printf("D ");
					break;
				case fB:
					printf("B ");
					break;
				case fV:
					printf("V ");
					break;
				case fN:
					printf("N ");
					break;
				}
		}
		std::printf("\n");
		i++;
	}
	printf("A: %d\n", rA);

	printf("PC: %d\n", rPC);
}

bool CPU_6502::getFlag(status_flag sf)
{
	return (rS >> sf & 0x1);
}

void CPU_6502::setFlag(status_flag sf, bool val)
{
	rS = ((rS & (0xFFFF << (sf + 1))) + (val << sf) + (rS & ((1 << sf) - 1)));
}

word CPU_6502::getWord(word ptr)
{
	return (word(RAM[ptr + 1]) << 8) + RAM[ptr]; //LLHH, little-endian
}

unsigned int CPU_6502::subtract(byte op1, byte op2)
{
	op2 = ~op2;

	return (unsigned int)op1 + (unsigned int)(op2 + 1);
}

bool CPU_6502::neg(unsigned int num)
{
	return (num >> 7) & 0x1;
}

word CPU_6502::fetch(addressing_mode addr, bool incPC)
{
	switch (addr)
	{
	case imm:
		return rPC++;
	case abs:
	{
		word a = getWord(rPC);
		if(incPC)
			rPC += 2;
		return a;
	}
	case zpg:
		return RAM[rPC++];
	case ind:
	{
		word a = getWord(RAM[rPC]);
		rPC += 2;
		return a;
	}
	case abs_X:
	{
		word a = getWord(rPC) + rX;
		rPC += 2;
		return a;
	}
	case abs_Y:
	{
		word a = getWord(rPC) + rY;
		rPC += 2;
		return a;
	} 
	case zpg_X:
		return RAM[rPC++] + rX;
	case zpg_Y:
		return RAM[rPC++] + rY;
	case index_ind:
		return getWord(RAM[rPC++] + rX);
	case ind_index:
		return getWord(RAM[rPC++]) + rY;
	}
}

bool CPU_6502::execute(bool* VRAM_W, bool* waitingForInput)
{
	if (waitingForInput != nullptr)
	{
		if (RAM[rPC + 1] == 0xE && RAM[rPC] == 0xA5 && (RAM[0xE] & 1) == 0)
		{
			*waitingForInput = true;
		}
	}

	word opcode = RAM[rPC++];
	int lo = opcode & 15;
	int hi = (opcode >> 4) & 15;

	addressing_mode addr = INSTS[hi][lo].Addr;

	if (VRAM_W != nullptr)
		*VRAM_W = false;

	switch (INSTS[hi][lo].name)
	{
	case ADC:
	{
		unsigned int mem = RAM[fetch(addr)];
		unsigned int res = (unsigned int)rA + mem + (unsigned int)getFlag(fC);

		bool negA = neg(rA);
		bool negM = neg(mem);
		bool negRes = neg(res);
		setFlag(fN, negRes);
		setFlag(fZ, ((res & 0xFF) == 0));
		setFlag(fC, (res >> 8));
		setFlag(fV, ((!negRes && negA && negM) || (negRes && !negA && !negM)));
		rA = byte(res & 0xFF);

		return true;
	}

	case AND:
	{
		rA = rA & RAM[fetch(addr)];

		setFlag(fN, (rA >> 7));
		setFlag(fZ, (rA == 0));

		return true;
	}

	case ASL:
	{
		byte* op;
		if (addr == acc)
			op = &rA;
		else
			op = &RAM[fetch(addr)];

		setFlag(fC, *op >> 7);
		*op <<= 1;

		setFlag(fN, (*op >> 7));
		setFlag(fZ, (*op == 0));

		return true;
	}

	case BCC:
	{
		if (!((rS >> fC) & 1))
			rPC += 1 + *(signed char*)(&RAM[rPC]);
		else
			rPC++;

		return true;
	}

	case BCS:
	{
		if (((rS >> fC) & 1))
			rPC += 1 + *(signed char*)(&RAM[rPC]);
		else
			rPC++;
		
		return true;
	}

	case BEQ:
	{
		if (((rS >> fZ) & 1))
			rPC += 1 + *(signed char*)(&RAM[rPC]);
		else
			rPC++;

		return true;
	}

	case BIT:
	{
		byte mem = RAM[fetch(addr)];
		setFlag(fN, mem >> 7);
		setFlag(fV, mem >> 6);
		setFlag(fZ, !(rA & rS));

		return true;
	}

	case BMI:
	{
		if (((rS >> fN) & 1))
			rPC += 1 + *(signed char*)(&RAM[rPC]);
		else
			rPC++;
		return true;
	}

	case BNE:
	{
		if (!((rS >> fZ) & 1))
			rPC += 1 + *(signed char*)(&RAM[rPC]);
		else
			rPC++;
		return true;
	}

	case BPL:
	{
		if (!((rS >> fN) & 1))
			rPC += 1 + *(signed char*)(&RAM[rPC]);
		else
			rPC++;
		return true;
	}

	case BRK:
	{
		setFlag(fI, 1);
		word rPC2 = rPC + 2;
		RAM[0x0100 + rSP--] = rPC2 & 0xFF;
		RAM[0x0100 + rSP--] = rPC2 >> 8;
		RAM[0x0100 + rSP--] = rS;

		return false;
	}

	case BVC:
	{
		if (!((rS >> fV) & 1))
			rPC += 1 + *(signed char*)(&RAM[rPC]);
		else
			rPC++;
		return true;
	}

	case BVS:
	{
		if (((rS >> fV) & 1))
			rPC += 1 + *(signed char*)(&RAM[rPC]);
		else
			rPC++;
		return true;
	}

	case CLC:
	{
		setFlag(fC, 0);

		return true;
	}

	case CLD:
	{
		setFlag(fD, 0);

		return true;
	}

	case CLI:
	{
		setFlag(fI, 0);

		return true;
	}

	case CLV:
	{
		setFlag(fV, 0);

		return true;
	}

	case CMP:
	{
		unsigned int res = subtract(rA, RAM[fetch(addr)]);
		setFlag(fN, neg(res));
		setFlag(fZ, ((res & 0xFF) == 0));
		setFlag(fC, (res >> 8));

		return true;
	}

	case CPX:
	{
		unsigned int res = subtract(rX, RAM[fetch(addr)]);
		setFlag(fN, neg(res));
		setFlag(fZ, ((res & 0xFF) == 0));
		setFlag(fC, (res >> 8));

		return true;
	}

	case CPY:
	{
		unsigned int res = subtract(rY, RAM[fetch(addr)]);
		setFlag(fN, neg(res));
		setFlag(fZ, ((res & 0xFF) == 0));
		setFlag(fC, (res >> 8));

		return true;
	}

	case DEC:
	{
		byte& mem = RAM[fetch(addr)];
		mem--;

		setFlag(fN, (mem >> 7));
		setFlag(fZ, (mem == 0));

		return true;
	}

	case DEX:
	{
		rX--;

		setFlag(fN, (rX >> 7));
		setFlag(fZ, (rX == 0));

		return true;
	}

	case DEY:
	{
		rY--;

		setFlag(fN, (rY >> 7));
		setFlag(fZ, (rY == 0));

		return true;
	}

	case EOR:
	{
		rA = rA ^ RAM[fetch(addr)];

		setFlag(fN, (rA >> 7));
		setFlag(fZ, (rA == 0));

		return true;
	}

	case INC:
	{
		byte& mem = RAM[fetch(addr)];
		mem++;

		setFlag(fN, (mem >> 7));
		setFlag(fZ, (mem == 0));

		return true;
	}

	case INX:
	{
		rX++;

		setFlag(fN, (rX >> 7));
		setFlag(fZ, (rX == 0));

		return true;
	}

	case INY:
	{
		rY++;

		setFlag(fN, (rY >> 7));
		setFlag(fZ, (rY == 0));

		return true;
	}

	case JMP:
	{
		rPC = fetch(addr, false);
		
		return true;
	}

	case JSR:
	{
		rPC += 2;

		RAM[0x0100 + rSP--] = rPC & 0xFF;
		RAM[0x0100 + rSP--] = rPC >> 8;
		rPC -= 2;
		rPC = fetch(addr, false);
		
		return true;
	}

	case LDA:
	{
		bool clearKeyboardFlag = (RAM[rPC - 1] == 0xA5 && RAM[rPC] == 0xE && (RAM[0xE] & 1) != 0);

		rA = RAM[fetch(addr)];

		if (clearKeyboardFlag)
			RAM[0xE] &= 0xFE; //11111110

		setFlag(fN, (rA >> 7));
		setFlag(fZ, (rA == 0));

		return true;
	}

	case LDX:
	{
		rX = RAM[fetch(addr)];

		setFlag(fN, (rX >> 7));
		setFlag(fZ, (rX == 0));

		return true;
	}

	case LDY:
	{
		rY = RAM[fetch(addr)];

		setFlag(fN, (rY >> 7));
		setFlag(fZ, (rY == 0));

		return true;
	}

	case LSR:
	{
		byte* op;
		if (addr == acc)
			op = &rA;
		else
			op = &RAM[fetch(addr)];

		setFlag(fC, *op & 1);
		*op >>= 1;

		setFlag(fN, (*op >> 7));
		setFlag(fZ, (*op == 0));

		return true;
	}

	case NOP:
	{
		return true;
	}

	case ORA:
	{
		rA = rA | RAM[fetch(addr)];

		setFlag(fN, (rA >> 7));
		setFlag(fZ, (rA == 0));

		return true;
	}

	case PHA:
	{
		RAM[0x0100 + rSP] = rA;
		rSP--;

		return true;
	}

	case PHP:
	{
		setFlag(f_, 1);
		RAM[0x100 + rSP] = rS;
		rSP--;

		return true;
	}

	case PLA:
	{
		rSP++;
		rA = RAM[0x0100 + rSP];

		setFlag(fN, (rA >> 7));
		setFlag(fZ, (rA == 0));

		return true;
	}

	case PLP:
	{
		rSP++;
		rS = RAM[0x0100 + rSP];

		return true;
	}

	case ROL:
	{
		byte* op;
		if (addr == acc)
			op = &rA;
		else
			op = &RAM[fetch(addr)];

		bool cachedCarry = getFlag(fC);

		setFlag(fC, *op >> 7);
		*op <<= 1;
		if (cachedCarry)
			(*op)++;

		setFlag(fN, (*op >> 7));
		setFlag(fZ, (*op == 0));

		return true;
	}

	case ROR:
	{
		byte* op;
		if (addr == acc)
			op = &rA;
		else
			op = &RAM[fetch(addr)];

		bool cachedCarry = rS >> fC;

		setFlag(fC, *op & 1);
		*op >>= 1;
		if (cachedCarry)
			(*op) += (1 << 7);

		setFlag(fN, (*op >> 7));
		setFlag(fZ, (*op == 0));

		return true;
	}

	case RTI:
	{
		rS = RAM[rSP++];
		rPC = 0;
		rPC += RAM[0x0100 + rSP++] << 8;
		rPC += RAM[0x0100 + rSP++];

		return true;
	}

	case RTS:
	{
		rPC = 0;
		rPC += (RAM[0x0100 + ++rSP] << 8);
		rPC += RAM[0x0100 + ++rSP];
		
		return true;
	}

	case SBC:
	{
		unsigned int mem = ~RAM[fetch(addr)];
		unsigned int res = (unsigned int)rA + mem + (unsigned int)getFlag(fC);

		bool negA = neg(rA);
		bool negM = neg(mem);
		bool negRes = neg(res);
		setFlag(fN, negRes);
		setFlag(fZ, ((res & 0xFF) == 0));
		setFlag(fC, !(res >> 8));
		setFlag(fV, ((!negRes && negA && negM) || (negRes && !negA && !negM)));
		rA = byte(res & 0xFF);

		return true;
	}

	case SEC:
	{
		setFlag(fC, 1);

		return true;
	}

	case SED:
	{
		setFlag(fD, 1);

		return true;
	}

	case SEI:
	{
		setFlag(fI, 1);

		return true;
	}

	case STA:
	{
		word a = fetch(addr);
		RAM[a] = rA;

		if (VRAM_W != nullptr)
		{
			if (a >= 0x8000 && a < 0x8400)
				*VRAM_W = true;
		}

		return true;
	}

	case STX:
	{
		RAM[fetch(addr)] = rX;

		return true;
	}

	case STY:
	{
		RAM[fetch(addr)] = rY;

		return true;
	}

	case TAX:
	{
		rX = rA;

		setFlag(fN, (rX >> 7));
		setFlag(fZ, (rX == 0));

		return true;
	}

	case TAY:
	{
		rY = rA;

		setFlag(fN, (rY >> 7));
		setFlag(fZ, (rY == 0));

		return true;
	}

	case TSX:
	{
		rX = rSP;

		setFlag(fN, (rX >> 7));
		setFlag(fZ, (rX == 0));

		return true;
	}

	case TXA:
	{
		rA = rX;

		setFlag(fN, (rA >> 7));
		setFlag(fZ, (rA == 0));

		return true;
	}

	case TXS:
	{
		rSP = rX;

		setFlag(fN, (rS >> 7));
		setFlag(fZ, (rS == 0));

		return true;
	}

	case TYA:
	{
		rA = rY;

		setFlag(fN, (rA >> 7));
		setFlag(fZ, (rA == 0));

		return true;
	}

	default:
		return false;
	}

	return true;
}

