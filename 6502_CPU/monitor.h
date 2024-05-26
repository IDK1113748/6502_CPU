#pragma once
#include "olcPixelGameEngine.h"
#include "CPU_6502.h"

#include <chrono>
#include <thread>

class monitor : public olc::PixelGameEngine
{
public:
	monitor(CPU_6502& cpuRef) : _cpu(cpuRef)
	{
		sAppName = "6502 monitor";
	}

private:
	CPU_6502& _cpu;

	bool run = true;
	bool restart = false;

	bool OnUserCreate() override
	{
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::Key::SPACE).bPressed)
		{
			_cpu.init();
			run = true;
			restart = true;
		}


		bool VRAM_W;
		if (run && _cpu.execute(&VRAM_W))
		{
			if (VRAM_W || restart)
			{
				Clear(olc::BLACK);

				std::string monitorText;
				for (int i = 0; i < 32; i++)
				{
					for (int j = 0; j < 32; j++)
					{
						char c = _cpu.RAM[0x8000 + i * 32 + j];

						if (c >= 32 && c <= 127)
							monitorText += c;
						else
							monitorText += '\0';
					}
					monitorText += "\n";
				}

				DrawString({ 8,8 }, monitorText);
			}
		}
		else
			run = false;
		return true;
	}
};

