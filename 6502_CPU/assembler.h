#pragma once

#include <iostream>
#include <string>
#include <vector>

//#define DEBUG_ASM

class assembler
{
	using byte = unsigned char;
	using word = unsigned short;

	CPU_6502& _cpu;

public:
	assembler(CPU_6502& cpuRef) : _cpu(cpuRef) {}

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

	int ptr = 0x200;

	/*RULES:
	definitions must be declared before any occurence
	a label cannot be defined after a line of code (e.g. "STX #$42 blabla:")
	*/

	bool alpha(char c)
	{
		return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
	}

	bool num(char c)
	{
		return (c >= '0' && c <= '9');
	}

	bool alphanum(char c)
	{
		return (alpha(c) || num(c));
	}

	bool whitespace(char c)
	{
		return (c == ' ' || c == '\t');
	}

	void preprocess(std::string& assembly)
	{
		if (assembly[assembly.length() - 1] != '\n')
			assembly += '\n';

		int ch = 0;

		std::string defined;
		std::string replacer;

		while (ch < assembly.length())
		{
			while (whitespace(assembly[ch]))
				ch++;

			if (assembly[ch] == 'd' && ch + 7 < assembly.length())
			{
				if (assembly.substr(++ch, 6) == "efine ")
				{
					ch += 6;
					while (whitespace(assembly[ch]))
						ch++;
					std::size_t foundEnd = assembly.find(' ', ch);
					defined = assembly.substr(ch, foundEnd - ch);

					ch = foundEnd;

					while (whitespace(assembly[ch]))
						ch++;

					replacer.clear();
					while (assembly[ch] != ' ' && assembly[ch] != ';' && assembly[ch] != '\n')
					{
						replacer += assembly[ch];
						ch++;
					}

#ifdef DEBUG_ASM
					std::cout << "|" << defined << "|"
						<< "  "
						<< "|" << replacer << "|\n";
#endif

					std::size_t occurrence;
					occurrence = assembly.find(defined, ch);

					while (occurrence != std::string::npos)
					{
						assembly.replace(occurrence, defined.length(), replacer);

						occurrence = assembly.find(defined, occurrence + replacer.size());
					}
				}
			}

			while (assembly[ch] != '\n')
				ch++;
			ch++;
		}

#ifdef DEBUG_ASM
		std::cout << assembly << "\n";
#endif
	}

	struct labelStruct
	{
		std::string labelName;
		int val;
	};

	std::vector<labelStruct> labels;

	int stoi(std::string s)
	{
		unsigned int n = 0;

		for (int i = 0; i < s.length(); i++)
		{
			n = 10 * n + s[i] - '0';
		}

		return n;
	}

	void collectLabels(std::string& assembly)
	{
		labels.clear();
		int ch = 0;
		int loc = 0x200;

		while (ch < assembly.length())
		{
			while (whitespace(assembly[ch]))
				ch++;

			bool isDefinition = false;
			if (assembly[ch] == 'd' && ch + 7 < assembly.length())
			{
				if (assembly.substr(ch+1, 6) == "efine ")
				{
					isDefinition = true;
				}
			}
			if (isDefinition)
			{
				while (assembly[ch] != '\n')
					ch++;
				ch++;
				continue;
			}

			bool isLabel = false;
			int l;
			for (l = ch; assembly[l] != '\n' && assembly[l] != ';' && !isLabel; l++)
				if (assembly[l] == ':')
					isLabel = true;

			if (assembly[l] == ';')
			{
				while (assembly[ch] != '\n')
					ch++;
				ch++;
				continue;
			}

			l--;
			if (isLabel)
			{
				labels.push_back({ assembly.substr(ch, l - ch), loc });
				ch = l + 1;
			}

			while (whitespace(assembly[ch]))
				ch++;

			if (assembly[ch] != '\n')
			{
				std::string opcode = assembly.substr(ch, 3);

				for (int i = 0; i < 3; i++)
				{
					if (opcode[i] >= 'a' && opcode[i] <= 'z')
						opcode[i] -= 32;
				}
				bool branch = (opcode[0] == 'B' && !(opcode == "BIT" && opcode == "BRK"));

				if (assembly[ch] == ';')
				{
					while (assembly[ch] != '\n')
						ch++;
					ch++;
					continue;
				}
				else
				{
					loc++;

					ch += 3;
					while (whitespace(assembly[ch]))
						ch++;

					bool jump = (opcode == "JMP" || opcode == "JSR");

					bool Yind = false;
					for (int i = ch; assembly[i] != ';' && assembly[i] != '\n' && !Yind; i++)
					{
						if(assembly[i] == ',')
							for (int j = i+1; assembly[j] != ';' && assembly[j] != '\n' && !Yind; j++)
							{
								if (assembly[j] == 'y' || assembly[j] == 'Y')
									Yind = true;
							}
					}
					bool mustBeDouble = (jump || (Yind && !(opcode == "LDX" || opcode == "STX")));

					if (opcode[0] == 'B' && opcode != "BRK" && opcode != "BIT")
						loc++;
					else
						switch (assembly[ch])
						{
						case '#':
							loc++;
							break;
						case '(':
						{
							if (opcode == "JMP")
								loc += 2;
							else
								loc++;
							break;
						}
						case '$':
						{
							int len = 0;

							for (int i = 0; alphanum(assembly[ch + 1 + i]); i++)
							{
								if (len > 0)
									len++;

								if (assembly[ch + 1 + i] != '0')
								{
									if (len == 0)
										len++;
								}

							}

							if (len <= 2 && !mustBeDouble)
								loc++;
							else
								loc += 2;
							break;
						}
						case '%':
						{
							int len = 0;

							for (int i = 0; assembly[ch + 1 + i] == '0' || assembly[ch + 1 + i] == '1'; i++)
							{
								if (len > 0)
									len++;

								if (assembly[ch + 1 + i] == '1')
								{
									if (len == 0)
										len++;
								}

							}

							if (len <= 8 && !mustBeDouble)
								loc++;
							else
								loc += 2;
							break;
						}
						default:
							if (alpha(assembly[ch]))
							{
								if (!(assembly[ch] == 'A' && (assembly[ch + 1] == '\n' || whitespace(assembly[ch + 1]) || assembly[ch + 1] == ';')))
								{
									if (branch)
										loc++;
									else
										loc += 2;
								}
							}
							else if (num(assembly[ch]) || (assembly[ch] == '-' && num(assembly[ch + 1])))
							{
								bool neg = (assembly[ch] == '-');
								if (neg)
									ch++;
								
								int len = 0;
								for (int i = 0; num(assembly[ch + i]); i++)
									len++;
								int n = stoi(assembly.substr(ch, len));

								if (!mustBeDouble && ((n <= 128 && neg) || (n < 256 && !neg)))
								{
									loc++;
								}
								else
								{
									loc += 2;
								}
							}
						}

					while (assembly[ch] != '\n')
						ch++;
					ch++;
				}
			}
			else
				ch++;
		}

#ifdef DEBUG_ASM
		for (const auto& l : labels)
			std::cout << l.labelName << " " << l.val << "\n";
#endif
	}

	int hextoi(char c)
	{
		if (num(c))
			return c - '0';
		else if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;

		return 0;
	}

	unsigned int stohex(std::string s)
	{
		unsigned int n = 0;

		for (int i = 0; i < s.length(); i++)
		{
			n = 16 * n + hextoi(s[i]);
		}

		return n;
	}

	unsigned int stobin(std::string s)
	{
		unsigned int n = 0;

		for (int i = 0; i < s.length(); i++)
		{
			n = n << 1;
			n += s[i] - '0';
		}

		return n;
	}

	void hexdump(int end)
	{
		for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				int index = i * 8 + j;
				if (index < end)
					printf("%02x  ", _cpu.RAM[0x200 + index]);
				else
				{
					printf("\n\n");
					return;
				}
			}
			printf("\n");
		}
	}

	int assemble(std::string& assembly)
	{
		preprocess(assembly);
		collectLabels(assembly);

		int ch = 0;
		int ptr = 0x200;

		while (ch < assembly.length())
		{
			while (whitespace(assembly[ch]))
				ch++;

			bool isDefinition = false;
			if (assembly[ch] == 'd' && ch + 7 < assembly.length())
			{
				if (assembly.substr(ch+1, 6) == "efine ")
				{
					isDefinition = true;
				}
			}
			if (isDefinition)
			{
				while (assembly[ch] != '\n')
					ch++;
				ch++;
				continue;
			}

			bool isLabel = false;
			int l;
			for (l = ch; assembly[l] != '\n' && assembly[l] != ';' && !isLabel; l++)
				if (assembly[l] == ':')
					isLabel = true;
			l--;
			if (isLabel)
			{
				ch = l + 1;
			}

			while (whitespace(assembly[ch]))
				ch++;

			if (assembly[ch] != '\n')
			{

				if (assembly[ch] == ';')
				{
					while (assembly[ch] != '\n')
						ch++;
					ch++;
					continue;
				}
				else
				{
					std::string opcode = assembly.substr(ch, 3);

					for (int i = 0; i < 3; i++)
					{
						if (opcode[i] >= 'a' && opcode[i] <= 'z')
							opcode[i] -= 32;
					}

					CPU_6502::inst_name enum_name = CPU_6502::ILL;
					for (int i = 0; i < 56; i++)
					{
						if (opcode == inst_name_str[i])
							enum_name = (CPU_6502::inst_name)i;
					}

					CPU_6502::addressing_mode am = CPU_6502::ill;

					ch += 3;
					while (whitespace(assembly[ch]))
						ch++;

					byte arg[2];

					int nArgs = 0;

					bool indirect = false;
					bool Xind = false;
					bool Yind = false;

					if (assembly[ch] == '(')
					{
						indirect = true;

						if (enum_name == CPU_6502::JMP)
						{
							am = CPU_6502::ind;
						}
						else
						{
							for (int j = ch; assembly[j] != '\n' && assembly[j] != ';' && !Yind; j++)
								if (assembly[j] == ',')
								{
									for (int i = j; assembly[i] != '\n' && assembly[i] != ';' && !Yind; i++)
										if (assembly[i] == 'y' || assembly[i] == 'Y')
											Yind = true;
								}

							if (Yind)
								am = CPU_6502::ind_index;
							else
							{
								Xind = true;
								am = CPU_6502::index_ind;
							}
						}
						ch++;
					}
					else if (opcode[0] == 'B' && opcode != "BIT" && opcode != "BRK")
					{
						am = CPU_6502::rel;
					}

					bool jump = (opcode == "JMP" || opcode == "JSR");

					if (!indirect)
					{
						for (int j = ch; assembly[j] != '\n' && assembly[j] != ';' && !Yind && !Xind; j++)
							if (assembly[j] == ',')
							{
								for (int i = j; assembly[i] != '\n' && assembly[i] != ';' && !Yind && !Xind; i++)
									switch (assembly[i])
									{
									case 'y':
									case 'Y':
										Yind = true;
										break;
									case 'x':
									case 'X':
										Xind = true;
										break;
									}
							}
					}

					if (assembly[ch] == '\n' || assembly[ch] == ';')
					{
						am = CPU_6502::impl;
					}
					else if (assembly[ch] == '#' || assembly[ch] == '$' || assembly[ch] == '%' || num(assembly[ch]) || assembly[ch] == '-')
					{
						if (!indirect)
						{
							if (assembly[ch] == '#')
							{
								am = CPU_6502::imm;
								ch++;
							}
						}

						switch (assembly[ch])
						{
						case '%':
						{
							int len = 0;
							int i;

							for (i = 0; assembly[ch + 1 + i] == '0' || assembly[ch + 1 + i] == '1'; i++)
							{
								if (len > 0)
									len++;

								if (assembly[ch + 1 + i] == '1')
								{
									if (len == 0)
										len++;
								}

							}

							unsigned int num = stobin(assembly.substr(ch + 1 + i - len, len));

							if (jump || len > 8)
							{
								nArgs = 2;
								arg[0] = num & 0xFF;
								arg[1] = (num >> 8) & 0xFF;

								if (am != CPU_6502::ind)
								{
									if (Xind)
										am = CPU_6502::abs_X;
									else if (Yind)
										am = CPU_6502::abs_Y;
									else
										am = CPU_6502::abs;
								}
							}
							else
							{
								nArgs = 1;
								arg[0] = num & 0xFF;

								if (am != CPU_6502::imm && !indirect)
								{
									if (Xind)
										am = CPU_6502::zpg_X;
									else if (Yind)
										am = CPU_6502::zpg_Y;
									else
										am = CPU_6502::zpg;
								}
							}
							break;
						}
						//case '0':
							//	break; //octal
						case '$':
							if (am == CPU_6502::rel)
							{
								//rewrite this garbage (although it works)
								nArgs = 1;
								int n;
								if (alphanum(assembly[ch + 3]) && alphanum(assembly[ch + 3]))
									n = hextoi(assembly[ch + 1]) * 16 * 16 * 16 + hextoi(assembly[ch + 2]) * 16 * 16 + hextoi(assembly[ch + 3]) * 16 + hextoi(assembly[ch + 4]);
								else n = hextoi(assembly[ch + 1]) * 16 + hextoi(assembly[ch + 2]);

								signed char offset = (signed short)n - (signed short)ptr - 2;
								arg[0] = *(byte*)(&offset);
							}
							else
							{
								int len = 0;
								int i;

								for (i = 0; alphanum(assembly[ch + 1 + i]); i++)
								{
									if (len > 0)
										len++;

									if (assembly[ch + 1 + i] != '0')
									{
										if (len == 0)
											len++;
									}
								}

								int num = stohex(assembly.substr(ch + 1 + i - len, len));

								if (jump || len > 2)
								{
									nArgs = 2;
									arg[0] = num & 0xFF;
									arg[1] = (num >> 8) & 0xFF;

									if (am != CPU_6502::ind)
									{
										if (Xind)
											am = CPU_6502::abs_X;
										else if (Yind)
											am = CPU_6502::abs_Y;
										else
											am = CPU_6502::abs;
									}
								}
								else
								{
									nArgs = 1;
									arg[0] = num & 0xFF;

									if (am != CPU_6502::imm && !indirect)
									{
										if (Xind)
											am = CPU_6502::zpg_X;
										else if (Yind)
											am = CPU_6502::zpg_Y;
										else
											am = CPU_6502::zpg;
									}
								}

								break;
							}
						default:
						{
							
							if (num(assembly[ch]) || (assembly[ch] == '-' && num(assembly[ch + 1])))
							{
								bool neg = (assembly[ch] == '-');
								if (neg)
								{
									ch++;
								}

								int len = 0;
								for (int i = 0; num(assembly[ch + i]); i++)
									len++;

								int n = stoi(assembly.substr(ch, len));

								if (!jump && ((n <= 128 && neg) || (n < 256 && !neg)))
								{
									nArgs = 1;
									unsigned char ubyte;

									if (neg)
									{
										signed char sbyte = -n;
										ubyte = *(unsigned char*)(&sbyte);
									}
									else
										ubyte = n;

									if (am != CPU_6502::imm && !indirect)
									{
										if (Xind)
											am = CPU_6502::zpg_X;
										else if (Yind)
											am = CPU_6502::zpg_Y;
										else
											am = CPU_6502::zpg;
									}

									arg[0] = ubyte;
								}
								else
								{
									nArgs = 2;
									unsigned short uword;
									if (neg)
									{
										signed short sword = -n;
										uword = *(unsigned short*)(&sword);
									}
									else
									{
										uword = n;
									}

									if (am != CPU_6502::ind)
									{
										if (Xind)
											am = CPU_6502::abs_X;
										else if (Yind)
											am = CPU_6502::abs_Y;
										else
											am = CPU_6502::abs;
									}

									arg[0] = uword & 0xFF;
									arg[1] = (uword >> 8) & 0xFF;
								}
							}
							break;
						}
						}
					}
					else if (alpha(assembly[ch]))
					{
						if (assembly[ch] == 'A' && !(alphanum(assembly[ch + 1]) || assembly[ch + 1] == '_'))
						{
							am = CPU_6502::acc;
						}
						else
						{
							std::string name;
							for (int i = ch; alphanum(assembly[i]) || assembly[i] == '_'; i++)
								name += assembly[i];

							bool foundLabel = false;
							for (const auto& l : labels)
							{
								if (name == l.labelName)
								{
									if (am == CPU_6502::rel)
									{
										nArgs = 1;

										signed char offset = (signed short)l.val - (signed short)ptr - 2;
										arg[0] = *(byte*)(&offset);
									}
									else
									{
										if (!indirect)
										{
											if (Xind)
												am = CPU_6502::abs_X;
											else if (Yind)
												am = CPU_6502::abs_Y;
											else
												am = CPU_6502::abs;
										}

										nArgs = 2; //Note: data labels cannot be used with the zero page (for example "ADC zpgaddr")
										//the absolute opcode will be used wherever there's a zero-page (with the exception of the indirect zpg, which will not assemble)

										arg[0] = l.val & 0xFF;
										arg[1] = l.val >> 8;
									}

									foundLabel = true;
									break;
								}
							}

							if (!foundLabel)
								std::cout << "WARNING: Label \"" << name << "\" not found.\n";
						}
					}

					if (am == CPU_6502::zpg_Y && enum_name != CPU_6502::STX && enum_name != CPU_6502::LDX)
					{
						nArgs = 2;
						arg[1] = 0;
						am = CPU_6502::abs_Y;
					}

					for (int i = 0; i < 16; i++)
						for (int j = 0; j < 15; j++)
						{
							if (_cpu.INSTS[i][j].name == enum_name && _cpu.INSTS[i][j].Addr == am)
							{
								_cpu.RAM[ptr++] = (i << 4) + j;

								for (int a = 0; a < nArgs; a++)
									_cpu.RAM[ptr++] = arg[a];

								goto ExitLoops;
							}
						}
				ExitLoops:

					while (assembly[ch] != '\n')
						ch++;
					ch++;
				}
			}
			else
				ch++;
		}
		_cpu.RAM[ptr] = 0x00;

		return ptr - 0x200;
	}
};