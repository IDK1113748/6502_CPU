#include "CPU_6502.h"
#include "assembler.h"
#include "disassembler.h"
#include "dmonitor.h"
#include "monitor.h"
#include <fstream>
using namespace std;

enum flag_types {
	fdecimal,
	fleadingZeroes,
	fdontRun,
	fdump,
	fdisassemble,
	fshowsource,
	fdebugmon,
	fmon
};
int flags;

bool getProgram(string& text)
{
	text.clear();
	string input;
	ifstream programFile;
	while (!programFile.is_open())
	{
		cout << "Enter program name or exit: ";
		getline(cin, input);

		std::size_t start = input.find_first_not_of(" \t");
		if (start == string::npos)
			return false;

		std::size_t endOfName = input.find_first_of(" \t-", start);
		if (endOfName == string::npos)
			endOfName = input.size();
		std::size_t flagStart = input.find('-');

		flags = 0;
		while (flagStart != string::npos)
		{
			flagStart++;
			std::size_t flagEnd = input.find_first_of(" \t-", flagStart);
			if (flagEnd == string::npos)
				flagEnd = input.size();

			const std::string flag_name = input.substr(flagStart, flagEnd - flagStart);
			switch (flag_name[0])
			{
			case 'd':
				if (flag_name == "de")
					flags |= (1 << fdisassemble);
				else if (flag_name == "dec")
					flags |= (1 << fdecimal);
				else if (flag_name == "dump")
					flags |= (1 << fdump);
				else if (flag_name == "dmon")
					flags |= (1 << fdebugmon);
				break;
			case 'z':
				flags |= (1 << fleadingZeroes);
				break;
			case 'x':
				flags |= (1 << fdontRun);
				break;
			case 's':
				flags |= (1 << fshowsource);
				break;
			case 'm':
				flags |= (1 << fmon);
			}

			flagStart = input.find('-', flagStart);
		}

		string program_name = input.substr(start, endOfName-start);
		if (program_name == "exit" || program_name == "Exit")
			return false;
		programFile.open("programs/" + program_name + ".txt");
	}

	string curLine;
	while (getline(programFile, curLine))
		text += curLine += "\n";
	programFile.close();

	return true;
}

bool flag(flag_types flag_type)
{
	return flags & (1 << flag_type);
}

int main()
{
	CPU_6502 cpu;
	assembler assmer(cpu);
	disassembler disassmer(cpu, assmer);

	string text;
	string disassembly;
	while (getProgram(text))
	{
		cpu.init();

		if (flag(fshowsource))
			cout << text << "\n";

		int size = assmer.assemble(text);

		if (flag(fdisassemble))
		{
			disassembly = disassmer.disassemble(flag(fdecimal), flag(fleadingZeroes));
			cout << disassembly + "\n";
		}
		
		if(flag(fdump))
			assmer.hexdump(size);

		if (!flag(fdontRun))
		{
			if (flag(fdebugmon))
			{
				disassembly = disassmer.disassemble(flag(fdecimal), flag(fleadingZeroes), true);

				dmonitor mon(cpu, disassmer, disassembly);
				if (mon.Construct(1280, 640, 1, 1))
					mon.Start();
			}
			else if (flag(fmon))
			{
				monitor mon(cpu);
				if (mon.Construct(272, 272, 2, 2))
					mon.Start();
			}
			else
				cpu.start();
		}
	}
}