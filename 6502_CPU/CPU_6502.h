#ifndef CPU_H
#define CPU_H

class CPU_6502
{
	using byte = unsigned char;
	using word = unsigned short;

public:
	CPU_6502();
	~CPU_6502();

	word rPC;
	byte rSP;
	byte rA;
	byte rX;
	byte rY;
	byte rS;

	void init();

	void start();

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
		ind_index,
		ill
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

	bool getFlag(status_flag sf);

	void setFlag(status_flag sf, bool val);

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
		TYA,
		ILL
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

	byte* RAM;

	word getWord(word ptr);

	unsigned int subtract(byte op1, byte op2);

	bool neg(unsigned int num);

	word fetch(addressing_mode addr, bool incPC = true);

	bool execute(bool* VRAM_W = nullptr);
};

#endif //CPU_H
