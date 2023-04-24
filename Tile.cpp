#include "Tile.h"
#include "TextureManager.h"
using namespace std;

Tile::Tile()
{
	setSprite("tile_hidden");
	hasMine = false;
	revealed = false;
	hasFlag = false;
	for (int i = 0; i < 8; i++)
	{
		adjacentTiles[i] = nullptr; 
	}
}

void Tile::setSprite(const char* name)
{
	this->tile.setTexture(TextureManager::GetTexture(name));
}

sf::Sprite Tile::GetSprite()
{
	return tile;
}

void Tile::SetPosition(float x, float y)
{
	tile.setPosition(x, y);
}

//My own version of GetGlobalBounds
sf::FloatRect Tile::GetGlobalBounds() const
{
	return tile.getGlobalBounds(); 
}

const sf::Vector2f& Tile::GetPosition()
{
	return tile.getPosition();
}