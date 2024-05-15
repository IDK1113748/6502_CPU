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

	std::string deassemble(int endPtr, bool decimal = false, bool leadingZeros = false)
	{
		std::string assembly;

		int ptr = 0;

		while (ptr != endPtr)
		{
			int hi = _cpu.RAM[ptr] >> 4;
			int lo = _cpu.RAM[ptr] & 0xF;
			assembly += inst_name_str[_cpu.INSTS[hi][lo].name] + " ";

			CPU_6502::addressing_mode am = _cpu.INSTS[hi][lo].Addr;

			ptr++;
			
			switch (am)
			{
			case CPU_6502::acc:
				assembly += "A\n";
				break;
			case CPU_6502::impl:
				assembly += "\n";
				break;
			case CPU_6502::rel:
			{
				int num = ptr + *(signed char* )&_cpu.RAM[ptr];

				std::cout << "num" << num << "\n";
				if (decimal)
				{
					if (num == 0)
						assembly += "0";
					else
					{
						if (num < 0)
							assembly += "-";

						std::string snum;

						while (num != 0)
						{
							std::string dig = std::to_string(num % 10);
							snum = dig + snum;

							num /= 10;
						}

						assembly += snum;
					}
				}
				else
				{
					assembly += "$";

					bool foundNonZero = false;

					for (int i = 3; i >= 0; i--)
					{
						int hexchar = (num >> 4 * i) & 0xF;
						std::cout << "hexchar " << hexchar << " ";

						if (hexchar < 10)
						{
							if (!(hexchar == 0 && !leadingZeros && !foundNonZero))
							{
								if (hexchar != 0)
									foundNonZero = true;

								assembly += hexchar + '0';
								std::cout << " a1 " << hexchar + '0' << "   ";
							}
						}
						else
						{
							assembly += hexchar - 10 + 'a';
							std::cout << " a2 " << hexchar - 10 + 'a' << "   ";
							foundNonZero = true;
						}
					}
				}
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
					assembly += "#";
				else if (am == CPU_6502::ind || am == CPU_6502::ind_index || am == CPU_6502::index_ind)
				{
					assembly += "(";
				}

				if (decimal)
				{
					signed short num = *(signed short*)(&u_num);

					if (num == 0)
						assembly += "0";
					else
					{
						if (num < 0)
							assembly += "-";

						std::string snum;

						while (num != 0)
						{
							std::string dig = std::to_string(num % 10);
							snum = dig + snum;

							num /= 10;
						}

						assembly += snum;
					}
				}
				else
				{
					std::cout << "x ";
					assembly += "$";

					bool foundNonZero = false;

					int i = 1;
					if (doubLen)
						i = 3;

					for (; i >= 0; i--)
					{
						int hexchar = (u_num >> 4 * i) & 0xF;

						if (hexchar < 10)
						{
							//std::cout << "h";
							if (!(hexchar == 0 && !leadingZeros && !foundNonZero))
							{
								if (hexchar != 0)
									foundNonZero = true;

								assembly += hexchar + '0';
								//std::cout << (int)hexchar << "\n";
							}
						}
						else
						{
							assembly += hexchar - 10 + 'a';
							//std::cout << (int)hexchar << "\n";
							foundNonZero = true;
						}
					}
				}
				switch (am)
				{
				case CPU_6502::ind:
					assembly += ")";
					break;
				case CPU_6502::ind_index:
					assembly += "), Y";
					break;
				case CPU_6502::index_ind:
					assembly += ", X)";
					break;
				case CPU_6502::abs_X:
				case CPU_6502::zpg_X:
					assembly += ", X";
					break;
				case CPU_6502::abs_Y:
				case CPU_6502::zpg_Y:
					assembly += ", Y";
					break;
				}

				assembly += "\n";
			}
			}
		}

		return assembly;
	}
};

