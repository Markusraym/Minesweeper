#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;
using std::unordered_map;
using std::string;

class TextureManager
{
	// static == one and only one of these in memory, ever
	static unordered_map<string, sf::Texture> textures;
	static void LoadTexture(string textureName); // LoadTexture("space");
public:
	static sf::Texture& GetTexture(string textureName); //GetTexture("space");
	static void Clear(); // Call this one at the end of main()
};

