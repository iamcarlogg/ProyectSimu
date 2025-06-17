#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <unordered_map>
#include "./Simu/PentagonGrid.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Laberinto de Pentágonos");
    sf::Vector2u windowSize = window.getSize();

    std::vector<std::string> layout = PentagonGrid::loadLayoutFromFile("map.txt");
    PentagonGrid grid(layout, 30.0f, windowSize);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                grid.handleMouseClick(mousePos);
            }
        }

        window.clear();
        grid.draw(window);
        grid.drawPlayer(window);
        window.display();
    }

    return 0;
}
