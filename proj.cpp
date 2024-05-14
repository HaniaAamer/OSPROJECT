#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include<unistd.h>
#include <X11/Xlib.h>
#include <pthread.h>
#include <mutex>
#include <cstdlib>
#include <random>
#include "mainmenu.h"
#include <iostream>
#include <semaphore.h> // Include the semaphore header file


using namespace sf;
using namespace std;


sem_t keySemaphore;   // Semaphore for keys
sem_t permitSemaphore; // Semaphore for exit permits
int boosters=2;
bool isfast=0;

const float tileSize = 25.0f; // Size of each tile in pixels
int SCORE = 0,
LIVES = 3;
bool isGameRunning = 1, 
isPaused = false, 
isFreightened = false,
isGameOver = false, 
pacmanDied = false,
key1free = 1, 
key2free = 1, 
permit1free = 1, 
permit2free = 1;
Vector2f pacmanPosition(1050, 105);
Vector2f pacmanVelocity(0.0f, 0.0f);

mutex gameRunningMutex, pausedMutex, pacPositionMutex, pacVelocityMutex;
mutex key1, key2, permit1, permit2;

const float cellSize = 25.0f;
const int WINDOW_WIDTH = 71 * cellSize,WINDOW_HEIGHT = 38 * cellSize,PACMAN_SIZE = 20,
GHOSTS_SIZE = 20,GHOST_COUNT = 3,MAZE_WIDTH = 71,MAZE_HEIGHT = 38;
const float PACMAN_SPEED = 4.0f;
char maze[MAZE_HEIGHT][MAZE_WIDTH] = {
     "*********************************************************************",
    "*       .           . * .    *                         #      * .   *",
    "*   #                 *      *      .                     .   *   . *",
    "*.         .   * .    *   .  * .           .         * .      *     *",
    "*      .       *      *      *                       *              *",
    "********       *             *****************       ****************",
    "*   .  *     . *    .      .                 * .     .       *      *",
    "*      *    ******                   .    .  *               *      *",
    "*.        . *  . *    *     *********        *      **********      *",
    "*           *    *    *             *     ****   .  *    .     .    *",
    "*      .    *    *  . *             *      .        *               *",
    "*           *    ******             *  .            *               *",
    "*    .               .   .   ************************               *",
    "*    *^                        .  |        *     .  *     .         *",
    "* .  *    .  ***********          |        *  .     *               *",
    "*    *       *    *    *          |        *        *     *******.  *",
    "*    *       *    *    *   .      |        *                    *   *",
    "*    *  .    *    *    *^         |        *       .            *   *",
    "*    *       * .  *    *          *        *                    *   *",
    "******    .  *        .*   .     .*        *       .        .   * . *",
    "*            *     .   *          ****************              *   *",
    "*    *********             .             . *                *********",
    "*            *                       .     *                *       *",
    "*            * .                 .   *******         .      *.      *",
    "******       *         *                             *      *       *",
    "*    *    .  *     .   *    *********  .             *      *     . *",
    "*  . *       *         *                             *      *       *",
    "*    *       *         *                             *      *       *",
    "*    *    .            *           **********    .   *      *       *",
    "*.   ****       .************                        *   .          *",
    "*    *           * .             .        .        . *             .*",
    "*    * .         *        *   .             *        * .     ********",
    "* .  *      ******     .  *      .   ********                       *",
    "*           *       .     *   .             ******  .     .      .  *",
    "*        .  * .           *                 *                *      *",
    "*  #     .  * .           *               . *                *   #  *",
    "*    .      *             *         .       *                *.     *",
    "*********************************************************************"
};
Texture pacmanTextureUp;
Texture pacmanTextureDown;
Texture pacmanTextureLeft;
Texture pacmanTextureRight;
Texture pacmanTextureneutral;
Texture ghostTexture1;
Texture ghostTexture2;
Texture ghostTexture3;
Texture frightnedGhostTexture;
Texture pellet1;
Texture pellet2;

Sprite pacmanSprite;
Sprite ghostSprite1;
Sprite ghostSprite2;
Sprite ghostSprite3;
Sprite palletSprite1;
Sprite palletSprite2;


pthread_t normalGhostThreads[1], runnerGhostThreads[2];
Vector2f normalGhostPositions[1], runnerGhostPositions[2];
// Function to generate a random direction
Vector2f generateRandomDirection() {
    static random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 3);

    int randomDirection = dis(gen);
    switch (randomDirection) {
        case 0: // Up
            return Vector2f(0, -1);
        case 1: // Down
            return Vector2f(0, 1);
        case 2: // Left
            return Vector2f(-1, 0);
        case 3: // Right
            return Vector2f(1, 0);
        default:
            return Vector2f(1, 0); // Default to right direction
    }
}
void* normalGhost(void* arg) 
{
    Vector2f* ghostPosition = static_cast<Vector2f*>(arg);
    float GHOST_SPEED = 2.0f; // Define GHOST_SPEED
    bool haveKey = false, havePermit = false, free = false;
    // Initialize previous directions as (1, 0) (right direction)
    Vector2f prevDirection = Vector2f(-1, 0);
    Vector2f prevPrevDirection = Vector2f(-1, 0); // Initially set to the same as prevDirection

    while (isGameRunning) 
    {
        sem_wait(&keySemaphore); // Wait for a key to become available
        sem_wait(&permitSemaphore); // Wait for an exit permit to become available

        pacPositionMutex.lock();
         if(!isPaused){
        // Choosing the direction to move ghost
        Vector2f direction = prevDirection * GHOST_SPEED;

        // Calculate the next position of the ghost
        Vector2f nextPosition = *ghostPosition + direction;
        FloatRect nextGhostBounds(nextPosition.x, nextPosition.y, GHOSTS_SIZE * 1, GHOSTS_SIZE * 1);
        FloatRect PacmanBounds(pacmanPosition.x, pacmanPosition.y, PACMAN_SIZE * 1, PACMAN_SIZE * 1);

        // Check for collision with walls
        bool wallCollision = false;
        for (int y = 0; y < MAZE_HEIGHT; ++y) {
            for (int x = 0; x < MAZE_WIDTH; ++x) {
                FloatRect wallBounds(x * tileSize, y * tileSize, tileSize, tileSize);
                if ((maze[y][x] == '*'))
                {
                    if (nextGhostBounds.intersects(wallBounds)) {
                        wallCollision = true;
                        break;
                    }
                }
                else if (maze[y][x] == '|') {
            if (prevDirection != Vector2f(-1, 0) && nextGhostBounds.intersects(FloatRect(x * tileSize, y * tileSize, tileSize, tileSize))) {
                // Check if the direction is not left, then collision with '|' wall
                wallCollision = true;
                break;
            }
        }
            }
            if (wallCollision) break;
        }

        if (wallCollision) {
            // Generate a new random direction upon wall collision
            Vector2f newDirection;
            do {
                newDirection = generateRandomDirection();
            } while (newDirection == prevDirection || newDirection == prevPrevDirection);

            prevPrevDirection = prevDirection;
            prevDirection = newDirection;
            direction = prevDirection * GHOST_SPEED;
            nextPosition = *ghostPosition + direction;
            wallCollision = false;
        }

        *ghostPosition = nextPosition;

        // Checking whether the Ghost is Eaten or not
        if (PacmanBounds.intersects(nextGhostBounds)) {
            if (!isFreightened) {
                pacmanDied = true;
            } else {
                // Handle the Ghost being eaten here
                free = false;
                nextPosition.x = 1000;
                nextPosition.y = 470;
                *ghostPosition = nextPosition;
            }
        }
        }
        sem_post(&permitSemaphore);
        sem_post(&keySemaphore);
        pacPositionMutex.unlock();
        sleep(milliseconds(10));
    }
    return nullptr;
}

 float GHOST_SPEED_R = 2.0f;// Define GHOST_SPEED
bool hasBooster = false;
bool ghost1HasBooster = false; // Flag to track if ghost 1 has eaten a booster
bool ghost2HasBooster = false; // Flag to track if ghost 2 has eaten a booster

void* runnerGhost1(void* arg) {
    Vector2f* ghostPosition = static_cast<Vector2f*>(arg);
    
    bool free = false;
    float originalSpeed = GHOST_SPEED_R; 
    Vector2f prevDirection = Vector2f(1, 0);
    Vector2f prevPrevDirection = Vector2f(1, 0); // Initially set to the same as prevDirection

    while (isGameRunning) {
        sem_wait(&keySemaphore);
        sem_wait(&permitSemaphore);
        pacPositionMutex.lock();
        if(!isPaused){
        // Apply booster speed if the ghost has eaten a booster
        if (hasBooster) {
            GHOST_SPEED_R = 4.0f;
        } else {
            GHOST_SPEED_R = originalSpeed;
        }

        // Choosing the direction to move ghost
        Vector2f direction = prevDirection * GHOST_SPEED_R;

        // Calculate the next position of the ghost
        Vector2f nextPosition = *ghostPosition + direction;
        FloatRect nextGhostBounds(nextPosition.x, nextPosition.y, GHOSTS_SIZE * 1, GHOSTS_SIZE * 1);
        FloatRect PacmanBounds(pacmanPosition.x, pacmanPosition.y, PACMAN_SIZE * 1, PACMAN_SIZE * 1);

        // Check for collision with walls
        bool wallCollision = false;
        for (int y = 0; y < MAZE_HEIGHT; ++y) {
            for (int x = 0; x < MAZE_WIDTH; ++x) {
                // Check for collision with pellets or booster if the ghost hasn't eaten a booster yet
               if (maze[y][x] == '^' && !hasBooster) { 
                    // Check for collision with '^' and if the ghost hasn't eaten a booster
                    FloatRect boosterBounds(x * tileSize, y * tileSize, tileSize, tileSize);
                    if (nextGhostBounds.intersects(boosterBounds)) {
                        hasBooster = true; // Set the flag to indicate that the ghost has eaten a booster
                        // You can add additional logic here if needed
                        maze[y][x] = ' '; // Remove the booster from the maze
                    }
                }
                // Check for collision with walls
                if (maze[y][x] == '*') {
                    FloatRect wallBounds(x * tileSize, y * tileSize, tileSize, tileSize);
                    if (nextGhostBounds.intersects(wallBounds)) {
                        wallCollision = true;
                        break;
                    }
                }
                else if (maze[y][x] == '|') {
            if (prevDirection != Vector2f(-1, 0) && nextGhostBounds.intersects(FloatRect(x * tileSize, y * tileSize, tileSize, tileSize))) {
                // Check if the direction is not left, then collision with '|' wall
                wallCollision = true;
                break;
            }
        }
            }
            if (wallCollision) break;
        }

        // Handle collision with walls
        if (wallCollision) {
            Vector2f newDirection;
            do {
                newDirection = generateRandomDirection();
            } while (newDirection == prevDirection || newDirection == prevPrevDirection);

            prevPrevDirection = prevDirection;
            prevDirection = newDirection;
            direction = prevDirection * GHOST_SPEED_R;
            nextPosition = *ghostPosition + direction;
            wallCollision = false;
        }

        *ghostPosition = nextPosition;

        // Check for collision with Pac-Man
        if (PacmanBounds.intersects(nextGhostBounds)) {
            if (!isFreightened) {
                pacmanDied = true;
            } else {
                free = false;
                nextPosition.x = 1000;
                nextPosition.y = 470;
                *ghostPosition = nextPosition;
            }
        }
}
        sem_post(&permitSemaphore);
        sem_post(&keySemaphore);
        pacPositionMutex.unlock();
        sleep(milliseconds(10));
    }
    return nullptr;
}

    void* runnerGhost2(void* arg) {
    Vector2f* ghostPosition = static_cast<Vector2f*>(arg);
    float GHOST_SPEED_R = 2.5f; // Define GHOST_SPEED for runner ghost 2
    bool hasBooster = false;
    float originalSpeed = GHOST_SPEED_R; 
    Vector2f prevDirection = Vector2f(1, 0);
    Vector2f prevPrevDirection = Vector2f(1, 0); // Initially set to the same as prevDirection

    while (isGameRunning) {
        sem_wait(&keySemaphore);
        sem_wait(&permitSemaphore);
        pacPositionMutex.lock();
        if(!isPaused){
        // Apply booster speed if the ghost has eaten a booster
        if (hasBooster) {
            GHOST_SPEED_R = 4.5f;
        } else {
            GHOST_SPEED_R = originalSpeed;
        }

        // Choosing the direction to move ghost
        Vector2f direction = prevDirection * GHOST_SPEED_R;

        // Calculate the next position of the ghost
        Vector2f nextPosition = *ghostPosition + direction;
        FloatRect nextGhostBounds(nextPosition.x, nextPosition.y, GHOSTS_SIZE * 1, GHOSTS_SIZE * 1);
        FloatRect PacmanBounds(pacmanPosition.x, pacmanPosition.y, PACMAN_SIZE * 1, PACMAN_SIZE * 1);

        // Check for collision with walls
        bool wallCollision = false;
        for (int y = 0; y < MAZE_HEIGHT; ++y) {
            for (int x = 0; x < MAZE_WIDTH; ++x) {
                // Check for collision with pellets or booster if the ghost hasn't eaten a booster yet
                if (maze[y][x] == '^' && !hasBooster) { 
                    // Check for collision with '^' and if the ghost hasn't eaten a booster
                    FloatRect boosterBounds(x * tileSize, y * tileSize, tileSize, tileSize);
                    if (nextGhostBounds.intersects(boosterBounds)) {
                        hasBooster = true; // Set the flag to indicate that the ghost has eaten a booster
                        // You can add additional logic here if needed
                        maze[y][x] = ' '; // Remove the booster from the maze
                    }
                }
                // Check for collision with walls
                if (maze[y][x] == '*') {
                    FloatRect wallBounds(x * tileSize, y * tileSize, tileSize, tileSize);
                    if (nextGhostBounds.intersects(wallBounds)) {
                        wallCollision = true;
                        break;
                    }
                }
                else if (maze[y][x] == '|') {
                    if (prevDirection != Vector2f(-1, 0) && nextGhostBounds.intersects(FloatRect(x * tileSize, y * tileSize, tileSize, tileSize))) {
                        // Check if the direction is not left, then collision with '|' wall
                        wallCollision = true;
                        break;
                    }
                }
            }
            if (wallCollision) break;
        }

        // Handle collision with walls
        if (wallCollision) {
            Vector2f newDirection;
            do {
                newDirection = generateRandomDirection();
            } while (newDirection == prevDirection || newDirection == prevPrevDirection);

            prevPrevDirection = prevDirection;
            prevDirection = newDirection;
            direction = prevDirection * GHOST_SPEED_R;
            nextPosition = *ghostPosition + direction;
            wallCollision = false;
        }

        *ghostPosition = nextPosition;

        // Check for collision with Pac-Man
        if (PacmanBounds.intersects(nextGhostBounds)) {
            if (!isFreightened) {
                pacmanDied = true;
            } else {
                // Handle the Ghost being eaten here
                // Set the ghost to a safe position
                nextPosition.x = 1000;
                nextPosition.y = 470;
                *ghostPosition = nextPosition;
            }
        }
}
        sem_post(&permitSemaphore);
        sem_post(&keySemaphore);
        pacPositionMutex.unlock();
        sleep(milliseconds(10));
    }
    return nullptr;
}
void* handleInput(void* arg) 
{
    RenderWindow* window = static_cast<RenderWindow*>(arg);
    while (isGameRunning) 
    {
        Event event;
        while (window->pollEvent(event)) 
        {
            if (event.type == Event::Closed) 
            {
                gameRunningMutex.lock();
                isGameRunning = false;
                gameRunningMutex.unlock();
            }
            if (event.type == Event::KeyPressed) 
            {
                pausedMutex.lock();
                if (!isPaused) {
                    pacVelocityMutex.lock();
                    if (event.key.code == Keyboard::Left) {
                        pacmanVelocity = Vector2f(-PACMAN_SPEED, 0.0f);
                        //cout<<"pacmanVelocity.x:"<<pacmanVelocity.x<<endl;
                    }
                    else if (event.key.code == Keyboard::Right) {
                        pacmanVelocity = Vector2f(PACMAN_SPEED, 0.0f);
                        //cout<<"pacmanVelocity.x:"<<pacmanVelocity.x<<endl;
                    }
                    else if (event.key.code == Keyboard::Up) {
                        pacmanVelocity = Vector2f(0.0f, -PACMAN_SPEED);
                        //cout<<"pacmanVelocity.y:"<<pacmanVelocity.y<<endl;
                    }
                    else if (event.key.code == Keyboard::Down) {
                        pacmanVelocity = Vector2f(0.0f, PACMAN_SPEED);
                        //cout<<"pacmanVelocity.y:"<<pacmanVelocity.y<<endl;
                    }
                    pacVelocityMutex.unlock();
                }
                pausedMutex.unlock();


                if (event.key.code == Keyboard::P) {
                    pausedMutex.lock();
                    isPaused = !isPaused; // Toggle pause state
                    pausedMutex.unlock();
                }
                else if (event.key.code == Keyboard::X) {
                    pausedMutex.lock();
                        gameRunningMutex.lock();
                        isGameRunning = false; // Toggle pause state
                        gameRunningMutex.unlock();
                    
                    pausedMutex.unlock();
                }

            }
        }
        sleep(milliseconds(10));
    }
    return nullptr;
}

void* checkCollision(void* arg) {
    float pelletTimer = 0.0;
    Clock clock;
    float boosterTimer = 0.0;
    const float boosterDuration = 5.0; // Duration of booster effect in seconds
    const float boosterSpeed = 6.0f; 
    while (isGameRunning) 
    {
        pausedMutex.lock();
        if (!isPaused) 
        {

            pacPositionMutex.lock();
            Vector2f nextPosition = pacmanPosition + pacmanVelocity;
            FloatRect nextPacmanBounds(nextPosition.x, nextPosition.y, PACMAN_SIZE * 1, PACMAN_SIZE * 1);

            // Check Pellet Timer
            if (isFreightened) {
                if (pelletTimer == 0.0) clock.restart();
                sf::Time elapsed = clock.getElapsedTime();
                pelletTimer = elapsed.asSeconds();
                if (pelletTimer >= 5.0) {
                    isFreightened = false;
                    pelletTimer = 0.0;
                }
            }
            // Checking timer to respawn pellets
            if (!isFreightened) {
                if (pelletTimer == 0.0) clock.restart();
                sf::Time elapsed = clock.getElapsedTime();
                pelletTimer = elapsed.asSeconds();
                if (pelletTimer > 10.0 && maze[1][55] == ' ') { maze[1][55] = '#'; pelletTimer = 0.0; }
                if (pelletTimer > 10.0 && maze[2][4] == ' ') { maze[2][4] = '#'; pelletTimer = 0.0; }
                if (pelletTimer > 10.0 && maze[35][3] == ' ') { maze[35][3] = '#'; pelletTimer = 0.0; }
                if (pelletTimer > 10.0 && maze[34][65] == ' ') { maze[35][65] = '#'; pelletTimer = 0.0; }
            }
              
            // Check for booster effect
            if (hasBooster) {
                if (boosterTimer == 0.0) clock.restart();
                sf::Time elapsed = clock.getElapsedTime();
                boosterTimer = elapsed.asSeconds();
                if (boosterTimer >= boosterDuration) {
                    hasBooster = false;
                    boosterTimer = 0.0;
                    boosters--;
                }
            }  
            
            // Check for collision with walls
            bool wallCollision = false;
            for (int y = 0; y < MAZE_HEIGHT; ++y) {
                for (int x = 0; x < MAZE_WIDTH; ++x) {
                    if (maze[y][x] == '*' || maze[y][x] == '|') {
                        FloatRect wallBounds(x * tileSize, y * tileSize, tileSize, tileSize);
                        if (nextPacmanBounds.intersects(wallBounds)) {
                            wallCollision = 1;
                            break;
                        }
                    }
                    else if (maze[y][x] == '.') {
                        FloatRect pelletBounds(x * 25 + 17, y * 25 + 17, 16, 16);

                        // Check if Pacman collects the coin
                        if (nextPacmanBounds.intersects(pelletBounds)) {
                            maze[y][x] = ' ';
                            ++SCORE;
                            //printf("%d\n", SCORE);

                        }
                    }
                    else if (maze[y][x] == '#' && !isFreightened) {
                        FloatRect pelletBounds(x * 25 + 17, y * 25 + 17, 16, 16);

                        // Check if Pacman collects the pellet
                        if (nextPacmanBounds.intersects(pelletBounds)) {
                            maze[y][x] = ' ';
                            isFreightened = 1;
                            clock.restart();
                        }
                    }
                }
                if (wallCollision) break;

            }

            if (!wallCollision) pacmanPosition = nextPosition;

            pacPositionMutex.unlock();
        }

        pausedMutex.unlock();
        sleep(milliseconds(9));
    }

    return nullptr;
}

void renderGame(RenderWindow& window, char maze[][MAZE_WIDTH]) {
    
    pacmanSprite.setPosition(pacmanPosition);
    float scaleFactor = 1.5f;
    pacmanSprite.setScale(scaleFactor * cellSize / pacmanTextureUp.getSize().x, scaleFactor * cellSize / pacmanTextureUp.getSize().y);
    pacmanSprite.setTexture(pacmanTextureneutral);
   
    if (pacmanVelocity.x > 0) 
    {
    pacmanSprite.setTexture(pacmanTextureRight);
    } 
    else if (pacmanVelocity.x < 0) 
    {
    pacmanSprite.setTexture(pacmanTextureLeft);
    } 
    else if (pacmanVelocity.y > 0) 
    {
    pacmanSprite.setTexture(pacmanTextureDown);
    } 
    else if (pacmanVelocity.y < 0) 
    {
    pacmanSprite.setTexture(pacmanTextureUp);
    }

    float ghostScaleFactor = 1.5f;
    ghostSprite1.setPosition(normalGhostPositions[0]);
    ghostSprite1.setScale(ghostScaleFactor * cellSize / ghostTexture1.getSize().x, ghostScaleFactor * cellSize / ghostTexture1.getSize().y);
    ghostSprite1.setTexture(ghostTexture1);
  
    ghostSprite2.setPosition(runnerGhostPositions[0]);
    ghostSprite2.setScale(ghostScaleFactor * cellSize / ghostTexture2.getSize().x, ghostScaleFactor * cellSize / ghostTexture2.getSize().y);
    ghostSprite2.setTexture(ghostTexture2);
    
    ghostSprite3.setPosition(runnerGhostPositions[1]);
    ghostSprite3.setScale(ghostScaleFactor * cellSize / ghostTexture3.getSize().x, ghostScaleFactor * cellSize / ghostTexture3.getSize().y);
    ghostSprite3.setTexture(ghostTexture3);
  

   


    sf::Font font;
    font.loadFromFile("font.ttf");

    // Draw Score Text
    sf::Text scoreText("Score:" + to_string(SCORE), font, 20);
   scoreText.setPosition(window.getSize().x - 200, window.getSize().y - 50);
    scoreText.setFillColor(sf::Color::White);
    window.draw(scoreText);

    // Draw Lives Text
    sf::Text livesText("Lives:" + to_string(LIVES), font, 20);
    livesText.setPosition(window.getSize().x - 400, window.getSize().y - 50);
    livesText.setFillColor(sf::Color::White);
    window.draw(livesText);

    // Draw Pac-Man Name
    sf::Texture nameTexture;
    nameTexture.loadFromFile("logo.jpg");
    sf::Sprite nameSprite;
    nameSprite.setTexture(nameTexture);
    nameSprite.setPosition(850, 50);
    nameSprite.setScale(0.75f, 0.75f);
   // window.draw(nameSprite);



    // Draw Maze
    for (int y = 0; y < MAZE_HEIGHT; ++y) {
        for (int x = 0; x < MAZE_WIDTH; ++x) {
            float xPos = x * tileSize;
            float yPos = y * tileSize;

            // Draw walls
            if (maze[y][x] == '*') {
                RectangleShape wall(Vector2f(tileSize, tileSize));
                wall.setPosition(xPos, yPos);
                wall.setFillColor(Color::Blue);
                window.draw(wall);
            }

            // Draw coins
            else if (maze[y][x] == '.') {
                CircleShape coin(3);
                coin.setFillColor(Color::White);
                coin.setPosition(xPos + tileSize / 2 - 3, yPos + tileSize / 2 - 3);
                window.draw(coin);
            }
            
            else if (maze[y][x] == '^') {

                float scalefruit = 1.5f;
                palletSprite2.setScale(scalefruit * cellSize / pellet2.getSize().x, scalefruit * cellSize / pellet2.getSize().y);
                palletSprite2.setPosition(xPos + tileSize / 2 - 3, yPos + tileSize / 2 - 3);
                palletSprite2.setTexture(pellet2);
                window.draw(palletSprite2);

                //CircleShape boostie(9);
                //boostie.setFillColor(Color::Red);
                //boostie.setPosition(xPos + tileSize / 2 - 3, yPos + tileSize / 2 - 3);
                //window.draw(boostie);
            }

            else if (maze[y][x] == '#') 
            {

                float scalefruit = 1.5f;
                palletSprite1.setScale(scalefruit * cellSize / pellet1.getSize().x, scalefruit * cellSize / pellet1.getSize().y);
                palletSprite1.setPosition(xPos + tileSize / 2 - 9, yPos + tileSize / 2 - 9);
                palletSprite1.setTexture(pellet1);
                window.draw(palletSprite1);
                //CircleShape pellet(9);
                //pellet.setFillColor(Color::Magenta);
                //pellet.setPosition(xPos + tileSize / 2 - 9, yPos + tileSize / 2 - 9);
                //window.draw(pellet);
            }
        }
    }

}

void* uiHandler(void* arg) 
{
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "P A C M A N");
    sf::Music music;
    if (!music.openFromFile("woho.wav")) 
    {
        std::cerr << "Failed to load background music" << std::endl;
    }
    music.setVolume(100); // Set volume to 50%
   // music.setLoop(true); // Loop the music

    MainMenu mainMenu(window);
    mainMenu.run();
   
    if (!pacmanTextureUp.loadFromFile("up_2.png") ||
        !pacmanTextureDown.loadFromFile("down_2.png") ||
        !pacmanTextureLeft.loadFromFile("left_2.png") ||
        !pacmanTextureRight.loadFromFile("right_2.png") ||
        !pacmanTextureneutral.loadFromFile("neutral.png")) {
        cerr << "Failed to load Pac-Man textures" << endl;
        return nullptr;
    }

    // Load ghost sprite
    if (!ghostTexture1.loadFromFile("ghost.png") ||
        !ghostTexture2.loadFromFile("right_1.png") ||
        !ghostTexture3.loadFromFile("ghostred.png") ||
        !frightnedGhostTexture.loadFromFile("frightened.png")) {
        cerr << "Failed to load ghost sprite" << endl;
        return nullptr;
    }

    if(!pellet1.loadFromFile("strawberry.png") ||
        !pellet2.loadFromFile("cherry.png"))
    {
        cerr << "Failed to load ghost sprite" << endl;
        return nullptr;
    }


    pthread_t inputThread, collisionThread;
    pthread_create(&inputThread, nullptr, handleInput, &window);
    pthread_create(&collisionThread, nullptr, checkCollision, nullptr);

   
    /*normalGhostPositions[0] = Vector2f(100, 100);
    normalGhostPositions[1] = Vector2f(700, 550);
    runnerGhostPositions[0] = Vector2f(200, 600);*/
    normalGhostPositions[0] = Vector2f(1000, 400);
    runnerGhostPositions[0] = Vector2f(1000, 480);//1000 480
    runnerGhostPositions[1] = Vector2f(900, 470);//900 470
    //for (int i = 0; i < GHOST_COUNT / 2; ++i) {
    pthread_create(&normalGhostThreads[0], nullptr, normalGhost, &normalGhostPositions[0]);
    //}
    //for (int i = 0; i < (GHOST_COUNT - GHOST_COUNT / 2); ++i) {
    pthread_create(&runnerGhostThreads[0], nullptr, runnerGhost1, &runnerGhostPositions[0]);
    pthread_create(&runnerGhostThreads[1], nullptr, runnerGhost2, &runnerGhostPositions[1]);
    //}

    while (isGameRunning) 
    {
        pausedMutex.lock();
        //cout<<"in game"<<endl;
        music.play();
        // Check if the game is paused
        if (!isPaused) 
        {
            window.clear(Color::Black);
            renderGame(window, maze);
          
            // Draw Pacman
            if (pacmanDied) 
            {
                pacmanPosition.x = 1050;
                pacmanPosition.y = 105;
                LIVES--;
                pacmanDied = false;
            }
            window.draw(pacmanSprite);
            // Draw Ghosts
           
            if (isFreightened == 1)
            {
                //cout<<"freightened"<<endl;
                ghostSprite1.setTexture(frightnedGhostTexture);
                ghostSprite2.setTexture(frightnedGhostTexture);
                ghostSprite3.setTexture(frightnedGhostTexture);
            }
            window.draw(ghostSprite1);
            window.draw(ghostSprite2);
            window.draw(ghostSprite3);
            

            // Check for Game Over
            if ( LIVES == 0) {
                isGameOver = 1;
                window.clear();
                sf::Font font;
                font.loadFromFile("font.ttf");

                sf::Texture overTexture;
                overTexture.loadFromFile("over.jpeg");
                

                sf::Sprite overSprite;
                overSprite.setTexture(overTexture);
                overSprite.setPosition(550, 100);
                window.draw(overSprite);

                sf::Text ExitText("X : EXIT", font, 40);
                ExitText.setPosition(750, 700);
                ExitText.setFillColor(sf::Color::Yellow);
                window.draw(ExitText);
            }
        }
        else {
            sf::Font font;
            font.loadFromFile("font.ttf");
            sf::Text ExitText("GAME PAUSED", font, 25);
            ExitText.setPosition(window.getSize().x - 700, window.getSize().y - 54);
            ExitText.setFillColor(Color::Yellow);
            window.draw(ExitText);
        }

        pausedMutex.unlock();

        window.display();
        sleep(milliseconds(10));
    }
    music.stop(); // Stop the music
    music.~Music(); // Destruct the music object
    return nullptr;
}


int main() {
     
    
    sem_init(&keySemaphore, 0, 1);    // Initial value is 1, allowing one thread at a time
    sem_init(&permitSemaphore, 0, 1); // Initial value is 1, allowing one thread at a time

    XInitThreads();
    srand(time(0));
    pthread_t uiThread;
    pthread_create(&uiThread, nullptr, uiHandler, nullptr);

    while (isGameRunning);
    
    return 0;
}
