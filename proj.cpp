#include <SFML/Graphics.hpp>
#include <pthread.h>
#include <iostream>
#include "mainmenu.h"
#include <semaphore.h>

using namespace std;
using namespace sf;

// Define global variables
const float cellSize = 25.0f;
const unsigned int windowWidth = 71 * cellSize;
const unsigned int windowHeight = 38 * cellSize;

const char level2_maze[38][71] = {
    "*********************************************************************",
    "*       .           . * .    *                                * .   *",
    "*                     *      *      .                     .   *   . *",
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
    "*    *                         .  *        *     .  *     .         *",
    "* .  *    .  ***********          *        *  .     *               *",
    "*    *       *    *    *                   *        *     *******.  *",
    "*    *       *    *    *   .               *                    *   *",
    "*    *  .    *    *    *                   *       .            *   *",
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
    "*        .  * .           *               . *                *      *",
    "*    .      *             *         .       *                *.     *",
    "*********************************************************************"
};

// Load Pac-Man textures for different directions
Texture pacmanTextureUp;
Texture pacmanTextureDown;
Texture pacmanTextureLeft;
Texture pacmanTextureRight;
Texture pacmanTextureneutral;
Texture ghostsprite1;
Texture ghostsprite2;

// Semaphore for controlling window display
sem_t displaySemaphore;

// Function for displaying the window contents
void displayWindowContents(RenderWindow& window, Sprite& pacmanSprite, Sprite& ghostSprite1, Sprite& ghostSprite2, RectangleShape mazeGrid[38][71]) {
    // Clear the window
    window.clear();

    // Draw the maze grid
    for (int row = 0; row < 38; row++) {
        for (int col = 0; col < 71; col++) {
            window.draw(mazeGrid[row][col]);
        }
    }

    // Draw Pac-Man and ghosts
    window.draw(pacmanSprite);
    window.draw(ghostSprite1);
    window.draw(ghostSprite2);

    // Display the drawn frame
    window.display();
}

// Function for the main engine thread
void* main_thread_funct(void* arg) {

    RenderWindow* windowPtr = static_cast<RenderWindow*>(arg);
    RenderWindow& window = *windowPtr;

    // Create a 2D array of rectangles for drawing the maze grid
    RectangleShape mazeGrid[38][71];

    // Initialize the maze grid
    for (int row = 0; row < 38; row++) {
        for (int col = 0; col < 71; col++) {
            mazeGrid[row][col].setPosition(col * cellSize, row * cellSize);
            mazeGrid[row][col].setSize(Vector2f(cellSize, cellSize));
            if (level2_maze[row][col] == '*') {
                mazeGrid[row][col].setFillColor(Color::Blue);
            } else if (level2_maze[row][col] == '.') {
                mazeGrid[row][col].setFillColor(Color::Yellow);
            } else {
                mazeGrid[row][col].setFillColor(Color::Black);
            }
        }
    }

    // Load Pac-Man textures for different directions
    if (!pacmanTextureUp.loadFromFile("/home/eman/Downloads/OSPROJECT-main/res/sprites/Pacman2/up_2.png") ||
        !pacmanTextureDown.loadFromFile("/home/eman/Downloads/OSPROJECT-main/res/sprites/Pacman2/down_2.png") ||
        !pacmanTextureLeft.loadFromFile("/home/eman/Downloads/OSPROJECT-main/res/sprites/Pacman2/left_2.png") ||
        !pacmanTextureRight.loadFromFile("/home/eman/Downloads/OSPROJECT-main/res/sprites/Pacman2/right_2.png") ||
        !pacmanTextureneutral.loadFromFile("/home/eman/Downloads/OSPROJECT-main/res/sprites/Pacman2/neutral.png")) {
        cerr << "Failed to load Pac-Man textures" << endl;
        return nullptr;
    }

    // Load ghost sprite
    if (!ghostsprite1.loadFromFile("/home/eman/Downloads/OSPROJECT-main/res/sprites/Ghost/Blinky/right_1.png") ||
        !ghostsprite2.loadFromFile("/home/eman/Downloads/OSPROJECT-main/res/sprites/Ghost/Clyde/right_1.png")) {
        cerr << "Failed to load ghost sprite" << endl;
        return nullptr;
    }

    // Create Pac-Man sprite
    Sprite pacmanSprite(pacmanTextureneutral);
    // Set initial scale for Pac-Man sprite
    float scaleFactor = 1.5f;
    pacmanSprite.setScale(scaleFactor * cellSize / pacmanTextureUp.getSize().x, scaleFactor * cellSize / pacmanTextureUp.getSize().y);

    // Set initial position for Pac-Man sprite
    float pacmanPosX = (windowWidth - pacmanSprite.getLocalBounds().width) / 2;
    float pacmanPosY = (windowHeight - pacmanSprite.getLocalBounds().height) / 2;
    pacmanSprite.setPosition(pacmanPosX, pacmanPosY);

    // Create ghost sprites
    Sprite ghostSprite1(ghostsprite1);
    float ghostScaleFactor1 = 1.5f;
    ghostSprite1.setScale(ghostScaleFactor1 * cellSize / ghostsprite1.getSize().x,
        ghostScaleFactor1 * cellSize / ghostsprite1.getSize().y);

    Sprite ghostSprite2(ghostsprite2);
    float ghostScaleFactor2 = 1.5f;
    ghostSprite2.setScale(ghostScaleFactor2 * cellSize / ghostsprite2.getSize().x,
        ghostScaleFactor2 * cellSize / ghostsprite2.getSize().y);

    // Set initial position for ghost sprites
    float ghost1PosX = cellSize * 5;
    float ghost1PosY = cellSize * 7;
    ghostSprite1.setPosition(ghost1PosX, ghost1PosY);

    float ghost2PosX = cellSize * 55;
    float ghost2PosY = cellSize * 20;
    ghostSprite2.setPosition(ghost2PosX, ghost2PosY);

    // Wait for the UI thread to signal to display window contents
    sem_wait(&displaySemaphore);

    // Main loop for rendering
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                pthread_exit(0);
            } else if (event.type == sf::Event::KeyPressed) {
                // Handle keyboard events for Pac-Man movement
                switch (event.key.code) {
                    case sf::Keyboard::Up:
                        pacmanSprite.setTexture(pacmanTextureUp);
                        pacmanPosY -= cellSize;
                        break;
                    case sf::Keyboard::Down:
                        pacmanSprite.setTexture(pacmanTextureDown);
                        pacmanPosY += cellSize;
                        break;
                    case sf::Keyboard::Left:
                        pacmanSprite.setTexture(pacmanTextureLeft);
                        pacmanPosX -= cellSize;
                        break;
                    case sf::Keyboard::Right:
                        pacmanSprite.setTexture(pacmanTextureRight);
                        pacmanPosX += cellSize;
                        break;
                    default:
                        break;
                }
                // Update Pac-Man sprite position
                pacmanSprite.setPosition(pacmanPosX, pacmanPosY);
            }
        }

        // Display window contents
        displayWindowContents(window, pacmanSprite, ghostSprite1, ghostSprite2, mazeGrid);
    }

    pthread_exit(0);
}

// Function for the first parallel thread (UI thread)
void* userinterfacethread(void* arg) {
    // Extract the RenderWindow object from the argument
    sf::RenderWindow* windowPtr = static_cast<sf::RenderWindow*>(arg);
    sf::RenderWindow& window = *windowPtr;

    // Create the main menu instance
    MainMenu mainMenu(window);
    // Run the main menu loop
    mainMenu.run();

    // Signal the main engine thread to start rendering
    sem_post(&displaySemaphore);

    pthread_exit(0);
}

// Function for the second parallel thread
void* ghostcontollerthread(void* arg) {
    // Example task
    while (true) {
        // Do some computation
    }

    pthread_exit(0);
}

int main() {
    // Initialize semaphore
    sem_init(&displaySemaphore, 0, 0);

    // Create the SFML window
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "PAC-MAN");

    // Create pthreads
    pthread_t mainengine, thread1, thread2;
    pthread_create(&mainengine, nullptr, &main_thread_funct, &window);
    pthread_create(&thread1, nullptr, &userinterfacethread, &window); // Pass window to user interface thread
    pthread_create(&thread2, nullptr, &ghostcontollerthread, nullptr);

    // Join pthreads
    pthread_join(mainengine, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Destroy semaphore
    sem_destroy(&displaySemaphore);

    return 0;
}
