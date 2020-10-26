#include <fstream>
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <cmath>
#include "Image.h"

static void error_exit(const std::string&);

void Image::load(const std::string& name)
{
	std::ifstream fin(name, std::ios_base::in|std::ios_base::binary);
	if (!fin.is_open()) error_exit("Can\'t open file \"" + name + "\".");
	
	char buffer[8];
	fin.read(buffer, 8);
	fin.close();
	
	png_bytep header = reinterpret_cast<png_bytep>(buffer);
	
	if (png_sig_cmp(header, 0, 8)) error_exit("File \"" + name + "\" doesn't have PNG signature.");
// --- --- ---
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) error_exit("PNG read struct wasn't created.");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		error_exit("PNG info struct wasn't created.");
	}
	// Anchor in case if errors in pnglib will be occurred.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		error_exit("A-A-A, STOP, OSHIBKA 0x000A!!!");
	}
	
	FILE* fp = fopen(name.c_str(), "rb");
	png_init_io(png_ptr, fp);
// --- --- ---
	png_read_info(png_ptr, info_ptr);
	
	width  = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	bit_depth   = png_get_bit_depth(png_ptr, info_ptr);
	colour_type = png_get_color_type(png_ptr, info_ptr);
	
	if (bit_depth == 16) png_set_strip_16(png_ptr);
	
	if (colour_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16 bit depth.
	if (colour_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if (colour_type == PNG_COLOR_TYPE_RGB  ||
		colour_type == PNG_COLOR_TYPE_GRAY ||
		colour_type == PNG_COLOR_TYPE_PALETTE) png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

	if (colour_type == PNG_COLOR_TYPE_GRAY ||
		colour_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);

	png_read_update_info(png_ptr, info_ptr);
	
	row_pointers = reinterpret_cast<png_bytep*>(malloc(sizeof(png_bytep) * height));
	for(int y=0; y<height; y++)
	{
		row_pointers[y] = reinterpret_cast<png_byte*>(malloc(png_get_rowbytes(png_ptr, info_ptr)));
	}
	png_read_image(png_ptr, row_pointers);
	fclose(fp);
	
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
}

// This whole function just was copy-pasted from niw's GitHub.
// https://gist.github.com/niw/5963798
void Image::write(const std::string& name)
{
	FILE *fp = fopen(name.c_str(), "wb");
	if(!fp) abort();

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png) abort();

	png_infop info = png_create_info_struct(png);
	if (!info) abort();

	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	// Output is 8bit depth, RGBA format.
	png_set_IHDR(
		png,
		info,
		width, height,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);

	if (!row_pointers) abort();

	png_write_image(png, row_pointers);
	png_write_end(png, NULL);
	fclose(fp);

	png_destroy_write_struct(&png, &info);
}

void Image::get_cell(int X, int Y, Cell& cell)
{
	if (cell.figure.size() != sym_width * sym_height) cell.figure.resize(sym_width * sym_height);
	if ((X > width/sym_width) || (Y > height/sym_height)) error_exit("Out of image range.");

	cell.type = 0;
	cell.pixels.resize(16, 0);
	for(int y=0; y < sym_height; y++)
	{
		for(int x=0; x < sym_width; x++)
		{
			// Image colours already supposed to be 4-bit by that moment.
			cell.figure[y*sym_width + x] = get_colour(X*sym_width + x, Y*sym_height + y);
			
			for(int i=0; i<16; i++)
			{
				if (cell.figure[y*sym_width + x] == palette[i])
				{
					cell.pixels[i]++;
					break;
				}
			}
		}
	}
	int max_idx = std::max_element(cell.pixels.begin(), cell.pixels.end()) - cell.pixels.begin();
	cell.colour = palette[max_idx];

	// Dark with sparkle.
	if ((cell.colour.max_delta(palette[0]) < 48) && (92 > cell.pixels[max_idx]))
	{
		cell.type = 1;
		
		// Finding sparkle.
		int second_max = 0, second_idx = 0;
		for(int i=0; i<16; i++)
		{
			if (i == max_idx) continue;
			if (cell.pixels[i] > second_max)
			{
				second_idx = i;
				second_max = cell.pixels[i];
			}
		}
		if (palette[second_idx].max_delta(palette[0]) > 64) cell.colour = palette[second_idx];
		else cell.colour = palette[max_idx];
	}
	// Uniform colour.
	else if (cell.pixels[max_idx] > 80) cell.type = 2;
	// Dual colour or border and mess.
	else if (cell.pixels[max_idx] > 36)
	{
		int second_idx=0, max=0;
		for(int i=0; i<16; i++)
		{
			if (cell.pixels[i] > max)
			{
				second_idx = i;
				max = cell.pixels[i];
			}
		}
		if (cell.pixels[second_idx] > 36) cell.type = 3;
		else cell.type = 4;
	}
	// Some mess.
	else cell.type = 5;
}

void Image::perform_dithering(const Colour* available_cols, int size)
{
	// For Jarvis, Judice, Ninke algorithm.
	double dithering_matrix[3][5] = 
	{
		{     0,      0,      0, 7.0/48, 5.0/48},
		{3.0/48, 5.0/48, 7.0/48, 5.0/48, 3.0/48},
		{1.0/48, 3.0/48, 5.0/48, 3.0/48, 1.0/48}
	};

	// Allocating memory for error matrix.
	double** error_matrix = new double*[height];
	for(int i=0; i<height; i++) error_matrix[i] = new double[width*3];
	
	for(int y=0; y<height; y++) for(int x=0; x<width*3; x++) error_matrix[y][x] = 0;
	
	for(int y=0; y<height; y++)
	{
		for(int x=0; x<width; x++)
		{
			double* err = &error_matrix[y][x*3];
			png_bytep ptr = &row_pointers[y][x*4];
			
			// Finding idx for the closest console colour (with error considered).
			int minimum = 512, idx = 0;
			for(int i=0; i<size; i++)
			{
				int dr =  *ptr    +  *err    - available_cols[i].r;
				int dg = *(ptr+1) + *(err+1) - available_cols[i].g;
				int db = *(ptr+2) + *(err+2) - available_cols[i].b;
				
				int delta = static_cast<int>(sqrt(dr*dr + dg*dg + db*db));
				if (delta < minimum)
				{
					minimum = delta;
					idx = i;
				}
			}
			Colour col = available_cols[idx];
			
			int err_r =  *ptr    +  *err    - col.r;
			int err_g = *(ptr+1) + *(err+1) - col.g;
			int err_b = *(ptr+2) + *(err+2) - col.b;
			
			*ptr = col.r; *(ptr+1) = col.g; *(ptr+2) = col.b;
			
			for(int j=0; j<3; j++)
			{
				for(int i=0; i<5; i++)
				{
					if (((x + i-2) < 0) || ((x + i-2) >= width) || ((y + j) >= height)) continue;
					if ((j==0) && ((i==0)||(i==1)||(i==2))) continue;
					
					error_matrix[y + j][x*3   + (i-2)*3] += dithering_matrix[j][i] * err_r;
					error_matrix[y + j][x*3+1 + (i-2)*3] += dithering_matrix[j][i] * err_g;
					error_matrix[y + j][x*3+2 + (i-2)*3] += dithering_matrix[j][i] * err_b;
				}
			}
		}
	}
	for(int i=0; i<height; i++) delete error_matrix[i];
	delete error_matrix;
}

void Image::reduce_colours()
{
	for(int y=0; y<height; y++)
	{
		for(int x=0; x<width; x++)
		{
			Colour input_col = get_colour(x, y);
			Colour output_col = {0, 0, 0};
			
			int minimum = 512;
			for(int i=0; i<16; i++)
			{
				int delta = input_col.eucl_delta(palette[i]);
				
				if (delta < minimum)
				{
					minimum = delta;
					output_col = palette[i];
				}
			}
			
			png_bytep ptr = &row_pointers[y][x*4];
			*ptr = output_col.r, *(ptr+1) = output_col.g, *(ptr+2) = output_col.b;
		}
	}
}

Image& Image::Palette(const Colour* value)
{
	for(int i=0; i<16; i++) palette[i] = value[i];
	return *this;
}

int Image::Width()  const {return width;}
int Image::Height() const {return height;}

Image::~Image()
{
	for(int y=0; y<height; y++) free(row_pointers[y]);
	free(row_pointers);
}

Colour Image::get_colour(int x, int y) const
{
	png_bytep ptr = &row_pointers[y][x*4];
	Colour col = {*ptr, *(ptr+1), *(ptr+2)};
	return col;
}

//--- --- --- --- --- --- --- --- ---

void Cell::print(const Colour* palette) const
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	for(int i=0; i < sym_width*sym_height; i++)
	{
		char s, c = 0;
		if (figure[i] == colour) s = '#';
		else s = '.';
		
		for(int j=0; j<16; j++)
		{
			if (figure[i] == palette[j]) c = j;
		}
		
		SetConsoleTextAttribute(hOut, (WORD) ((0 << 4) | c));
		std::cout << s;
		if (i%8 == 7) std::cout << std::endl;
	}
	SetConsoleTextAttribute(hOut, (WORD) ((0 << 4) | 8));
	
	std::cout << "Type: " << type << std::endl;
	colour.print();
	
	for(int i=0; i<16; i++)
	{
		std::cout << "i: " << i << " count: " << pixels[i] << std::endl;
	}
}

//--- --- --- --- --- --- --- --- ---

int Colour::eucl_delta(Colour x) const
{
	// Unsigned can't be negative, so we need to subtract in right order.
	int dr = (x.r > r) ? x.r-r:r-x.r;
	int dg = (x.g > g) ? x.g-g:g-x.g;
	int db = (x.b > b) ? x.b-b:b-x.b;
	return static_cast<int>(sqrt(dr*dr + dg*dg + db*db));
}

int Colour::max_delta(Colour x) const
{
	int dr = (x.r > r) ? x.r-r:r-x.r;
	int dg = (x.g > g) ? x.g-g:g-x.g;
	int db = (x.b > b) ? x.b-b:b-x.b;
	
	int max = (dr >= db) ? dr:db;
	max = (max >= dg) ? max:dg;
	return max;
}

int Colour::internal_delta() const
{
	int drg = (r > g) ? r-g:g-r;
	int drb = (r > b) ? r-b:b-r;
	int dgb = (g > b) ? g-b:b-g;
	
	int max = (drg >= drb) ? drg:drb;
	max = (max >= dgb) ? max:dgb;
	return max;
}

bool operator == (const Colour& l, const Colour& r)
{
    return (l.r==r.r && l.g==r.g && l.b==r.b);
}

void Colour::print() const
{
	std::cout << "RGB: " << (int)r << " " << (int)g << " " << (int)b << std::endl;
}

//--- --- --- --- --- --- --- --- ---

void error_exit(const std::string& str)
{
	std::cerr << str;
	exit(-1);
}