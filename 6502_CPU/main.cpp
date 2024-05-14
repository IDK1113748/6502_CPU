#include "CPU_6502.h"
#include "assembler.h"
#include <fstream>
using namespace std;

bool getProgram(string& text)
{
	text.clear();
	string program_name;
	ifstream programFile;
	while (!programFile.is_open())
	{
		cout << "Enter program name or exit: ";
		cin >> program_name;
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

int main()
{
	CPU_6502 cpu;
	assembler assmer(cpu);

	string text;
	while (getProgram(text))
	{
		cpu.init();
		assmer.assemble(text);
		//cpu.start();
	}
}