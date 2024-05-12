#include <SFML/Graphics.hpp>
#include <pthread.h>
#include <iostream>

using namespace std;
using namespace sf;

const float cellSize = 25.0f; // Increased cell size for larger spacing
const unsigned int windowWidth = 71 * cellSize; // Adjusted based on new cell size
const unsigned int windowHeight = 38 * cellSize;

// Function prototypes for the main engine thread and parallel threads
void* main_thread_funct(void*);
void* parallel_thread1(void*);
void* parallel_thread2(void*);

// 2D array representing the maze grid
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

// Function for the main engine thread
void* main_thread_funct(void* arg) {
    RenderWindow* windowPtr = static_cast<RenderWindow*>(arg);
    RenderWindow& window = *windowPtr;

    // Create a 2D array of rectangles for drawing the maze grid
    RectangleShape mazeGrid[38][71];

    // Initialize the maze grid
    for (int row = 0; row < 38; row++) {
        for (int col = 0; col < 71; col++) {
            // Set the position of each cell based on the cell size
            mazeGrid[row][col].setPosition(col * cellSize, row * cellSize);

            // Set the size of each cell
            mazeGrid[row][col].setSize(Vector2f(cellSize, cellSize));

            // Set the color and style based on the maze grid character
            if (level2_maze[row][col] == '*') {
                mazeGrid[row][col].setFillColor(Color::Blue);
            } else if (level2_maze[row][col] == '.') {
                mazeGrid[row][col].setFillColor(Color::Yellow);
            } else {
                mazeGrid[row][col].setFillColor(Color::Black);
            }
        }
    }

    // Main loop for rendering
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                pthread_exit(0);
            }
        }

        // Clear the window
        window.clear();

        // Draw the maze grid
        for (int row = 0; row < 38; row++) {
            for (int col = 0; col < 71; col++) {
                window.draw(mazeGrid[row][col]);
            }
        }

        // Display the drawn frame
        window.display();
    }

    pthread_exit(0);
}

// Function for the first parallel thread
void* parallel_thread1(void* arg) {
    while (true) {
        // Perform some task without calling window.display()
        // For example, simulate some computation
    }

    pthread_exit(0);
}

// Function for the second parallel thread
void* parallel_thread2(void* arg) {
    while (true) {
        // Perform some task without calling window.display()
        // For example, simulate some computation
    }

    pthread_exit(0);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "PAC-MAN");

    pthread_t mainengine, thread1, thread2;
    pthread_create(&mainengine, nullptr, &main_thread_funct, &window);
    pthread_create(&thread1, nullptr, &parallel_thread1, nullptr);
    pthread_create(&thread2, nullptr, &parallel_thread2, nullptr);

    pthread_join(mainengine, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}

