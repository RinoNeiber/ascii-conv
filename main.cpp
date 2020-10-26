#define _WIN32_WINNT 0x0601

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <algorithm>
#include <conio.h>
#include <tuple>
#include <iterator>
#include "Genetic.h"

// Tuple fields: name, popylation size, sensitivity, write out, show help.
std::tuple<std::string, int, int, bool, bool>
parsing(int, char**);

void print_result(const CHAR_INFO*, COORD, const Colour*);

int main(int argc, char* argv[])
{
	using namespace std;

	string name;
	int pop_size, sensitivity;
	bool write_out, show_help;
	tie(name, pop_size, sensitivity, write_out, show_help) = parsing(argc, argv);
	
	if (show_help)
	{
		cout << endl;
		cout << "Usage: conv image_name [-p <number>] [-s <number>] [-w]" << endl << endl;
		cout << "Parameters:" << endl << endl;
		cout << setfill(' ') << setw(17) << left << "    -p <number>";
		cout << "Defines population size. Recommended range 400 .. 1600." << endl;
		cout << setfill(' ') << setw(17) << left << "    -s <number>";
		cout << "Defines sensitivity for pixel's comparation. Recommended range " << endl;
		cout << setfill(' ') << setw(17) << left << "     ";
		cout << "10 .. 30 (smaller - more strict selection of colours)." << endl;
		cout << setfill(' ') << setw(17) << left << "    -w";
		cout << "Enables writing source image in 16-colour palette" << endl;
		cout << setfill(' ') << setw(17) << left << "     ";
		cout << "in reduced.png file." << endl << endl;
		return 0;
	}

	// Creating relative or absolute path to arts from app directory.
	string path(argv[0]);
	replace_if(path.begin(), path.end(), [] (const char& s) {return s == '/';}, '\\');
	if (path.find(':') == string::npos) path.insert(0, ".\\");
	//size_t idx = path.rfind('\\');
	//path.replace(idx, path.length()-idx, "\\");  // Groundwork for the future.

	Image source_img, ascii_img;
	source_img.load(path + name + ".png");
	ascii_img.load(path + "font.png");
// --- --- ---
	FreeConsole();
	AllocConsole();

	// Finding most suitable palette.
	Genetic gen;
	int offset = 1;
	Colour palette[16] = {{0, 0, 0}};  // Reserving black colour for background.
	gen.compute(&source_img, palette, offset, pop_size, sensitivity);

	ascii_img.Palette(default_palette);
	source_img.Palette(palette);
	source_img.reduce_colours();

	if (write_out) source_img.write("output.png");

	// Getting cells.
	SHORT X_count = source_img.Width()/sym_width;
	SHORT Y_count = source_img.Height()/sym_height;

	Cell ascii[128];
	Cell* cell = new Cell[X_count * Y_count];
	for(int i=0; i < X_count*Y_count; i++)
	{
		int X = i%X_count, Y = i/X_count;
		source_img.get_cell(X, Y, cell[i]);

		if ((i==0)||(i==7)||(i==8)||(i==9)||(i==10)||(i==13) || (i>127)) continue;
		ascii_img.get_cell(i%64, i/64, ascii[i]);
	}
	
	// Converting into symbols.
	CHAR_INFO* result_img = new CHAR_INFO[X_count * Y_count];
	for(int i=0; i < X_count*Y_count; i++)
	{
		// Setting colour.
		for(int j=0; j<16; j++)
		{
			if (cell[i].colour == palette[j])
			{
				result_img[i].Attributes = static_cast<WORD>((0 << 4) | j);
				break;
			}
		}

		// Choosing symbol.
		int max = 0;
		for(int j=0; j<128; j++)
		{
			// Skipping technical symbols.
			if ((j==0)||(j==7)||(j==8)||(j==9)||(j==10)||(j==13)) continue;

			if (cell[i].colour == palette[0])
			{
				result_img[i].Char.AsciiChar = ' ';
				continue;
			}

			// WARNING: there is template image only for 8x12 sized font!
			int similarity = 0;
			for(int k=0; k<96; k++)
			{	
				bool cell_pix, symbol_pix;
				if (cell[i].figure[k] == cell[i].colour) cell_pix = true;
				else cell_pix = false;
				if (ascii[j].figure[k] == default_palette[7]) symbol_pix = true;
				else symbol_pix = false;
					
				if (cell_pix == symbol_pix) similarity += 2;
				else if ((cell_pix == false) && (symbol_pix == true)) similarity -= 1;
			}
			if (similarity > max)
			{
				max = similarity;
				result_img[i].Char.AsciiChar = static_cast<CHAR>(j);
			}
		}
	}
// --- --- ---
	COORD size = {X_count, Y_count};
	print_result(result_img, size, palette);
	
	delete cell;
	delete result_img;
	
	getch();
	return 0;
}

std::tuple<std::string, int, int, bool, bool>
parsing(int argc, char** argv)
{
	using namespace std;
	vector<string> errors;

	string name;
	int first_param_idx = 2;
	if (argc > 1)
	{
		name = argv[1];
		if (name[0] == '-' || name[0] == '/')
		{
			name = "input";
			first_param_idx = 1;
		}
	}
	else name = "input";
	
	string parameter;
	int pop_size = 600, sensitivity = 20, value;
	bool write_out = false, show_help = false;
	for(int i=first_param_idx; i<argc; i++)
	{
		parameter = argv[i];
		
		if (parameter.size() != 2)
		{
			errors.push_back("Invalid parameter " + parameter + ".");
			continue;
		}
		if (parameter[0] != '-' && parameter[0] != '/')
		{
			errors.push_back("Expect symbol '-' or '/' in parameter " + parameter + ".");
			continue;
		}
		if (parameter[1] != 'p' && parameter[1] != 's' && parameter[1] != 'w' && parameter[1] != 'h' && parameter[1] != '?')
		{
			errors.push_back("Invalid parameter " + parameter + ".");
			continue;
		}

		if (parameter[1] == 'h' || parameter[1] == '?')
		{
			show_help = true;
			break;
		}
		if (parameter[1] == 'w')
		{
			write_out = true;
			continue;
		}

		if (i+1 >= argc)
		{
			errors.push_back("No value for parameter " + parameter + ".");
			break;
		}
		try {value = stoi(argv[++i]);}
		catch (invalid_argument e)
		{
			errors.push_back("Invalid value for parameter " + parameter + ".");
			continue;
		}
		if (value < 1)
		{
			errors.push_back("Invalid value for parameter " + parameter + ".");
			continue;
		}

		if (parameter[1] == 'p') pop_size = value;
		if (parameter[1] == 's') sensitivity = value;
	}

	if (errors.size())
	{
		copy(errors.begin(), errors.end(), ostream_iterator<string>(cout, "\n"));
		show_help = true;
	}
	return make_tuple(name, pop_size, sensitivity, write_out, show_help);
}

void print_result(const CHAR_INFO* buffer, COORD size, const Colour* palette)
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	CONSOLE_SCREEN_BUFFER_INFOEX info;
	info.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	GetConsoleScreenBufferInfoEx(hOut, &info);
	
	for(int i=0; i<16; i++) info.ColorTable[i] = RGB(palette[i].r, palette[i].g, palette[i].b);
	
	COORD pos = info.dwCursorPosition;
	if (pos.X != 0) pos = {0, (SHORT)(pos.Y + 1)};
	
	info.dwSize.X = size.X;
	info.dwSize.Y = (SHORT)(pos.Y + size.Y);
	
	info.bFullscreenSupported = true;
	SetConsoleScreenBufferInfoEx(hOut, &info);
	
	SMALL_RECT r = {pos.X, pos.Y, (SHORT)(pos.X + size.X), (SHORT)(pos.Y + size.Y)};
	WriteConsoleOutput(hOut, buffer, size, {0, 0}, &r);
	
	pos = {0, (SHORT)(pos.Y + size.Y)};
	SetConsoleCursorPosition(hOut, pos);
}