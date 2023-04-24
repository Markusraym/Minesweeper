#include "TextureManager.h"

// "redeclare" the variable, so the file knows it exists and can use it
unordered_map<string, sf::Texture> TextureManager::textures;

void TextureManager::LoadTexture(string fileName)
{
	string path = "images/";
	path += fileName + ".png";

	textures[fileName].loadFromFile(path);
}

sf::Texture& TextureManager::GetTexture(string textureName)
{
	//if the texture DOESN'T EXIST... LOAD IT first and then return it
	if (textures.find(textureName) == textures.end())
	{
		LoadTexture(textureName);
	}
	//load it first? then return it? 
	return textures[textureName];
}

void TextureManager::Clear()
{
	textures.clear(); //Get rid of all stored objects
}