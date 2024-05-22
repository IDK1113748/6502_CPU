#pragma once
#include "olcPixelGameEngine.h"
#include "CPU_6502.h"
#include "deassembler.h"

class dmonitor : public olc::PixelGameEngine
{
public:
	dmonitor(CPU_6502& cpuRef, deassembler& deasmRef, std::string& deassemblyRef) : _cpu(cpuRef), _deasm(deasmRef), _deassembly(deassemblyRef)
	{
		sAppName = "6502 debug monitor";
	}

private:
	CPU_6502& _cpu;
	deassembler& _deasm;
	std::string& _deassembly;

	void drawWindows()
	{
		Clear(olc::PixelF(0.25f, 0.25f, 0.25f));

		FillRect({ 32, 48 }, { 544, 544 }, olc::BLACK);

		FillRect({ 864, 48 }, { 392, 296 }, olc::BLACK);

		FillRect({ 864, 368 }, { 392, 16 }, olc::PixelF(0.35f, 0.35f, 0.35f));
		FillRect({ 864, 384 }, { 392, 208 }, olc::PixelF(0.20f, 0.20f, 0.20f));
		for(int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
			{
				if((i + j) % 2 == 0)
					FillRect({ 872 - 4 + 96*j, 392 - 2 + 48*i }, { 88 + 8, 44 + 4 }, olc::PixelF(0.10f, 0.10f, 0.10f));
			}

	}

	void drawPC()
	{
		DrawString({ 704, 64 }, "PC", olc::WHITE, 4);
		DrawString({ 672, 128 }, _deasm.itohex(_cpu.rPC, true, true), olc::WHITE, 4);

	}

	void drawInternalStatus()
	{
		drawPC();

		DrawString({ 672, 192 }, "A " + _deasm.itohex(_cpu.rA, false, true), olc::WHITE, 4);

		DrawString({ 672, 256 }, "X " + _deasm.itohex(_cpu.rX, false, true), olc::WHITE, 4);

		DrawString({ 672, 320 }, "Y " + _deasm.itohex(_cpu.rY, false, true), olc::WHITE, 4);

		DrawString({ 640, 384 }, "SP " + _deasm.itohex(_cpu.rSP, false, true), olc::WHITE, 4);

		std::string binP;
		for (int i = 7; i >= 0; i--)
		{
			if (i != 5)
				binP += (int)_cpu.getFlag(CPU_6502::status_flag(i)) + '0';
		}
		DrawString({ 704, 448 }, "P", olc::WHITE, 4);
		DrawString({ 608, 512 }, "NVBDIZC\n" + binP, olc::WHITE, 4);
	}

	void drawPage()
	{
		DrawString({ 872, 372 }, "Page " + _deasm.itohex(page, false, true));

		if (page == 1)
		{
			FillRect({ 872 + 24 * (_cpu.rSP & 0xF), 392 + 12 * ((_cpu.rSP >> 4) & 0xF) }, { 16,8 }, olc::PixelF(0.4f, 0.2f, 0.2f));
		}

		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < 16; j++)
				DrawString({ 872 + 24 * j, 392 + 12 * i }, _deasm.itohex(_cpu.RAM[256 * page + 16 * i + j], false, true));

		}
	}

	int line = 0;
	int startDeasm;
	int lenDeasm;

	void drawDeassembly()
	{
		if (lenDeasm == 0)
			return;

		for(const auto& i : breakpoints)
		{
			if (i >= line/2 && i < line/2 + 16)
				FillCircle({ 882 , 59 + 16*(i-line/2)}, 5, olc::RED);
		}

		DrawString({ 892, 56 }, _deassembly.substr(startDeasm, lenDeasm), olc::WHITE, 1);
	}

	unsigned char page = 0;

	void drawMonitor()
	{
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
		}

		int ch;
		for (ch = 0; 32*ch < monitorText.size() - 32; ch++)
			DrawString({ 48, 64 + 16 * ch }, monitorText.substr(32 * ch, 32), olc::WHITE, 2);
		DrawString({ 48, 64 + 16 * ch }, monitorText.substr(32 * ch, monitorText.size() - 32 * ch));
	}

	void drawAll()
	{
		drawWindows();
		drawMonitor();
		drawDeassembly();
		drawInternalStatus();
		drawPage();
	}

	void findSubstrDeasm()
	{
		int l = line;
		int ch = 0;
		lenDeasm = 0;
		while (l > 0)
		{
			if (ch >= _deassembly.size())
				return;

			if (_deassembly[ch] == '\n')
				l--;
			ch++;
		}

		startDeasm = ch;

		l = 36;
		while (l > 0 && ch < _deassembly.size())
		{
			if (_deassembly[ch] == '\n')
				l--;
			ch++;
			lenDeasm++;
		}
	}

	bool OnUserCreate() override
	{
		findSubstrDeasm();
		drawAll();
		return true;
	}

	float waitTime = 0.1f;
	float timePassed = 0.0f;

	bool run = false;

	std::vector<int> breakpoints;

	bool OnUserUpdate(float fElapsedTime) override
	{
		bool redraw = false;
		timePassed += fElapsedTime;
		
		if (GetKey(olc::Key::R).bPressed)
		{
			_cpu.init();
			redraw = true;
		}

		if (GetKey(olc::Key::SPACE).bPressed)
		{
			timePassed = 0.0f;
			run = !run;
		}

		if (run)
		{
			do
			{
				for (const auto& brkpt : breakpoints)
					if (_deasm.assembledInsts[brkpt] == _cpu.rPC)
					{
						run = false;
						break;
					}

			} while (run && _cpu.execute());
			run = false;
			redraw = true;
		}
		else
		{
			if (GetKey(olc::Key::E).bPressed)
			{
				redraw = true;

				_cpu.execute();
			}
		}

		if (GetMouse(0).bPressed)
		{
			olc::vi2d mousePos = GetMousePos();
			
			if (mousePos.x >= 864 && mousePos.x <= 864 + 392 && mousePos.y >= 48 && mousePos.y <= 48 + 296)
			{
				int l;
				l = (mousePos.y - 48) / 16 + line / 2;
				std::cout << l << "\n";
				bool found = false;
			
				for (auto i = breakpoints.begin(); i != breakpoints.end(); i++)
				{
					if (*i == l)
					{
						found = true;
						breakpoints.erase(i);
						break;
					}
				}
				if(!found)
					breakpoints.push_back(l);
				redraw = true;
			}

		}
		
		int mouseWheel = GetMouseWheel();
		if (mouseWheel != 0)
		{
			olc::vi2d mousePos = GetMousePos();

			if (mousePos.x >= 864 && mousePos.x <= 864 + 392 && mousePos.y >= 384 && mousePos.y <= 384 + 208)
			{
				if (mouseWheel > 0)
				{
					page--;
				}
				else
				{
					page++;
				}
			}
			else
			{
				if (mouseWheel > 0)
				{
					if (line >= 4)
						line -= 4;
				}
				else
				{
					line += 4;
				}

				findSubstrDeasm();
			}
			redraw = true;

		}
		else if (GetKey(olc::Key::UP).bPressed)
		{
			page = (page + 16) & 0xF0;
			redraw = true;
		}
		else if (GetKey(olc::Key::DOWN).bPressed)
		{
			page = (page - 16) & 0xF0;
			redraw = true;
		}
		else if (GetKey(olc::Key::LEFT).bPressed)
		{
			page--;
			redraw = true;
		}
		else if (GetKey(olc::Key::RIGHT).bPressed)
		{
			page++;
			redraw = true;
		}
		
		if (redraw)
			drawAll();
	
		return true;
	}
};

