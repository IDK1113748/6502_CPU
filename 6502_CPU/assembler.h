#pragma once

#include <iostream>
#include <string>
#include <vector>

#define DEBUG_ASM

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

	int ptr = 0;

	/*RULES:
	definitions must be declared before any occurence
	a label cannot be defined after a line of code (e.g. "STX #$42 blabla:")
	*/

	//std::string assembly =
		//"Man:\n"
		//"   define ohn ock\n"
		//"LDA #$32\n"
		//"define DA_ADres $58\n"
		//"  STA $4521  ; Idk dude\n"
		//"JMP Man\n"
		//"JMP (DA_ADres) \n"
		//"JSR Del\n"
		//"Cohn:  \n"
		//"LDA %0000101\n"
		//"ROL A\n"
		//"Del: LDA #$42\n"
		//"SBC ($1256), Y\n"
		//"ROL A\n"
		//"END:";

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

	void collectLabels(std::string& assembly)
	{
		labels.clear();
		int ch = 0;
		int loc = 0;

		while (ch < assembly.length())
		{
			while (whitespace(assembly[ch]))
				ch++;

			bool isDefinition = false;
			if (assembly[ch] == 'd' && ch + 7 < assembly.length())
			{
				if (assembly.substr(++ch, 6) == "efine ")
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
			for (l = ch; assembly[l] != '\n' && !isLabel; l++)
				if (assembly[l] == ':')
					isLabel = true;

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
						for (int i = 0; i < 3; i++)
						{
							if (opcode[i] >= 'a' && opcode[i] <= 'z')
								opcode[i] -= 32;
						}
						if (opcode == "JMP")
							loc += 2;
						else
							loc++;
						break;
					}
					case '$':
					{
						if (assembly[ch + 1] == '0' && assembly[ch + 2] == '0')
							loc++;
						else if (alphanum(assembly[ch + 3]))
							loc += 2;
						else
							loc++;
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

						if (len <= 8)
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

	int ctohex(char c)
	{
		if (num(c))
			return c - '0';
		else if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;

		return 0;
	}

	unsigned int stobin(std::string s)
	{
		unsigned int n = 0;

		for (int i = 0; i < s.length(); i++)
		{
			//std::cout << (int)n << "\n";
			n = n << 1;
			n += s[i] - '0';
		}

		return n;
	}

	void assemble(std::string& assembly)
	{
		preprocess(assembly);
		collectLabels(assembly);

		int ch = 0;
		int ptr = 0;

		while (ch < assembly.length())
		{
			while (whitespace(assembly[ch]))
				ch++;

			bool isDefinition = false;
			if (assembly[ch] == 'd' && ch + 7 < assembly.length())
			{
				if (assembly.substr(++ch, 6) == "efine ")
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
			for (l = ch; assembly[l] != '\n' && !isLabel; l++)
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

					if (!indirect)
					{
						for (int j = ch; assembly[j] != '\n' && assembly[j] != ';' && !Yind && !Xind; j++)
							if (assembly[l] == ',')
							{
								for (int i = j; assembly[j] != '\n' && assembly[j] != ';' && !Yind && !Xind; i++)
									switch (assembly[ch])
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
					else if (assembly[ch] == '#' || assembly[ch] == '$' || assembly[ch] == '%')
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

							unsigned int num = stobin(assembly.substr(ch + 1, i));

							if (indirect || (am != CPU_6502::imm && len > 8/*&& i > 7 && !isZpg*/))
							{
								nArgs = 2;
								if (indirect && i <= 7)
								{
									arg[0] = num;
									arg[1] = 0;
								}
								else
								{
									arg[0] = num & 0xFF;
									arg[1] = (num >> 8) & 0xFF;
								}

								if (!indirect)
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
								arg[0] = num;

								if (am != CPU_6502::imm)
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
								nArgs = 1;
								int n;
								if (alphanum(assembly[ch + 3]) && alphanum(assembly[ch + 3]))
									n = ctohex(assembly[ch + 1]) * 16 * 16 * 16 + ctohex(assembly[ch + 2]) * 16 * 16 + ctohex(assembly[ch + 3]) * 16 + ctohex(assembly[ch + 4]);
								else n = ctohex(assembly[ch + 1]) * 16 + ctohex(assembly[ch + 2]);

								signed char offset = (signed short)n - (signed short)ptr - 2;
								arg[0] = *(byte*)(&offset);
							}
							else if (indirect || !(am == CPU_6502::imm || !alphanum(assembly[ch + 3]) || (alphanum(assembly[ch + 3]) && assembly[ch + 1] == '0' && assembly[ch + 2] == '0')))
							{
								nArgs = 2;
								if (indirect && !alphanum(assembly[ch + 3]))
								{
									arg[0] = ctohex(assembly[ch + 1]) * 16 + ctohex(assembly[ch + 2]);
									arg[1] = 0;
								}
								else
								{
									arg[0] = ctohex(assembly[ch + 3]) * 16 + ctohex(assembly[ch + 4]);
									arg[1] = ctohex(assembly[ch + 1]) * 16 + ctohex(assembly[ch + 2]);
								}

								if (!indirect)
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
								if (alphanum(assembly[ch + 3]) && assembly[ch + 1] == '0' && assembly[ch + 2] == '0')
									ch += 2;
								nArgs = 1;
								arg[0] = ctohex(assembly[ch + 1]) * 16 + ctohex(assembly[ch + 2]);

								if (am != CPU_6502::imm)
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
							//default:
							//decimal (num())
						}
					}
					else if (alpha(assembly[ch]))
					{
						if (assembly[ch] == 'A' && !alphanum(assembly[ch + 1]))
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

										nArgs = 2;
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

					for (int i = 0; i < 16; i++)
						for (int j = 0; j < 15; j++)
						{
							if (_cpu.INSTS[i][j].name == enum_name && _cpu.INSTS[i][j].Addr == am)
							{
								std::cout << "ptr: " << ptr << "\n";
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

#ifdef DEBUG_ASM
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				int index = i * 8 + j;
				if (index < ptr)
					printf("%02x  ", _cpu.RAM[index]);
				else
				{
					printf("\n\n");
					return;
				}
			}
			printf("\n");
		}
#endif
	}
};

/*int main()
{
	//string assembly = "Start\n";
	//
	//	string line;
	//	do{
	//		getline(cin, line);
	//		assembly += line + "\n";
	//	}while(line != "Exit");
	//
	//	cout << assembly;

	assembler asmer;
	asmer.preprocess();
	asmer.collectLabels();

	asmer.assemble();

	std::cout << "\n";
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
			printf("%02x  ", asmer.RAM[i * 10 + j]);
		printf("\n");
	}
}*/