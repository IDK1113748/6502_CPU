#pragma once
#include <vector>
#include "olcPixelGameEngine.h"

struct KeyCharMap
{
	KeyCharMap(olc::Key _k, char _l, char _u) : k(_k), l(_l), u(_u) {}

	olc::Key k;
	char l;
	char u;
};

std::vector<KeyCharMap> valueInputKeys =
{
	{olc::A, 'a', 'A'},
	{olc::B, 'b', 'B'},
	{olc::C, 'c', 'C'},
	{olc::D, 'd', 'D'},
	{olc::E, 'e', 'E'},
	{olc::F, 'f', 'F'},
	{olc::G, 'g', 'G'},
	{olc::H, 'h', 'H'},
	{olc::I, 'i', 'I'},
	{olc::J, 'j', 'J'},
	{olc::K, 'k', 'K'},
	{olc::L, 'l', 'L'},
	{olc::M, 'm', 'M'},
	{olc::N, 'n', 'N'},
	{olc::O, 'o', 'O'},
	{olc::P, 'p', 'P'},
	{olc::Q, 'q', 'Q'},
	{olc::R, 'r', 'R'},
	{olc::S, 's', 'S'},
	{olc::T, 't', 'T'},
	{olc::U, 'u', 'U'},
	{olc::V, 'v', 'V'},
	{olc::W, 'w', 'W'},
	{olc::X, 'x', 'X'},
	{olc::Y, 'y', 'Y'},
	{olc::Z, 'z', 'Z'},
	{olc::K0, '0', ')'},
	{olc::K1, '1', '!'},
	{olc::K2, '2', '@'},
	{olc::K3, '3', '#'},
	{olc::K4, '4', '$'},
	{olc::K5, '5', '%'},
	{olc::K6, '6', '^'},
	{olc::K7, '7', '&'},
	{olc::K8, '8', '*'},
	{olc::K9, '9', '('},
	{olc::SPACE, ' ', ' '},
	{olc::BACK, '\b', '\b'},
	{olc::ENTER, '\n', '\n'},
	{olc::NP0, '0', '0'},
	{olc::NP1, '1', '1'},
	{olc::NP2, '2', '2'},
	{olc::NP3, '3', '3'},
	{olc::NP4, '4', '4'},
	{olc::NP5, '5', '5'},
	{olc::NP6, '6', '6'},
	{olc::NP7, '7', '7'},
	{olc::NP8, '8', '8'},
	{olc::NP9, '9', '9'},
	{olc::NP_MUL, '*', '*'},
	{olc::NP_DIV, '/', '/'},
	{olc::NP_ADD, '+', '+'},
	{olc::NP_SUB, '-', '-'},
	{olc::NP_DECIMAL, '.', '.'},
	{olc::PERIOD, '.', '>'},
	{olc::EQUALS, '=', '+'},
	{olc::COMMA, ',', '<'},
	{olc::MINUS, '-', '_'},
	{olc::OEM_1, ';', ':'},
	{olc::OEM_2, '/', '?'},
	{olc::OEM_3, '`', '~'},
	{olc::OEM_4, '[', '{'},
	{olc::OEM_5, '\\', '|'},
	{olc::OEM_6, ']', '}'},
	{olc::OEM_7, '\'', '"'},
	{olc::OEM_8, '-', '-'}
};

