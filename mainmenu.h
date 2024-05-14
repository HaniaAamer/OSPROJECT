#ifndef MAINMENU_H
#define MAINMENU_H

#include <SFML/Graphics.hpp>
#include <iostream>

class MainMenu {
public:
    MainMenu(sf::RenderWindow& window) : window(window), gameStarted(false) {}

    void run();

private:
    sf::RenderWindow& window;
    bool gameStarted; // Flag to indicate whether the game has started

    void displayMenu();
    void handleKeyPress(sf::Keyboard::Key key);

    sf::Texture backgroundTexture;
    sf::Sprite menupng;

    sf::Texture pacmanTextureUp;
    sf::Sprite pacmanSpriteUp;

};

void MainMenu::run() {

     if (!backgroundTexture.loadFromFile("logo.jpg") ||
        !pacmanTextureUp.loadFromFile("welp.jpeg")) {
        std::cerr << "Failed to load texture" << std::endl;
        return;
    }
  
    menupng.setTexture(backgroundTexture);
    menupng.setPosition((window.getSize().x - menupng.getLocalBounds().width) / 2, 50);
    //menupng.setScale(0.5, 0.5);

    pacmanSpriteUp.setTexture(pacmanTextureUp);
    pacmanSpriteUp.setPosition((window.getSize().x - pacmanSpriteUp.getLocalBounds().width) / 2, 500);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed) {
                handleKeyPress(event.key.code);
            }
        }

        if (gameStarted) {
            // Break out of the loop if the game has started
            break;
        }

        window.clear();
        displayMenu();
        window.display();
    }
}
void MainMenu::displayMenu() {

      window.draw(menupng);
    window.draw(pacmanSpriteUp);


    sf::Font font;
    if (!font.loadFromFile("font.ttf")) {
        std::cerr << "Failed to load font" << std::endl;
        return;
    }

    sf::Text title("M E N U", font, 30);
    title.setFillColor(sf::Color::Yellow);
    // Center title horizontally and place it at the top of the window
    title.setPosition((window.getSize().x - title.getLocalBounds().width) / 2, 400);

    sf::Text start("Press Space to Start", font, 30);
    start.setFillColor(sf::Color::White);
    // Center start text horizontally and place it below the title
    start.setPosition((window.getSize().x - start.getLocalBounds().width) / 2, title.getPosition().y + 100);

    sf::Text exit("Press Escape to Exit", font, 30);
    exit.setFillColor(sf::Color::White);
    // Center exit text horizontally and place it below the start text
    exit.setPosition((window.getSize().x - exit.getLocalBounds().width) / 2, start.getPosition().y + 50);

    window.draw(title);
    window.draw(start);
    window.draw(exit);
}


void MainMenu::handleKeyPress(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Space) {
        // Start the game
        std::cout << "Starting the game..." << std::endl;
        gameStarted = true; // Set the flag to true to indicate the game has started
    }
    else if (key == sf::Keyboard::Escape) {
        std::cout << "Exiting the game..." << std::endl;
        window.close();
    }
}

#endif // MAINMENU_H
