#include "CPU_6502.h"
#include "assembler.h"
using namespace std;

int main()
{
	CPU_6502 cpu;
	assembler assmer(cpu);

	string text =
		"define ADDR $0A\n"
		"LDA #$42\n"
		"STA ADDR\n"
		"LDA #$20\n"
		"SEC\n"
		"ADC ADDR\n";

	cpu.init();
	assmer.assemble(text);
	cpu.start();

}