#pragma once
#include <string>
#include <iostream>
class deassembler
{
	using byte = unsigned char;
	using word = unsigned short;

	CPU_6502& _cpu;

public:
	deassembler(CPU_6502& cpuRef) : _cpu(cpuRef) {}

	const std::string inst_name_str[57] =
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
		"err" };

	std::string itohex(word u_num, bool doubleBytes, bool leadingZeroes)
	{
		std::string hex;

		bool foundNonZero = false;

		int i = 1;
		if (doubleBytes)
			i = 3;

		for (; i >= 0; i--)
		{
			if (i == 0)
				foundNonZero = true;

			int hexchar = (u_num >> 4 * i) & 0xF;

			if (hexchar < 10)
			{
				if (!(hexchar == 0 && !leadingZeroes && !foundNonZero))
				{
					if (hexchar != 0)
						foundNonZero = true;

					hex += hexchar + '0';
				}
			}
			else
			{
				hex += hexchar - 10 + 'a';
				foundNonZero = true;
			}
		}

		return hex;
	}

	std::string deassembleLine(int& ptr, bool& decimal, bool& leadingZeroes, bool& doubleBreak)
	{
		std::string line;

		int hi = _cpu.RAM[ptr] >> 4;
		int lo = _cpu.RAM[ptr] & 0xF;
		line += inst_name_str[_cpu.INSTS[hi][lo].name] + " ";

		CPU_6502::addressing_mode am = _cpu.INSTS[hi][lo].Addr;

		ptr++;

		switch (am)
		{
		case CPU_6502::acc:
			line += "A\n";
			break;
		case CPU_6502::impl:
			line += "\n";
			break;
		case CPU_6502::rel:
		{
			int num = ptr + *(signed char*)&_cpu.RAM[ptr] + 1;

			if (decimal)
			{
				if (num == 0)
					line += "0";
				else
				{
					if (num < 0)
					{
						num = -num;
						line += "-";
					}

					std::string snum;

					while (num != 0)
					{
						std::string dig = std::to_string(num % 10);
						snum = dig + snum;

						num /= 10;
					}

					line += snum;
				}
			}
			else
			{
				line += "$";

				line += itohex(num, true, leadingZeroes);
			}
			line += "\n";
			ptr++;
			break;
		}
		case CPU_6502::imm:
		case CPU_6502::abs:
		case CPU_6502::abs_X:
		case CPU_6502::abs_Y:
		case CPU_6502::zpg:
		case CPU_6502::zpg_X:
		case CPU_6502::zpg_Y:
		case CPU_6502::ind:
		case CPU_6502::ind_index:
		case CPU_6502::index_ind:
		{
			unsigned short u_num = (unsigned short)_cpu.RAM[ptr];

			bool doubLen = false;

			if (am == CPU_6502::abs || am == CPU_6502::abs_X || am == CPU_6502::abs_Y || am == CPU_6502::ind)
			{
				u_num += (unsigned short)_cpu.RAM[ptr + 1] << 8;
				doubLen = true;
				ptr += 2;
			}
			else
				ptr++;

			if (am == CPU_6502::imm)
				line += "#";
			else if (am == CPU_6502::ind || am == CPU_6502::ind_index || am == CPU_6502::index_ind)
			{
				line += "(";
			}

			if (decimal)
			{
				signed short num = *(signed short*)(&u_num);

				if (num == 0)
					line += "0";
				else
				{
					if (num < 0)
					{
						num = -num;
						line += "-";
					}

					std::string snum;

					while (num != 0)
					{
						std::string dig = std::to_string(num % 10);
						snum = dig + snum;

						num /= 10;
					}

					line += snum;
				}
			}
			else
			{
				line += "$";

				line += itohex(u_num, doubLen, leadingZeroes);
			}
			switch (am)
			{
			case CPU_6502::ind:
				line += ")";
				break;
			case CPU_6502::ind_index:
				line += "), Y";
				break;
			case CPU_6502::index_ind:
				line += ", X)";
				break;
			case CPU_6502::abs_X:
			case CPU_6502::zpg_X:
				line += ", X";
				break;
			case CPU_6502::abs_Y:
			case CPU_6502::zpg_Y:
				line += ", Y";
				break;
			}

			line += "\n";
		}
		}

		if (doubleBreak)
			line += "\n";

		return line;
	}

	std::string deassemble(int endPtr, bool decimal = false, bool leadingZeroes = false, bool doubleBreak = false)
	{
		std::string output;

		int ptr = 0;

		while (ptr != endPtr)
		{
			output += "$" + itohex(ptr, true, true) + "  ";

			int nBytes;
			int lo = _cpu.RAM[ptr] & 15;
			int hi = (_cpu.RAM[ptr] >> 4) & 15;
			switch (_cpu.INSTS[hi][lo].Addr)
			{
			case CPU_6502::impl:
			case CPU_6502::acc:
				nBytes = 1;
				break;

			case CPU_6502::abs:
			case CPU_6502::abs_X:
			case CPU_6502::abs_Y:
			case CPU_6502::ind:
				nBytes = 3;
				break;

			default:
				nBytes = 2;
			}

			for (int i = 0; i < nBytes; i++)
			{
				output += itohex(_cpu.RAM[ptr + i], false, true) + " ";
			}
			for (int i = nBytes; i < 3; i++)
			{
				output += "   ";
			}

			output += " " + deassembleLine(ptr, decimal, leadingZeroes, doubleBreak);
		}
		return output;
	}
};

