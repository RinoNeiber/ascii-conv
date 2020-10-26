#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <windows.h>
#include "Genetic.h"
#include "Drawing.h"

// This is a part of original getCPUTime() for Windows
/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */
static double getCPUTime();

void Individual::fitness(const Colour* image, const bool* pixel_is_free, int sensitivity, int size)
{
	effectiveness = 0;
	for(int i=0; i<size; i++)
	{
		if (!pixel_is_free[i]) continue;
		
		// Rough test firstly.
		if (chromosome.max_delta(image[i]) > sensitivity) continue;
		
		if (chromosome.eucl_delta(image[i]) < sensitivity)
		{
			effectiveness++;
		}
	}
}

//--- --- --- --- --- --- --- --- ---

void Genetic::compute(const Image* image, Colour* palette, int offset, int pop_size, int sensitivity)
{
	srand(time(0));
	
	int size = image->Width() * image->Height();
	cli::draw_borders();
	
	// For block pixels whose colors are already selected.
	bool* pixel_map = new bool[size];
	
	// Buffer for more optimal colour access.
	Colour* colour_map = new Colour[size];
	
	int locked_pixels = 0;
	for(int i=0; i<size; i++)
	{
		pixel_map[i] = true;
		colour_map[i] = image->get_colour(i % image->Width(), i / image->Width());
		
		// Blocking colours that's already in palette.
		for(int j=0; j<offset; j++)
		{
			if (palette[j].eucl_delta(colour_map[i]) < sensitivity)
			{
				pixel_map[i] = false;
				locked_pixels++;
				break;
			}
		}
	}
	for(int i=0; i<offset; i++) cli::update_colour(palette[i], i, 0, size-locked_pixels);
	
	population.resize(pop_size);
	for(int k=offset; k<16; k++)
	{
		// For update_colour().
		double time = getCPUTime();

		// Uniform distribution in colour cube.
		int root = static_cast<int>(pow(pop_size, 1.0/3)) + 1;
		for(int i=0; i<pop_size; i++)
		{
			int step = 256 / root;
			population[i].chromosome = 
			{
				(unsigned char)( i / (root*root) * step),          // Z.
				(unsigned char)((i % (root*root)) / root * step),  // Y.
				(unsigned char)((i%root) * step)                   // X.
			};
		}

		int age = 0;
		int old_eff = 0;
		mut_probability = 0.1;
		
		// Major loop.
		while(true)
		{
			// For update_main_table().
			double time = getCPUTime();
			
			for(int i=0; i<pop_size; i++)
			{
				// For update_elipsis().
				double time = getCPUTime();
				
				// Colours in palette shouldn't be too close to each other.
				bool colour_is_allowed = true;
				for(int j=0; j<k; j++)
				{
					if (population[i].chromosome.eucl_delta(palette[j]) < 1.5*sensitivity) 
					{
						colour_is_allowed = false;
						break;
					}
				}
				if (colour_is_allowed) population[i].fitness(colour_map, pixel_map, sensitivity, size);
				else population[i].effectiveness = 0;

				time = getCPUTime() - time;
				cli::update_elipsis(time);
			}
			std::sort(population.begin(), population.end());

			time = getCPUTime() - time;
			cli::update_main_table(age, time, population.back().effectiveness, population.back().chromosome, k);
			
			if ((population.back().effectiveness == old_eff)) mut_probability += 0.2;
			else mut_probability = 0.1;
			old_eff = population.back().effectiveness;
			
			if (age == 25) break;
			if (mut_probability > 0.7) break;
			
			// Just some crazy system of parents selection.
			// 33, 12, 8, 4, 3, -(60)-> 1 * 40.
			int mul = pop_size/100;
			
			std::vector<Individual> happy_parents(90);
			for(int i=0; i<90; i++) happy_parents[i] = population[pop_size-1 - i];

			for(int i=0; i<33*mul; i++) crossover(happy_parents[0], happy_parents[1], i);
			for(int i=0; i<12*mul; i++) crossover(happy_parents[2], happy_parents[3], 33*mul + i);
			for(int i=0; i< 8*mul; i++) crossover(happy_parents[4], happy_parents[5], 45*mul + i);
			for(int i=0; i< 4*mul; i++) crossover(happy_parents[6], happy_parents[7], 53*mul + i);
			for(int i=0; i< 3*mul; i++) crossover(happy_parents[8], happy_parents[9], 57*mul + i);

			for(int i=0; i<40;  i++)
			{
				for(int j=0; j<mul; j++)
				{
					crossover(happy_parents[10 + i*2], happy_parents[11 + i*2], (60 + i)*mul + j);
				}
			}

			for(int i=0; i<pop_size; i++) mutation(population[i]);

			age++;
		}
		Individual max = population.back();
		
		// Locking already considered pixels.
		for(int i=0; i<size; i++)
		{
			if (pixel_map[i] == false) continue;
			if (max.chromosome.eucl_delta(colour_map[i]) < sensitivity)
			{
				pixel_map[i] = false;
				locked_pixels++;
			}
		}
		palette[k] = max.chromosome;

		time = getCPUTime() - time;
		cli::update_colour(palette[k], k, time, size-locked_pixels);
	}
	delete pixel_map;
	delete colour_map;
}

void Genetic::crossover(const Individual& left, const Individual& right, int pos)
{
	// For uniform crossover.
	unsigned char mask[3];
	for(int i=0; i<3; i++) mask[i] = rand() % 256;
	
	Colour child_col;
	child_col.r = (left.chromosome.r & mask[0]) + (right.chromosome.r & ~mask[0]);
	child_col.g = (left.chromosome.g & mask[1]) + (right.chromosome.g & ~mask[1]);
	child_col.b = (left.chromosome.b & mask[2]) + (right.chromosome.b & ~mask[2]);
	
	population[pos].chromosome = child_col;
}

void Genetic::mutation(Individual& value)
{
	if ((double)rand() / RAND_MAX < mut_probability)
	{
		value.chromosome.r = rand() % 256;
		value.chromosome.g = rand() % 256;
		value.chromosome.b = rand() % 256;
	}
}

double getCPUTime()
{
	FILETIME createTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;
	if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime) != -1)
	{
		ULARGE_INTEGER li = {{userTime.dwLowDateTime, userTime.dwHighDateTime}};
		return li.QuadPart / 10000.0;  // Milliseconds.
	}
	else return -1;
}
