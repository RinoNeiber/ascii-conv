#ifndef IMAGE_H
#define IMAGE_H

#include <png.h>
#include <string>
#include <vector>

// Enables/disables logging (must be removed... someday...).
const bool DEBUG = false;

struct Colour
{
	unsigned char r, g, b;

	int max_delta (Colour x) const;  // Linear norm.
	int eucl_delta(Colour x) const;  // Euclidean norm.
	int internal_delta() const;
	
	friend bool operator == (const Colour& left, const Colour& right);
	
	// For debug purpose.
	void print() const;
};

const int sym_width = 8;
const int sym_height = 12;

const Colour default_palette[] =
{
	{  0,   0,   0},  // 0  Black.
	{  0,   0, 128},  // 1  Blue.
	{  0, 128,   0},  // 2  Green.
	{  0, 128, 128},  // 3  Cyan.
	{128,   0,   0},  // 4  Red.
	{128,   0, 128},  // 5  Magenta.
	{128, 128,   0},  // 6  Brown.
	{192, 192, 192},  // 7  Light gray.
	{128, 128, 128},  // 8  Dark gray.
	{  0,   0, 255},  // 9  Light blue.
	{  0, 255,   0},  // 10 Light green.
	{  0, 255, 255},  // 11 Light cyan.
	{255,   0,   0},  // 12 Light red.
	{255,   0, 255},  // 13 Light magenta.
	{255, 255,   0},  // 14 Yellow.
	{255, 255, 255}   // 15 White.
};

struct Cell
{
	// Leftovers from previous experiments.
	// There would be more, further in the code. I don't remember them all.
	int type;
	int region;

	Colour colour;
	std::vector<int> pixels;     // How many pixels of each colour are presented in cell.
	std::vector<Colour> figure;  // Stores a piece of original image.
	
	// For debug purpose.
	void print(const Colour*) const;
};

class Image
{
public:
	void load(const std::string&);
	void write(const std::string&);
	
	Colour get_colour(int, int) const;
	
	void reduce_colours();
	void get_cell(int, int, Cell&);
	void perform_dithering(const Colour*, int);
	
	// Setters.
	Image& Palette(const Colour*);
	
	// Getters.
	int Height() const;
	int Width() const;
	
	~Image();
private:
	int width;
	int height;
	png_byte colour_type;
	png_byte bit_depth;
	
	Colour palette[16];
	
	png_bytep* row_pointers;
};

#endif  // IMAGE_H