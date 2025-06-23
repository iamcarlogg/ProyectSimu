#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "./Simu/PentagonGrid.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Laberinto de Pentágonos");
    sf::Vector2u windowSize = window.getSize();

    auto layout = PentagonGrid::loadLayoutFromFile("map.txt");
    PentagonGrid grid(layout, 30.0f, windowSize, &window);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed
                && event.mouseButton.button == sf::Mouse::Left) {
                auto mp = window.mapPixelToCoords(
                    sf::Mouse::getPosition(window)
                );
                grid.handleMouseClick(mp);
            }
        }

        window.clear(sf::Color(10, 15, 30));
        grid.draw(window);
        grid.drawPlayer(window);
        window.display();
    }

    return 0;
}
