#pragma once
#include "olcPixelGameEngine.h"
#include "CPU_6502.h"
#include "disassembler.h"
#include "keyToAscii.h"

class dmonitor : public olc::PixelGameEngine
{
public:
	dmonitor(CPU_6502& cpuRef, disassembler& disasmRef, std::string& disassemblyRef) : _cpu(cpuRef), _disasm(disasmRef), _disassembly(disassemblyRef)
	{
		sAppName = "6502 debug monitor";
	}

private:
	CPU_6502& _cpu;
	disassembler& _disasm;
	std::string& _disassembly;

	void drawWindows()
	{
		Clear(olc::PixelF(0.25f, 0.25f, 0.25f));

		FillRect({ 48, 48 }, { 544, 544 }, olc::BLACK);

		FillRect({ 864, 48 }, { 392, 296 }, olc::BLACK);

		FillRect({ 1236, 48 }, { 20, 296 }, olc::PixelF(0.6f, 0.6f, 0.6f));

		FillRect({ 864, 368 }, { 392, 16 }, olc::PixelF(0.35f, 0.35f, 0.35f));
		FillRect({ 864, 384 }, { 392, 208 }, olc::PixelF(0.20f, 0.20f, 0.20f));
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
			{
				if ((i + j) % 2 == 0)
					FillRect({ 872 - 4 + 96 * j, 392 - 2 + 48 * i }, { 88 + 8, 44 + 4 }, olc::PixelF(0.10f, 0.10f, 0.10f));
			}

	}

	void drawPC()
	{
		DrawString({ 704, 64 }, "PC", olc::WHITE, 4);
		DrawString({ 672, 128 }, _disasm.itohex(_cpu.rPC, true, true), olc::WHITE, 4);

	}

	void drawInternalStatus()
	{
		drawPC();

		DrawString({ 672, 192 }, "A " + _disasm.itohex(_cpu.rA, false, true), olc::WHITE, 4);

		DrawString({ 672, 256 }, "X " + _disasm.itohex(_cpu.rX, false, true), olc::WHITE, 4);

		DrawString({ 672, 320 }, "Y " + _disasm.itohex(_cpu.rY, false, true), olc::WHITE, 4);

		DrawString({ 640, 384 }, "SP " + _disasm.itohex(_cpu.rSP, false, true), olc::WHITE, 4);

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
		DrawString({ 872, 372 }, "Page " + _disasm.itohex(page, false, true));

		if (page == 1)
		{
			FillRect({ 872 + 24 * (_cpu.rSP & 0xF), 392 + 12 * ((_cpu.rSP >> 4) & 0xF) }, { 16,8 }, olc::PixelF(0.4f, 0.2f, 0.2f));
		}

		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < 16; j++)
				DrawString({ 872 + 24 * j, 392 + 12 * i }, _disasm.itohex(_cpu.RAM[256 * page + 16 * i + j], false, true));

		}
	}

	int line = 0;
	int startdisasm;
	int lendisasm;

	void drawdisassembly()
	{
		if (lendisasm == 0)
			return;

		int currentLine = std::lower_bound(_disasm.assembledInsts.begin(), _disasm.assembledInsts.end(), _cpu.rPC) - _disasm.assembledInsts.begin();
		if (currentLine >= line / 2 && currentLine < line / 2 + 18)
			DrawLine({ 875 , 59 + 16 * (currentLine - line / 2) }, { 890 , 59 + 16 * (currentLine - line / 2) }, olc::YELLOW);

		for (const auto& i : breakpoints)
		{
			if (i >= line / 2 && i < line / 2 + 18)
				FillCircle({ 897 , 59 + 16 * (i - line / 2) }, 5, olc::RED);
		}

		DrawString({ 912, 56 }, _disassembly.substr(startdisasm, lendisasm), olc::WHITE, 1);

		int posStart = 48 + int(double(line) / (double)_disasm.assembledInsts.size() / 2.0 * 296.0);
		int posEnd = 48 + int(double(line + 36) / (double)_disasm.assembledInsts.size() / 2.0 * 296.0);
		if (posEnd > 344)
			posEnd = 344;
		FillRect({ 1236,posStart }, { 20, posEnd - posStart }, olc::PixelF(0.33f, 0.33f, 0.33f));

		for (const int& b : breakpoints)
		{
			int bPos = 48 + int(double(b) / (double)_disasm.assembledInsts.size() * 296.0);
			DrawLine({ 1236,bPos }, { 1256,bPos }, olc::RED);
		}
		int curPos = 48 + int(double(currentLine) / (double)_disasm.assembledInsts.size() * 296.0);

		DrawLine({ 1236,curPos }, { 1256,curPos }, olc::YELLOW);
	}

	unsigned char page = 0;

	void drawMonitor()
	{
		if ((_cpu.RAM[0xC] & 1) == 0)
		{
			std::string monitorText;
			int ch = 0;
			for (int i = 0; i < 32; i++)
			{
				for (int j = 0; j < 32; j++)
				{
					ch++;
					if (ch == 32)
					{
						monitorText += "\n";
						ch = 0;
					}
					char c = _cpu.RAM[0x8000 + i * 32 + j];

					if (c >= 32 && c <= 127 || c == '\n')
					{
						monitorText += c;
						if (c == '\n')
						{
							ch = 0;
						}
					}
					else
						monitorText += '\0';
				}
			}

			int begin = 0;
			std::size_t nextNewline = monitorText.find('\n');
			for (int l = 0; nextNewline != std::string::npos && l < 25; l++)
			{
				DrawString({ 48, 64 + 20 * l }, monitorText.substr(begin, nextNewline - begin), olc::WHITE, 2);

				begin = nextNewline + 1;
				nextNewline = monitorText.find('\n', begin);
			}
		}
		else
		{
			for (int y = 0; y < 128; y++)
				for (int x = 0; x < 128; x++)
				{
					unsigned char rgb = _cpu.RAM[0x4000 + 128 * y + x];
					olc::Pixel color(int(((rgb >> 5) & 7)* 36.42858), int(((rgb >> 2) & 7)* 36.4285), (rgb & 3)*85);
					FillRect({ 64 + 4 * x, 64 + 4 * y }, { 4,4 }, color);
				}
		}
	}

	void drawAll()
	{
		drawWindows();
		drawMonitor();
		drawdisassembly();
		drawInternalStatus();
		drawPage();
	}

	void findSubstrdisasm()
	{
		int l = line;
		int ch = 0;
		lendisasm = 0;
		while (l > 0)
		{
			if (ch >= _disassembly.size())
				return;

			if (_disassembly[ch] == '\n')
				l--;
			ch++;
		}

		startdisasm = ch;

		l = 36;
		while (l > 0 && ch < _disassembly.size())
		{
			if (_disassembly[ch] == '\n')
				l--;
			ch++;
			lendisasm++;
		}
	}

	void followDisassemblyIfOut(bool minusArow = false)
	{
		int upperBound;
		if (_disasm.assembledInsts.size() <= line / 2 + 18)
			upperBound = 0x10000;
		else
			upperBound = _disasm.assembledInsts[line / 2 + 18];			
		
		if (!(_cpu.rPC >= _disasm.assembledInsts[line / 2] && _cpu.rPC < upperBound))
		{
			for (int i = 0; i < _disasm.assembledInsts.size(); i++)
			{
				if (_disasm.assembledInsts[i] == _cpu.rPC)
				{
					line = 2 * i;
					if (minusArow)
						line -= 2;
						findSubstrdisasm();
						break;
				}
			}
		}
	}

	bool OnUserCreate() override
	{
		findSubstrdisasm();
		drawAll();
		return true;
	}

	float waitTime = 0.1f;
	float timePassed = 0.0f;

	bool run = false;
	bool touchedScrollbar = false;

	std::vector<int> breakpoints;

	bool waitingForInput = false;

	//RAM[0XD] = xxxxxCSx C = caps lock S = shift

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (waitingForInput)
		{
			_cpu.RAM[0xD] = (_cpu.RAM[0xD] & 0xFD) + 0x2 * GetKey(olc::Key::SHIFT).bHeld;

			if (AnyKeyPressed() && !GetKey(olc::Key::SHIFT).bPressed)
			{
				int index = std::lower_bound(valueInputKeys.begin(), valueInputKeys.end(), GetLastKeyPressed(), [](const KeyCharMap& lhs, olc::Key key) {return (int)lhs.k < (int)key; }) - valueInputKeys.begin();

				if (GetKey(olc::Key::CAPS_LOCK).bPressed)
				{
					_cpu.RAM[0xD] = (_cpu.RAM[0xD] & 0xFB) + 0x4 * int(!bool(_cpu.RAM[0xD] >> 2));
				}
				else
					if (index != valueInputKeys.size())
					{
						_cpu.RAM[0xE] = 1;
						_cpu.RAM[0xF] = valueInputKeys[index].l;
						waitingForInput = false;
					}
			}

			_cpu.execute(nullptr);
			drawAll();
			return true;
		}

		bool redraw = false;

		timePassed += fElapsedTime;

		if (GetKey(olc::Key::C).bPressed)
		{
			_cpu.mon_clear();

			redraw = true;
		}

		if (GetKey(olc::Key::R).bPressed)
		{
			_cpu.init();
			followDisassemblyIfOut();

			redraw = true;
		}

		bool canContinue = false;

		if (GetKey(olc::Key::SPACE).bPressed)
		{
			canContinue = true;
			timePassed = 0.0f;
			run = !run;
		}

		if (run)
		{
			while (run && _cpu.execute(nullptr, &waitingForInput) && !waitingForInput)
			{
				for (const auto& brkpt : breakpoints)
					if (brkpt < _disasm.assembledInsts.size())
						if (_disasm.assembledInsts[brkpt] == _cpu.rPC)
						{
							run = false;
							break;
						}

			}
			followDisassemblyIfOut(true);

			if (!waitingForInput)
				run = false;
			redraw = true;
		}
		else
		{
			if (GetKey(olc::Key::I).bPressed)
			{
				redraw = true;

				_cpu.execute(nullptr, &waitingForInput);

				followDisassemblyIfOut();
			}

			if (GetKey(olc::Key::O).bPressed)
			{
				redraw = true;

				int functionCalls = 0;

				if (_cpu.RAM[_cpu.rPC] == 0x20) // 20 = JSR
				{
					do {
						_cpu.execute(nullptr, &waitingForInput);
						if (_cpu.RAM[_cpu.rPC] == 0x20)
						{
							functionCalls++;
						}
						else if (_cpu.RAM[_cpu.rPC] == 0x60)
						{
							functionCalls--;
						}
					} while (_cpu.RAM[_cpu.rPC] != 0x60 || functionCalls >= 0); // 60 = RTS
					_cpu.execute(nullptr, &waitingForInput);
				}
				else
					_cpu.execute(nullptr, &waitingForInput);

				followDisassemblyIfOut();
			}
		}

		if (GetMouse(0).bHeld)
		{
			olc::vi2d mousePos = GetMousePos();
			if (GetMouse(0).bPressed)
			{
				if (mousePos.x >= 864 && mousePos.x <= 864 + 372 && mousePos.y >= 48 && mousePos.y <= 48 + 296)
				{
					int l;
					l = (mousePos.y - 48) / 16 + line / 2;
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
					if (!found)
						breakpoints.push_back(l);
					redraw = true;
				}
				else if (mousePos.x >= 1236 && mousePos.x <= 1256 && mousePos.y >= 48 && mousePos.y <= 48 + 296)
				{
					touchedScrollbar = true;
				}
			}

			if (touchedScrollbar && mousePos.y <= 48 + 296)
			{
				if (mousePos.y < 48)
					line = 0;
				else
					line = int(double((mousePos.y - 48) * 2 * _disasm.assembledInsts.size()) / 296.0);
				findSubstrdisasm();
				redraw = true;
			}
		}
		else
			touchedScrollbar = false;

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
					if (line < 4)
						line = 0;
					else
						line -= 4;
				}
				else
				{
					line += 4;
				}

				findSubstrdisasm();
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

