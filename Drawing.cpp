#define _WIN32_WINNT 0x0601

#include <iostream>
#include <iomanip>
#include <windows.h>
#include "Drawing.h"

static const SMALL_RECT size = {0, 0, 55, 25};
static HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
static CONSOLE_SCREEN_BUFFER_INFOEX info;

static void draw(SHORT, SHORT, const char*, DWORD);
static void draw_number(SHORT, SHORT, double, int);
static void draw_rect(SHORT, Colour);

void cli::draw_borders()
{
	info.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	SetConsoleWindowInfo(hOut, true, &size);
	GetConsoleScreenBufferInfoEx(hOut, &info);
	info.ColorTable[15] = RGB(192, 192, 192);
	info.dwSize = {size.Right, size.Bottom};
	info.bFullscreenSupported = true;
	info.wAttributes = 15;
	SetConsoleScreenBufferInfoEx(hOut, &info);

	CONSOLE_CURSOR_INFO cursor;
	GetConsoleCursorInfo(hOut, &cursor);
	cursor.bVisible = false;
	SetConsoleCursorInfo(hOut, &cursor);
	
	char symbol = -77;  // Vertical line for columns in tables.
	for(SHORT i=0; i<21; i++)
	{
		draw( 4, 3+i, &symbol, 1);
		draw(17, 3+i, &symbol, 1);
		draw(27, 3+i, &symbol, 1);
	}
	for(SHORT i=0; i<9;  i++) draw(43, 15+i, &symbol, 1);
	
	// Table 4x4 for palette.
	char up[]         = {-55, -51, -51, -51, -47, -51, -51, -51, -47, -51, -51, -51, -47, -51, -51, -51, -69};
	char bottom[]     = {-56, -51, -51, -51, -49, -51, -51, -51, -49, -51, -51, -51, -49, -51, -51, -51, -68};
	char middle_one[] = {-70,  32,  32,  32, -77,  32,  32,  32, -77,  32,  32,  32, -77,  32,  32,  32, -70};
	char middle_two[] = {-57, -60, -60, -60, -59, -60, -60, -60, -59, -60, -60, -60, -59, -60, -60, -60, -74};

	for(SHORT i=0; i<4; i++)
	{
		draw(37, 2+i*3, middle_one, 17);
		draw(37, 3+i*3, middle_one, 17);
		draw(37, 4+i*3, middle_two, 17);
	}
	draw(37, 1, up, 17);
	draw(37, 13, bottom, 17);

	draw( 1,  1, "Searching colours.", 18);
	draw( 1,  3, "Age",                 3);
	draw( 5,  3, "Elapsed time",       12);
	draw(18,  3, "Max. eff.",           9);
	draw(28,  3, "R, G, B ",            8);
	draw(37, 15, "Colour",              6);
	draw(44, 15, "Total time",         10);
}

void cli::update_elipsis(double delta)
{
	static int time = 0;
	time = int(time+delta) % 1500;
	
	if (time < 500) draw(19, 1, "  ", 2);
	else if (time > 1000) draw(19, 1, "..", 2);
	else draw(19, 1, ". ", 2);
}

void cli::update_colour(Colour colour, SHORT i, double delta, int pixels_count)
{
	draw_rect(i, colour);

	static double time = 0;
	time += delta;
	if (i%2)
	{
		draw_number(37, 16 + (i/2), i, 6);
		draw_number(44, 16 + (i/2), time, 10);
	}
	draw_number(24, 1, pixels_count, 12);
}

void cli::update_main_table(int age, double delta, int eff, Colour colour, SHORT i)
{
	using namespace std;

	static SHORT row = -1;
	row = (row+1) % 20;

	static double time = 0;
	if (age == 0) time = 0;
	time += delta;

	draw_number( 1, 4 + row,   age,  3);
	draw_number( 5, 4 + row, delta, 12);
	draw_number(18, 4 + row,   eff,  9);
	draw( 1, 5 + row, "   ",           3);
	draw( 5, 5 + row, "            ", 12);
	draw(18, 5 + row, "         ",     9);
	draw(28, 5 + row, "        ",      8);
	draw(30, 4 + row, ";", 1);
	draw(33, 4 + row, ";", 1);
	draw_rect(i, colour);

	COORD pos = {28, 4 + row};
	SetConsoleCursorPosition(hOut, pos);
	cout << setw(2) << hex << (int)colour.r << ';' << setw(2) << (int)colour.g << ';' << setw(2) << (int)colour.b;
}

static void draw(SHORT x, SHORT y, const char* buf, DWORD size)
{
	DWORD temp;
	COORD pos = {x, y};
	SetConsoleCursorPosition(hOut, pos);
	WriteConsole(hOut, buf, size, &temp, nullptr);
}

static void draw_number(SHORT x, SHORT y, double value, int length)
{
	COORD pos = {x, y};
	SetConsoleCursorPosition(hOut, pos);
	std::cout << std::setfill(' ') << std::setw(length) << std::left << value;
}

static void draw_rect(SHORT i, Colour colour)
{
	info.ColorTable[i] = RGB(colour.r, colour.g, colour.b);
	SetConsoleScreenBufferInfoEx(hOut, &info);
	
	CHAR_INFO buf[6];
	for(int j=0; j<6; j++)
	{
		buf[j].Char.AsciiChar = -37;
		buf[j].Attributes = static_cast<WORD>((0 << 4) | i);
	}
	COORD size = {3, 2}, buf_pos = {0, 0}, pos = {SHORT(38 + 4*(i%4)), SHORT(2 + 3*(i/4))};
	SMALL_RECT r = {pos.X, pos.Y, pos.X+2, pos.Y+2};
	WriteConsoleOutput(hOut, buf, size, buf_pos, &r);
}
