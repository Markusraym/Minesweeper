#pragma once
#include "TextureManager.h"
#include <SFML/Graphics.hpp>

class Tile
{
public:

	sf::Sprite tile;
	bool hasFlag;
	bool revealed;
	bool hasMine; 
	Tile* adjacentTiles[8];

	Tile();
	void setSprite(const char* name);
	void SetPosition(float x, float y);
	sf::FloatRect GetGlobalBounds() const;
	sf::Sprite GetSprite();
	const sf::Vector2f& GetPosition();
};