#include <iostream>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <vector>
#include "TextureManager.h"
#include <fstream>
#include "Tile.h"
#include "Random.h"
#include <set>

int main()
{
    void PlaceFlag(sf::RenderWindow & window, vector<sf::Sprite> Tiles, int i); //prototype to place the flag
    void changeFlagCount(sf::RenderWindow & window, vector<sf::Sprite>&counters, int flagCount, bool won, int height); //prototype
    void readBrd(string filePath, vector<int>& brdtest, vector<Tile>&Tiles); //prototype for function to read the .brd files
    void Reset(vector <sf::Sprite>&mines, vector <sf::Sprite>&flags, vector <Tile>& Tiles, int& mineCount, vector<sf::Sprite>& numbers, int width, int height); //prototype to reset the game 
    void adjacentTile(vector<Tile>&Tiles, int width, int height); //prototype of function that assigns all the pointer variables of tiles to the correct positions
    void Recursion(Tile & currentTile, vector<sf::Sprite>& numbers, int& victoryCounter); //Recursion function to reveal all nearby tiles that have no adjacent mines

    // Read the information from the config.cfg file 
    int width = 0, height = 0, mineCount = 0;
    ifstream inFile("boards/config.cfg");
    string lineFromFile;
    if (inFile.is_open())
    {
        istringstream stream(lineFromFile);
        inFile >> width;
        inFile >> height;
        inFile >> mineCount;
    } 
    
    inFile.close();
    //Take the height multiplied by width, and then subtract mine count from it. Have a counter for everytime a non mine tile is revealed, add +1. 
    int windowWidth = (width * 32); //Gets the required number of pixels for width
    int windowHeight = ((height * 32) + 100); //Gets the required number of pixels for height

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Minesweeper"); //Making the window
    sf::Event event; 

    vector<Tile> Tiles; //Vector of tiles that originally start out as all hidden tiles
    vector<sf::Sprite> flags; // Vector to store all the flags
    vector<sf::FloatRect> rectanglesTiles; // Vector to store all the rectangles to assign a rectangle to every hidden tile
    vector<int> brdIndexs; // Vector to store the ones and zeros from the .brd files
    vector<sf::Sprite> mines; // Vector to store all the mines
    vector<sf::Sprite> debugMines; //Vector to store all the mines when in debug mode
    vector<sf::Sprite> numbers; //Vector to store all the number sprites
    vector<sf::Sprite> winFlags; 
    int flagCounter = mineCount;
    bool stop = false;
    bool debug = false;
    bool lost = false; 
    bool won = false; 
    int victoryCounter = 0; 
    int victoryNeeded = ((width * height) - mineCount); //This gives the amount of non mine tiles on board, need to reveal all of them to win. 
    int mineCountcopy = mineCount;
 
    //Make a board with 400 tiles and pushing them into a vector 
    for (int rows = 0; rows < height; rows++)
    {
        for (int column = 0; column < width; column++)
        {
            Tile tile;
            tile.SetPosition(column * 32, rows * 32);
            Tiles.push_back(tile);
        }
    }

    //Randomly assigning tiles to have mines
    vector<int> randomvector; 
    set<int> random;
    int count = 0;
    while (count < mineCount) //this whole while loop prevents duplicate numbers from being made
    {
            int value;
            value = Random::Int(0, ((height * width) - 1) );
            if (random.find(value) == random.end())
            {
                random.insert(value);
                randomvector.push_back(value);
                count++;
            }
    }
    for (int i = 0; i < randomvector.size(); i++)
    {
        Tiles[randomvector[i]].hasMine = true;
    }

    //Some code to have a rectangle over every hiddenTile sprite so I can check if the rectangle contains the mouse
    for (int i = 0; i < Tiles.size(); i++) // For loop to assign a rectangle to every hiddenTile
    {
        sf::FloatRect rectangleHiddenTile = Tiles[i].GetGlobalBounds(); 
        rectanglesTiles.push_back(rectangleHiddenTile);
    }

    //Load the counter image outside of the while loop, have it set to the mineCount, and then constantly update it inside of the while loop 
    vector <sf::Sprite> counters;
    //Load 10 different numbers, 0 through 9, and store them into a vector for later use
    for (int i = 0; i <= 10; i++)
    {
        sf::Sprite counterSprite(TextureManager::GetTexture("digits"));
        sf::FloatRect counterRect = counterSprite.getGlobalBounds();
        counterSprite.setTextureRect(sf::IntRect((i * 21), 0, 21, 32));
        counterSprite.setPosition(0, 512);
        counters.push_back(counterSprite);
    }

    //Load the miscelaneous images in the bottom right
    sf::Sprite happyFaceSprite(TextureManager::GetTexture("face_happy"));
    sf::Sprite debugSprite(TextureManager::GetTexture("debug"));
    sf::Sprite test1Sprite(TextureManager::GetTexture("test_1"));
    sf::Sprite test2Sprite(TextureManager::GetTexture("test_2"));
    sf::Sprite test3Sprite(TextureManager::GetTexture("test_3"));
    //Set the positons of the images
    happyFaceSprite.setPosition(((width * 32)/2) - 32, (height * 32));
    debugSprite.setPosition(((width * 32) - 256), (height * 32));
    test1Sprite.setPosition(((width * 32) - 192), (height * 32));
    test2Sprite.setPosition(((width * 32) - 128), (height * 32));
    test3Sprite.setPosition(((width * 32) - 64), (height * 32));

    //Rectangles for buttons 
    sf::FloatRect test1Rect = test1Sprite.getGlobalBounds();
    sf::FloatRect test2Rect = test2Sprite.getGlobalBounds();
    sf::FloatRect test3Rect = test3Sprite.getGlobalBounds();
    sf::FloatRect debugRect = debugSprite.getGlobalBounds();
    sf::FloatRect happyRect = happyFaceSprite.getGlobalBounds();

    //Call adjcantTile, a function which sets every Tile's adjacentMine array's pointers. 
    adjacentTile(Tiles, width, height);
      
    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {

            if (event.type == sf::Event::Closed)
                window.close();

            sf::Vector2i mousePosition = sf::Mouse::getPosition(window); // Variable to keep track of the mouse's position

            vector<sf::FloatRect> rectanglesFlags; // Vector to store all the rectangles for flags
            for (int i = 0; i < flags.size(); i++) // For loop to assign a rectangle to every flag (so I can deactivate a flag and such)
            {
                sf::FloatRect rectangleFlag = flags[i].getGlobalBounds();
                rectanglesFlags.push_back(rectangleFlag);
            }

            if (event.type == sf::Event::MouseButtonPressed)
            {

                //if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) //If you left click
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    //If you left click a tile to reveal it
                    for (int i = 0; i < rectanglesTiles.size(); i++)
                    {
                        if ((rectanglesTiles[i].contains(mousePosition.x, mousePosition.y)) && (lost == false) && (Tiles[i].revealed == false) && (Tiles[i].hasMine == false) && (Tiles[i].hasFlag == false) && (won == false))
                        {
                            Recursion(Tiles[i], numbers, victoryCounter);
                        }
                        //when you left click, check if hasMine == true. If true, turn the "tile_hidden" to "tile_revealed" with "mine" above it.
                        if (((rectanglesTiles[i].contains(mousePosition.x, mousePosition.y)) && Tiles[i].hasMine == true) && (lost == false) && (Tiles[i].hasFlag == false) && (won == false))
                        {
                            Tiles[i].setSprite("tile_revealed");
                            lost = true;
                            happyFaceSprite.setTexture(TextureManager::GetTexture("face_lose"));
                        }
                    }
                    //If user clicks on test button 1
                    if (test1Rect.contains(mousePosition.x, mousePosition.y))
                    {
                        happyFaceSprite.setTexture(TextureManager::GetTexture("face_happy")); //Once you win or lose a game, and click on a test board, it'll reset the face to default. 
                        mineCount = 1; //Easier to just count how many mines are in each test board so the win condition is accurate 
                        victoryNeeded = ((width * height) - mineCount); //Reset the victoryNeeded (amount of tiles to reveal to win) with the new mineCount number
                        victoryCounter = 0;
                        flagCounter = mineCount; //Set the flag counter to the board's mine count
                        won = false;
                        flags.clear();
                        winFlags.clear();
                        brdIndexs.clear();
                        numbers.clear();
                        mines.clear();
                        readBrd("testboard1", brdIndexs, Tiles);
                        for (int i = 0; i < Tiles.size(); i++)
                        {
                            Tiles[i].setSprite("tile_hidden");
                            Tiles[i].revealed = false;
                        }
                    }
                    //If user clicks on test button 2
                    if (test2Rect.contains(mousePosition.x, mousePosition.y))
                    {
                        happyFaceSprite.setTexture(TextureManager::GetTexture("face_happy")); //Once you win or lose a game, and click on a test board, it'll reset the face to default. 
                        mineCount = 102; //Easier to just count how many mines are in each test board so the win condition is accurate 
                        victoryNeeded = ((width * height) - mineCount); //Reset the victoryNeeded (amount of tiles to reveal to win) with the new mineCount number
                        victoryCounter = 0;
                        flagCounter = mineCount; //Set the flag counter to the board's mine count
                        won = false;
                        winFlags.clear();
                        flags.clear();
                        brdIndexs.clear();
                        numbers.clear();
                        mines.clear();
                        readBrd("testboard2", brdIndexs, Tiles);
                        for (int i = 0; i < Tiles.size(); i++)
                        {
                            Tiles[i].setSprite("tile_hidden");
                            Tiles[i].revealed = false;
                        }

                    }
                    //If user clicks on test button 3
                    if (test3Rect.contains(mousePosition.x, mousePosition.y))
                    {
                        happyFaceSprite.setTexture(TextureManager::GetTexture("face_happy")); //Once you win or lose a game, and click on a test board, it'll reset the face to default. 
                        mineCount = 83; //Easier to just count how many mines are in each test board so the win condition is accurate 
                        victoryNeeded = ((width * height) - mineCount);  //Reset the victoryNeeded (amount of tiles to reveal to win) with the new mineCount number
                        flagCounter = mineCount; //Set the flag counter to the board's mine count
                        victoryCounter = 0;
                        won = false;
                        winFlags.clear();
                        flags.clear();
                        brdIndexs.clear();
                        numbers.clear();
                        mines.clear();
                        readBrd("testboard3", brdIndexs, Tiles);
                        for (int i = 0; i < Tiles.size(); i++)
                        {
                            Tiles[i].setSprite("tile_hidden");
                            Tiles[i].revealed = false;
                        }
                    }
                    //If user clicks on debug button, itll set debug = true and itll draw the mines differently at the bottom of the main while loop 
                    if ((debugRect.contains(mousePosition.x, mousePosition.y)) && (debug == false))
                    {
                        debug = true;
                    }
                    //Allow the user to turn off debug mode 
                    else if ((debugRect.contains(mousePosition.x, mousePosition.y)) && (debug == true))
                    {
                        debug = false;
                    }
                    //If the user clicks on the happy face to reset the board
                    if (happyRect.contains(mousePosition.x, mousePosition.y))
                    {
                        //Reset the mines, reset the tiles textures, reset the victoryCounter, and reset the flag count.
                        flagCounter = mineCountcopy;
                        winFlags.clear();
                        happyFaceSprite.setTexture(TextureManager::GetTexture("face_happy"));
                        Reset(mines, flags, Tiles, mineCountcopy, numbers, width, height);
                        victoryCounter = 0;
                        lost = false;
                        won = false;
                    }
                }
                 if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) //If you right click to place a flag
                {
                    for (int i = 0; i < rectanglesTiles.size(); i++)
                    {
                        //if rectanglesTiles[i] contains mouse AND Tiles[i] hasFlag == false
                        if ((rectanglesTiles[i].contains(mousePosition.x, mousePosition.y)) && (Tiles[i].hasFlag == false) && (won == false))
                        {
                            // Make a flag sprite 
                            sf::Sprite flagSprite(TextureManager::GetTexture("flag"));
                            // Put the flag sprite in the same position as the rectanglesTiles[i]
                            flagSprite.setPosition(Tiles[i].GetPosition());
                            // Put into vector 
                            flags.push_back(flagSprite);
                            flagCounter--;
                            Tiles[i].hasFlag = true;
                        }
                        //if rectangleTiles[i].contains the mouse AND there is a flag already in that spot, remove it 
                        else if ((rectanglesTiles[i].contains(mousePosition.x, mousePosition.y)) && (Tiles[i].hasFlag == true) && (won == false))
                        {
                            //for loop to find the flag and to remove it
                            for (int j = 0; j < flags.size(); j++)
                            {
                                if (flags[j].getPosition() == Tiles[i].GetPosition())
                                {
                                    flags.erase(flags.begin() + j);
                                }
                            }
                            flagCounter++;
                            Tiles[i].hasFlag = false;
                        }
                    }
                }
            }
            if (victoryCounter == victoryNeeded) //You revealed all tiles! You win!
            {
                happyFaceSprite.setTexture(TextureManager::GetTexture("face_win"));
                won = true; //It sets lost to true so you cant click on anything else other than the reset button 
                victoryCounter = 0;
                //flagcount = 0?
                //For victory, all tiles with mines have flags placed on top of eachother
                //For lost, all tiles with mines are revealed   
            }
        }
            window.clear();
            //Draw the sprites 
            //This is if the player won, flags need to be placed on top of all mines
            if ((debug != true) && (lost != true) && (won == true))
            {
                for (unsigned int i = 0; i < Tiles.size(); i++)
                {
                    window.draw(Tiles[i].GetSprite());
                }
                for (unsigned int i = 0; i < flags.size(); i++)
                {
                    window.draw(flags[i]);
                }
                for (unsigned int i = 0; i < mines.size(); i++)
                {
                    window.draw(mines[i]);
                }
                for (unsigned int i = 0; i < Tiles.size(); i++)
                {
                    if (Tiles[i].hasMine == true)
                    {
                        sf::Sprite winFlag(TextureManager::GetTexture("flag"));
                        winFlag.setPosition(Tiles[i].GetPosition());
                        winFlags.push_back(winFlag);

                    }
                }
                for (unsigned int i = 0; i < numbers.size(); i++)
                {
                    window.draw(numbers[i]);
                }
                for (unsigned int i = 0; i < winFlags.size(); i++)
                {
                    window.draw(winFlags[i]);
                }
            }
            else if ((debug == true) || (lost == true)) //Set the textures underneath mines to revealed if you lost, or just reveal mines if debug is true. 
            {
                debugMines.clear();
                for (unsigned int i = 0; i < Tiles.size(); i++)
                {
                    if (lost == true)
                    {
                        if (Tiles[i].hasMine == true)
                        {
                            Tiles[i].setSprite("tile_revealed");
                        }  
                    }
                    window.draw(Tiles[i].GetSprite());
                }
                for (unsigned int i = 0; i < flags.size(); i++)
                {
                    window.draw(flags[i]);
                }
                for (unsigned int i = 0; i < Tiles.size(); i++)
                {
                    if (Tiles[i].hasMine == true)
                    {
                        sf::Sprite debugMine(TextureManager::GetTexture("mine"));
                        debugMine.setPosition(Tiles[i].GetPosition());
                        window.draw(debugMine);
                        debugMines.push_back(debugMine);
                    }
                }
                for (unsigned int i = 0; i < numbers.size(); i++)
                {
                    window.draw(numbers[i]);
                }
            }
            else //If the game isn't in debug mode or if you haven't lost
            {
                for (unsigned int i = 0; i < Tiles.size(); i++)
                {
                    window.draw(Tiles[i].GetSprite());
                }
                for (unsigned int i = 0; i < flags.size(); i++)
                {
                    window.draw(flags[i]);
                }
                for (unsigned int i = 0; i < mines.size(); i++)
                {
                    window.draw(mines[i]);
                }
                for (unsigned int i = 0; i < numbers.size(); i++)
                {
                    window.draw(numbers[i]);
                }
            }

        window.draw(happyFaceSprite);
        window.draw(debugSprite);
        window.draw(test1Sprite);
        window.draw(test2Sprite);
        window.draw(test3Sprite);

        //Update the flag count at every loop 
        changeFlagCount(window, counters, flagCounter, won, height);
        
        //Display
        window.display();
    }
    TextureManager::Clear();
    return 0;
}

void changeFlagCount(sf::RenderWindow& window, vector<sf::Sprite>& counters, int flagCount, bool won, int height) //Function constantly updates the flag count in the bottom left corner of game
{
    //First digit: flagCount / 10 
    //Second digit: flagCount % 10

    if (won == true)
    {
        flagCount = 0;
    }

    for (int i = 0; i < 10; i++)
    {
        if ((flagCount >= 0) && (flagCount < 100))
        {
            counters[0].setPosition(0, (height * 32));
            window.draw(counters[0]);

            if (flagCount / 10 == i)
            {
                counters[i].setPosition(21, (height * 32));
                window.draw(counters[i]);
            }
            if (flagCount % 10 == i)
            {
                counters[i].setPosition(42, (height * 32));
                window.draw(counters[i]);
            }
        }
        if (flagCount >= 100)
        {
            if (((flagCount / 10) / 10) == i)
            {
                counters[i].setPosition(0, (height * 32));
                window.draw(counters[i]);
            }
            if ((flagCount / 10) % 10 == i)
            {
                counters[i].setPosition(21, (height * 32));
                window.draw(counters[i]);
            }
            if (flagCount % 10 == i)
            {
                counters[i].setPosition(42, (height * 32));
                window.draw(counters[i]);
            }
        }
        else if (flagCount < 0)
        {
            int negativeCount;
            negativeCount = abs(flagCount);
            // Negative sign
            counters[10].setPosition(0, (height * 32));
            window.draw(counters[10]);
            //Zero sign
            counters[0].setPosition(21, (height * 32));
            window.draw(counters[0]);
            // Tens place 
            if (negativeCount == i)
            {
                counters[i].setPosition(42, (height * 32));
                window.draw(counters[i]);
            }
        }
    }
}

void readBrd(string filePath, vector<int>& brdtest, vector<Tile>& Tiles) //Function reads .cfg files which consist of 1's and 0's, and sets up a board where the 1's are mines and 0's are not. 
{
    //Somehow reload the board, and have an if loop saying if = 1, hasMine = true. 
    //Reset the board so that there are no mines prior 
    for (int i = 0; i < Tiles.size(); i++)
    {
        Tiles[i].hasMine = false; 
    }
    
    string path = "boards/";
    path += filePath + ".brd";
    ifstream inFile(path);
    string lineFromFile;
    if (inFile.is_open())
    {
        while (getline(inFile, lineFromFile))
        {
            istringstream stream(lineFromFile);
            string line;
            getline(stream, line, ' ');
            for (int i = 0; i < line.size(); i++)
            {
                int index = line[i] - '0';
                brdtest.push_back(index);
            }
        }
        for (int i = 0; i < 400; i++)
        {
            if (brdtest[i] == 1)
            {
                Tiles[i].hasMine = true;
            }
        }
    }
    inFile.close();
}

void Reset(vector <sf::Sprite>& mines, vector <sf::Sprite>& flags, vector <Tile>& Tiles, int& mineCount, vector <sf::Sprite>& numbers, int width, int height) //Function resets the game board
{
    //Resets the mines, resets the tiles textures, resets the numbers vector, resets the flags, and resets the flag count.
    numbers.clear();
    mines.clear();
    flags.clear();
    for (int i = 0; i < Tiles.size(); i++)
    {
        Tiles[i].setSprite("tile_hidden");
    }
    for (int i = 0; i < Tiles.size(); i++)
    {
        Tiles[i].hasMine = false;
        Tiles[i].revealed = false; 
    }

    //Reset the mines with random
    vector<int> randomvector;
    set<int> random;
    int count = 0;
    while (count < mineCount)
    {
        int value;
        value = Random::Int(0, ((width * height) - 1) );
        if (random.find(value) == random.end())
        {
            random.insert(value);
            randomvector.push_back(value);
            count++;
        }
    }
    for (int i = 0; i < randomvector.size(); i++)
    {
        Tiles[randomvector[i]].hasMine = true;
    }

}


void Recursion(Tile& currentTile, vector <sf::Sprite>& numbers, int& victoryCounter) //This function reveals tiles when clicked, and then checks adjacent tiles for mines and if theres no mines, it reveals them too
{
    currentTile.setSprite("tile_revealed");
    //When the tile is clicked, iterate through its adjacent mines and see if each one is pointing towards a mine
    int adjacentMines = 0;
    for (int j = 0; j < 8; j++)
    {
        if (currentTile.adjacentTiles[j] != nullptr)
        {
            if (currentTile.adjacentTiles[j]->hasMine == true) //If there is an adjacent mine, adjacentMine is incremented by one
            {
                adjacentMines++;
            }
        }
    }
    sf::Sprite numberSprite;  //Make a sprite with no texture, and then assign it's texture determined by adjacentMine's count
    numberSprite.setPosition(currentTile.GetPosition());

    if (adjacentMines == 1)
    {
        numberSprite.setTexture(TextureManager::GetTexture("number_1"));
    }
    else if (adjacentMines == 2)
    {
        numberSprite.setTexture(TextureManager::GetTexture("number_2"));
    }
    else if (adjacentMines == 3)
    {
        numberSprite.setTexture(TextureManager::GetTexture("number_3"));
    }
    else if (adjacentMines == 4)
    {
        numberSprite.setTexture(TextureManager::GetTexture("number_4"));
    }
    else if (adjacentMines == 5)
    {
        numberSprite.setTexture(TextureManager::GetTexture("number_5"));
    }
    else if (adjacentMines == 6)
    {
        numberSprite.setTexture(TextureManager::GetTexture("number_6"));
    }
    else if (adjacentMines == 7)
    {
        numberSprite.setTexture(TextureManager::GetTexture("number_7"));
    }
    else if (adjacentMines == 8)
    {
        numberSprite.setTexture(TextureManager::GetTexture("number_8"));
    }
    numbers.push_back(numberSprite);
    
    currentTile.revealed = true;
    victoryCounter++; 

    if (adjacentMines == 0) //If there is no adjacent mines
    {
        for (int i = 0; i < 8; i++) //It searches through its adjacent tiles and if any have a mine count, it draws their sprite numbers
        {
            if ((currentTile.adjacentTiles[i] != nullptr) && (currentTile.adjacentTiles[i]->revealed == false))
            {
                Recursion(*currentTile.adjacentTiles[i], numbers, victoryCounter); //Recursion! Ew. 
            }
        }
    }
}


void adjacentTile(vector <Tile>& Tiles, int width, int height) //function which sets every Tile's adjacentMine array's pointers.
//Theres so much repeated code because I have to make it work with different heights. I'm sure there is a better way but I have to get this done in time. 
{
    for (int i = 0; i < Tiles.size(); i++)
    {
        /*===== FIRST ROW =====*/
        if (i == 0) //The first tile on the first row
        {
            Tiles[i].adjacentTiles[0] = nullptr;
            Tiles[i].adjacentTiles[1] = nullptr;
            Tiles[i].adjacentTiles[2] = nullptr;
            Tiles[i].adjacentTiles[3] = nullptr;
            Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
            Tiles[i].adjacentTiles[5] = &Tiles[width + 1];
            Tiles[i].adjacentTiles[6] = &Tiles[i + width];
            Tiles[i].adjacentTiles[7] = nullptr;

        }
        if ((i > 0) && (i < width - 1)) //Inbetween the first tile and the last tiel on first row 
        {
            Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
            Tiles[i].adjacentTiles[1] = nullptr;
            Tiles[i].adjacentTiles[2] = nullptr;
            Tiles[i].adjacentTiles[3] = nullptr;
            Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
            Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
            Tiles[i].adjacentTiles[6] = &Tiles[i + width];
            Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
        }
        if (i == width - 1) //The last tile on the first row, index 24 
        {
            Tiles[i].adjacentTiles[0] = &Tiles[i - 1]; //Index 23
            Tiles[i].adjacentTiles[1] = nullptr;
            Tiles[i].adjacentTiles[2] = nullptr;
            Tiles[i].adjacentTiles[3] = nullptr;
            Tiles[i].adjacentTiles[4] = nullptr;
            Tiles[i].adjacentTiles[5] = nullptr;
            Tiles[i].adjacentTiles[6] = &Tiles[width + i]; //Index 49
            Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1]; //Index 48
        }

        if (height == 16)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 15 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 15 ====*/ 
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 15 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1]; 
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1]; 
            }
        }

        if (height == 17)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 16 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14) || (i == width * 15))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1]; 
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 16 ====*/ 
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)) || ((i < ((width * 16) - 1)) && (i > width * 15)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 16 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)) || (i == ((width * 16) - 1))) 
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1]; 
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1]; 
            }
        }

        if (height == 18)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 17 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14) || (i == width * 15) || (i == width * 16))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 17 ====*/ 
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)) || ((i < ((width * 16) - 1)) && (i > width * 15)) ||
                ((i < ((width * 17) - 1)) && (i > width * 16)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 17 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)) || (i == ((width * 16) - 1)) || (i == ((width * 17) - 1))) //i == width * 16 - 1
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1]; 
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1]; 
            }
        }

        if (height == 19)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 18 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14) || (i == width * 15) || (i == width * 16) ||
                (i == width * 17))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 18 ====*/ 
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)) || ((i < ((width * 16) - 1)) && (i > width * 15)) ||
                ((i < ((width * 17) - 1)) && (i > width * 16)) || ((i < ((width * 18) - 1)) && (i > width * 17)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 18 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)) || (i == ((width * 16) - 1)) || (i == ((width * 17) - 1)) || (i == ((width * 18) - 1))) //i == width * 16 - 1
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1]; 
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1]; 
            }
        }

        if (height == 20)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 19 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14) || (i == width * 15) || (i == width * 16) ||
                (i == width * 17) || (i == width * 18))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1]; 
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 19 ====*/ 
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)) || ((i < ((width * 16) - 1)) && (i > width * 15)) ||
                ((i < ((width * 17) - 1)) && (i > width * 16)) || ((i < ((width * 18) - 1)) && (i > width * 17)) || ((i < ((width * 19) - 1)) && (i > width * 18)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 19 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)) || (i == ((width * 16) - 1)) || (i == ((width * 17) - 1)) || (i == ((width * 18) - 1)) || (i == ((width * 19) - 1))) //i == width * 16 - 1
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1]; 
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];  
            }
        }

        if (height == 21)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 19 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14) || (i == width * 15) || (i == width * 16) ||
                (i == width * 17) || (i == width * 18) || (i == width * 19))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1]; 
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 20 ====*/ 
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)) || ((i < ((width * 16) - 1)) && (i > width * 15)) ||
                ((i < ((width * 17) - 1)) && (i > width * 16)) || ((i < ((width * 18) - 1)) && (i > width * 17)) || ((i < ((width * 19) - 1)) && (i > width * 18)) ||
                ((i < ((width * 20) - 1)) && (i > width * 19)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 20 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)) || (i == ((width * 16) - 1)) || (i == ((width * 17) - 1)) || (i == ((width * 18) - 1)) || (i == ((width * 19) - 1))
                || (i == ((width * 20) - 1))) 
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1]; 
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }
        }

        if (height == 22)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 21 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14) || (i == width * 15) || (i == width * 16) ||
                (i == width * 17) || (i == width * 18) || (i == width * 19) || (i == width * 20))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 21 ====*/ 
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)) || ((i < ((width * 16) - 1)) && (i > width * 15)) ||
                ((i < ((width * 17) - 1)) && (i > width * 16)) || ((i < ((width * 18) - 1)) && (i > width * 17)) || ((i < ((width * 19) - 1)) && (i > width * 18)) ||
                ((i < ((width * 20) - 1)) && (i > width * 19)) || ((i < ((width * 21) - 1)) && (i > width * 20)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 21 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)) || (i == ((width * 16) - 1)) || (i == ((width * 17) - 1)) || (i == ((width * 18) - 1)) || (i == ((width * 19) - 1))
                || (i == ((width * 20) - 1)) || (i == ((width * 21) - 1))) //i == width * 16 - 1
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1]; 
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1]; 
            }
        }

        if (height == 23)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 22 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14) || (i == width * 15) || (i == width * 16) ||
                (i == width * 17) || (i == width * 18) || (i == width * 19) || (i == width * 20) || (i == width * 21))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1]; 
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 22 ====*/
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)) || ((i < ((width * 16) - 1)) && (i > width * 15)) ||
                ((i < ((width * 17) - 1)) && (i > width * 16)) || ((i < ((width * 18) - 1)) && (i > width * 17)) || ((i < ((width * 19) - 1)) && (i > width * 18)) ||
                ((i < ((width * 20) - 1)) && (i > width * 19)) || ((i < ((width * 21) - 1)) && (i > width * 20)) || ((i < ((width * 22) - 1)) && (i > width * 21)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 22 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)) || (i == ((width * 16) - 1)) || (i == ((width * 17) - 1)) || (i == ((width * 18) - 1)) || (i == ((width * 19) - 1))
                || (i == ((width * 20) - 1)) || (i == ((width * 21) - 1)) || (i == ((width * 22) - 1))) 
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1]; 
            }
        }

        if (height == 24)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 23 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14) || (i == width * 15) || (i == width * 16) ||
                (i == width * 17) || (i == width * 18) || (i == width * 19) || (i == width * 20) || (i == width * 21) || (i == width * 22))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1]; 
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 23 ====*/ 
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)) || ((i < ((width * 16) - 1)) && (i > width * 15)) ||
                ((i < ((width * 17) - 1)) && (i > width * 16)) || ((i < ((width * 18) - 1)) && (i > width * 17)) || ((i < ((width * 19) - 1)) && (i > width * 18)) ||
                ((i < ((width * 20) - 1)) && (i > width * 19)) || ((i < ((width * 21) - 1)) && (i > width * 20)) || ((i < ((width * 22) - 1)) && (i > width * 21)) ||
                ((i < ((width * 23) - 1)) && (i > width * 22)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 23 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)) || (i == ((width * 16) - 1)) || (i == ((width * 17) - 1)) || (i == ((width * 18) - 1)) || (i == ((width * 19) - 1))
                || (i == ((width * 20) - 1)) || (i == ((width * 21) - 1)) || (i == ((width * 22) - 1)) || (i == ((width * 23) - 1))) //i == width * 16 - 1
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1]; 
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1]; 
            }
        }

        if (height == 25)
        {
            /*==== FIRST TILE ON THE LEFT FOR ROWS 2 THROUGH 24 =====*/ 
            if ((i == width) || (i == width * 2) || (i == width * 3) || (i == width * 4) || (i == width * 5) || (i == width * 6) || (i == width * 7) || (i == width * 8)
                || (i == width * 9) || (i == width * 10) || (i == width * 11) || (i == width * 12) || (i == width * 13) || (i == width * 14) || (i == width * 15) || (i == width * 16) ||
                (i == width * 17) || (i == width * 18) || (i == width * 19) || (i == width * 20) || (i == width * 21) || (i == width * 22) || (i == width * 23))
            {
                Tiles[i].adjacentTiles[0] = nullptr;
                Tiles[i].adjacentTiles[1] = nullptr;
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1]; 
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];  
                Tiles[i].adjacentTiles[7] = nullptr;
            }

            /*==== INBETWEEN FIRST AND LAST TILES FOR ROWS 2 THROUGH 24 ====*/ 
            if (((i < ((width * 2) - 1)) && (i > width)) || ((i > width * 2) && (i < ((width * 3) - 1))) || ((i < ((width * 4) - 1)) && (i > width * 3))
                || ((i < ((width * 5) - 1)) && (i > width * 4)) || ((i < ((width * 6) - 1)) && (i > width * 5)) || ((i < ((width * 7) - 1)) && (i > width * 6)) ||
                ((i < ((width * 8) - 1)) && (i > width * 7)) || ((i < ((width * 9) - 1)) && (i > width * 8)) || ((i < ((width * 10) - 1)) && (i > width * 9)) ||
                ((i < ((width * 11) - 1)) && (i > width * 10)) || ((i < ((width * 12) - 1)) && (i > width * 11)) || ((i < ((width * 13) - 1)) && (i > width * 12)) ||
                ((i < ((width * 14) - 1)) && (i > width * 13)) || ((i < ((width * 15) - 1)) && (i > width * 14)) || ((i < ((width * 16) - 1)) && (i > width * 15)) ||
                ((i < ((width * 17) - 1)) && (i > width * 16)) || ((i < ((width * 18) - 1)) && (i > width * 17)) || ((i < ((width * 19) - 1)) && (i > width * 18)) ||
                ((i < ((width * 20) - 1)) && (i > width * 19)) || ((i < ((width * 21) - 1)) && (i > width * 20)) || ((i < ((width * 22) - 1)) && (i > width * 21)) ||
                ((i < ((width * 23) - 1)) && (i > width * 22)) || ((i < ((width * 24) - 1)) && (i > width * 23)))
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
                Tiles[i].adjacentTiles[2] = &Tiles[i - width];
                Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
                Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
                Tiles[i].adjacentTiles[5] = &Tiles[i + width + 1];
                Tiles[i].adjacentTiles[6] = &Tiles[i + width];
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }

            /*==== LAST TILE ON THE RIGHT FOR ROWS 2 THROUGH 24 =====*/  
            if ((i == ((width * 2) - 1)) || (i == ((width * 3) - 1)) || (i == ((width * 4) - 1)) || (i == ((width * 5) - 1)) || (i == ((width * 6) - 1)) || (i == ((width * 7) - 1))
                || (i == ((width * 8) - 1)) || (i == ((width * 9) - 1)) || (i == ((width * 10) - 1)) || (i == ((width * 11) - 1)) || (i == ((width * 12) - 1)) || (i == ((width * 13) - 1))
                || (i == ((width * 14) - 1)) || (i == ((width * 15) - 1)) || (i == ((width * 16) - 1)) || (i == ((width * 17) - 1)) || (i == ((width * 18) - 1)) || (i == ((width * 19) - 1))
                || (i == ((width * 20) - 1)) || (i == ((width * 21) - 1)) || (i == ((width * 22) - 1)) || (i == ((width * 23) - 1)) || (i == ((width * 24) - 1))) //i == width * 16 - 1
            {
                Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
                Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1]; 
                Tiles[i].adjacentTiles[2] = &Tiles[i - width]; 
                Tiles[i].adjacentTiles[3] = nullptr;
                Tiles[i].adjacentTiles[4] = nullptr;
                Tiles[i].adjacentTiles[5] = nullptr;
                Tiles[i].adjacentTiles[6] = &Tiles[width + i]; 
                Tiles[i].adjacentTiles[7] = &Tiles[i + width - 1];
            }
        }

        /*==== FIRST TILE ON LAST LINE =====*/     
        if (i == ((width * height) - width)) 
        {
            Tiles[i].adjacentTiles[0] = nullptr;
            Tiles[i].adjacentTiles[1] = nullptr;
            Tiles[i].adjacentTiles[2] = &Tiles[i - width];
            Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
            Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
            Tiles[i].adjacentTiles[5] = nullptr;
            Tiles[i].adjacentTiles[6] = nullptr;
            Tiles[i].adjacentTiles[7] = nullptr;
        }

        /*==== INBETWEEN FIRST AND LAST TILE ON LAST LINE ====*/ 
        if ((i > ((width * height) - width)) && i < ((width * height) - 1))
        {
            Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
            Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
            Tiles[i].adjacentTiles[2] = &Tiles[i - width];
            Tiles[i].adjacentTiles[3] = &Tiles[i - width + 1];
            Tiles[i].adjacentTiles[4] = &Tiles[i + 1];
            Tiles[i].adjacentTiles[5] = nullptr;
            Tiles[i].adjacentTiles[6] = nullptr;
            Tiles[i].adjacentTiles[7] = nullptr;
        }
        /*==== LAST TILE ON LAST LINE (DEFAULT LINE 16) ====*/    
        if (i == ((width * height) - 1))
        {
            Tiles[i].adjacentTiles[0] = &Tiles[i - 1];
            Tiles[i].adjacentTiles[1] = &Tiles[i - width - 1];
            Tiles[i].adjacentTiles[2] = &Tiles[i - width];
            Tiles[i].adjacentTiles[3] = nullptr;
            Tiles[i].adjacentTiles[4] = nullptr;
            Tiles[i].adjacentTiles[5] = nullptr;
            Tiles[i].adjacentTiles[6] = nullptr;
            Tiles[i].adjacentTiles[7] = nullptr;
        }
    }
}

