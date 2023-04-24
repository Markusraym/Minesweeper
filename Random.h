#pragma once
#include <random>
class Random
{
	// One instance of the x variable, and all instances SHARE this variable
	static std::mt19937 random; 
public:
	static int Int(int min, int max); //Random integers
	static float Float(float min, float max); //Random float
};



