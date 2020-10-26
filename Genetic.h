#ifndef GENETIC_H
#define GENETIC_H

#include "Image.h"

struct Individual
{
	Colour chromosome;
	int effectiveness;
	
	void fitness(const Colour*, const bool*, int, int);
	
	bool operator < (const Individual& value) const {return this->effectiveness < value.effectiveness;}
};

class Genetic
{
public:
	void compute(const Image*, Colour*, int, int, int);
private:
	std::vector<Individual> population;
	
	void crossover(const Individual&, const Individual&, int);
	void mutation(Individual&);
	
	int sensitivity;
	int pop_size;
	double mut_probability;
};

#endif  // GENETIC_H