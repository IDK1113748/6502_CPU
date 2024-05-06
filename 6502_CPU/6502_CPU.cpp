#include <iostream>
#include <string>
using namespace std;

class CPU_6502
{
	using byte = unsigned char;
	using word = unsigned short;

	word rPC;
	byte rSP;
	byte rA;
	byte rX;
	byte rY;
	byte rS;

public:
	void start()
	{
		rS = 0x2;
		rSP = 0x01FF;

		rPC = 0;
		rA = 0;
		rX = 0;
		rY = 0;
		for (int i = 0; i < 0x10000; i++)
			RAM[i] = 0x00;

		RAM[0] = 0x69;
		RAM[1] = 23;
		RAM[2] = 0x69;
		RAM[3] = 12;

		printf("A: %d\n", rA);

		while (execute()) {}

		printf("A: %d\n", rA);

		printf("PC: %d\n", rPC);
	}

private:
	enum addressing_mode
	{
		impl,
		imm,
		acc,
		rel,
		abs,
		zpg,
		ind,
		abs_X,
		abs_Y,
		zpg_X,
		zpg_Y,
		index_ind,
		ind_index
	};

	enum status_flag
	{
		fC,
		fZ,
		fI,
		fD,
		fB,
		f_,
		fV,
		fN
	};

	bool getFlag(status_flag sf)
	{
		return (rS >> sf & 0x1);
	}

	void setFlag(status_flag sf, bool val)
	{
		rS = (rS & (0xFFFF << (sf + 1)) + val << rS + val & (1 << sf - 1));
	}

	enum inst_name
	{
		ADC,
		AND,
		ASL,
		BCC,
		BCS,
		BEQ,
		BIT,
		BMI,
		BNE,
		BPL,
		BRK,
		BVC,
		BVS,
		CLC,
		CLD,
		CLI,
		CLV,
		CMP,
		CPX,
		CPY,
		DEC,
		DEX,
		DEY,
		EOR,
		INC,
		INX,
		INY,
		JMP,
		JSR,
		LDA,
		LDX,
		LDY,
		LSR,
		NOP,
		ORA,
		PHA,
		PHP,
		PLA,
		PLP,
		ROL,
		ROR,
		RTI,
		RTS,
		SBC,
		SEC,
		SED,
		SEI,
		STA,
		STX,
		STY,
		TAX,
		TAY,
		TSX,
		TXA,
		TXS,
		TYA
	};

	const string inst_name_str[57] =
	{
		"ADC",
		"AND",
		"ASL",
		"BCC",
		"BCS",
		"BEQ",
		"BIT",
		"BMI",
		"BNE",
		"BPL",
		"BRK",
		"BVC",
		"BVS",
		"CLC",
		"CLD",
		"CLI",
		"CLV",
		"CMP",
		"CPX",
		"CPY",
		"DEC",
		"DEX",
		"DEY",
		"EOR",
		"INC",
		"INX",
		"INY",
		"JMP",
		"JSR",
		"LDA",
		"LDX",
		"LDY",
		"LSR",
		"NOP",
		"ORA",
		"PHA",
		"PHP",
		"PLA",
		"PLP",
		"ROL",
		"ROR",
		"RTI",
		"RTS",
		"SBC",
		"SEC",
		"SED",
		"SEI",
		"STA",
		"STX",
		"STY",
		"TAX",
		"TAY",
		"TSX",
		"TXA",
		"TXS",
		"TYA",
		"err"
	};

	struct
	{
		inst_name name;
		addressing_mode Addr;
	} INSTS[16][15] =
	{ 
		{{BRK, impl}, {ORA, index_ind}, {},         {}, {},           {ORA, zpg},   {ASL, zpg},   {}, {PHP, impl}, {ORA, imm},   {ASL, acc},  {}, {},           {ORA, abs},   {ASL, abs}},
		{{BPL, rel},  {ORA, ind_index}, {},         {}, {},           {ORA, zpg_X}, {ASL, zpg_X}, {}, {CLC, impl}, {ORA, abs_Y}, {},          {}, {},           {ORA, abs_X}, {ASL, abs_X}},
		{{JSR, abs},  {AND, index_ind}, {},         {}, {BIT, zpg},   {AND, zpg},   {ROL, zpg},   {}, {PLP, impl}, {AND, imm},   {ROL, acc},  {}, {BIT, abs},   {AND, abs},   {ROL, abs}},
		{{BMI, rel},  {AND, ind_index}, {},         {}, {},           {AND, zpg_X}, {ROL, zpg_X}, {}, {SEC, impl}, {AND, abs_Y}, {},          {}, {},           {AND, abs_X}, {ROL, abs_X}},
		{{RTI, impl}, {EOR, index_ind}, {},         {}, {},           {EOR, zpg},   {LSR, zpg},   {}, {PHA, impl}, {EOR, imm},   {LSR, acc},  {}, {JMP, abs},   {EOR, abs},   {LSR, abs}},
		{{BVC, rel},  {EOR, ind_index}, {},         {}, {},           {EOR, zpg_X}, {LSR, zpg_X}, {}, {CLI, impl}, {EOR, abs_Y}, {},          {}, {},           {EOR, abs_X}, {LSR, abs_X}},
		{{RTS, impl}, {ADC, index_ind}, {},         {}, {},           {ADC, zpg},   {ROR, zpg},   {}, {PLA, impl}, {ADC, imm},   {ROR, acc},  {}, {JMP, ind},   {ADC, abs},   {ROR, abs}},
		{{BVS, rel},  {ADC, ind_index}, {},         {}, {},           {ADC, zpg_X}, {ROR, zpg_X}, {}, {SEI, impl}, {ADC, abs_Y}, {},          {}, {},           {ADC, abs_X}, {ROR, abs_X}},
		{{},          {STA, index_ind}, {},         {}, {STY, zpg},   {STA, zpg},   {STX, zpg},   {}, {DEY, impl}, {},           {TXA, impl}, {}, {STY, abs},   {STA, abs},   {STX, abs}},
		{{BCC, rel},  {STA, ind_index}, {},         {}, {STY, zpg_X}, {STA, zpg_X}, {STX, zpg_Y}, {}, {TYA, impl}, {STA, abs_Y}, {TXS, impl}, {}, {},           {STA, abs_X}, {}},
		{{LDY, imm},  {LDA, index_ind}, {LDX, imm}, {}, {LDY, zpg},   {LDA, zpg},   {LDX, zpg},   {}, {TAY, impl}, {LDA, imm},   {TAX, impl}, {}, {LDY, abs},   {LDA, abs},   {LDX, abs}},
		{{BCS, rel},  {LDA, ind_index}, {},         {}, {LDY, zpg_X}, {LDA, zpg_X}, {LDX, zpg_Y}, {}, {CLV, impl}, {LDA, abs_Y}, {TSX, impl}, {}, {LDY, abs_X}, {LDA, abs_X}, {LDX, abs_Y}},
		{{CPY, imm},  {CMP, index_ind}, {},         {}, {CPY, zpg},   {CMP, zpg},   {DEC, zpg},   {}, {INY, impl}, {CMP, imm},   {DEX, impl}, {}, {CPY, abs},   {CMP, abs},   {DEC, abs}},
		{{BNE, rel},  {CMP, ind_index}, {},         {}, {},           {CMP, zpg_X}, {DEC, zpg_X}, {}, {CLD, impl}, {CMP, abs_Y}, {},          {}, {},           {CMP, abs_X}, {DEC, abs_X}},
		{{CPX, imm},  {SBC, index_ind}, {},         {}, {CPX, zpg},   {SBC, zpg},   {INC, zpg},   {}, {INX, impl}, {SBC, imm},   {NOP, impl}, {}, {CPX, abs},   {SBC, abs},   {INC, abs}},
		{{BEQ, rel},  {SBC, ind_index}, {},         {}, {},           {SBC, zpg_X}, {INC, zpg_X}, {}, {SED, impl}, {SBC, abs_Y}, {},          {}, {},           {SBC, abs_X}, {INC, abs_X}}
	};

	byte RAM[0x10000];

	word getWord(word ptr)
	{
		return word(RAM[ptr + 1]) << 16 + RAM[ptr]; //LLHH, little-endian
	}

	word fetch(addressing_mode addr)
	{
		switch (addr)
		{
		case imm:
			return rPC++;
		case abs:
		{
			word a = getWord(rPC);
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
			return getWord(RAM[RAM[rPC++] + rX]);
		case ind_index:
			return getWord(RAM[RAM[rPC++]]) + rY;
		}

	}

	bool execute()
	{
		word opcode = RAM[rPC++];
		int lo = opcode & 15;
		int hi = (opcode >> 4) & 15;

		addressing_mode addr = INSTS[hi][lo].Addr;

		switch (INSTS[hi][lo].name)
		{
		case BRK:
		{
			return false;
		}
		case ADC:
		{
			int mem = RAM[fetch(addr)];

			int res = (int)rA + mem + (int)getFlag(fC);

			setFlag(fN, (res < 0));
			setFlag(fZ, (res == 0));
			setFlag(fC, (res >> 9) & 1);
			setFlag(fV, (res > 127 || res < -128));

			rA = byte(res & 0xFFFF);

			return true;
		}

		case AND:
		{
			rA = rA & RAM[fetch(addr)];

			setFlag(fN, (rA < 0));
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
			*op << 1;

			setFlag(fN, (*op < 0));
			setFlag(fZ, (*op == 0));

			return true;
		}

		case BCC:
		{
			if (!((rS >> fC) & 1))
				rPC += 1 + RAM[rPC];
			else
				rPC++;

			return true;
		}

		case BCS:
		{
			if (((rS >> fC) & 1))
				rPC += 1 + RAM[rPC];
			else
				rPC++;

			return true;
		}

		case LDA:
		{
			rA = RAM[fetch(addr)];

			setFlag(fN, (rA < 0));
			setFlag(fZ, (rA == 0));

			return true;
		}

		case NOP:
		{
			return true;
		}

		default:
			return false;
		}

		return true;
	}
};

int main()
{
	CPU_6502 cpu;

	cpu.start();
}