#include "Random.h"
#include <ctime>

//static variables must be redeclared in global space
std::mt19937 Random::random(time(0));

int Random::Int(int min, int max) //Random integers
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(random);
}
float Random::Float(float min, float max) //Random float
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(random);
}

/*
	Random example:
	int value = Random::Int(0, 100); //Getting a random integer number from 0 to 100
	*/