#include <SFML/Graphics.hpp>
#include "Simu/PentagonGrid.h"


int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Pentagon Pathfinding Grid");

    std::vector<std::string> layout = {
        "1111111111",
        "10a0000001",
        "1011111101",
        "1010000101",
        "1010111101",
        "10000000b1",
        "1111111111"
    };

    PentagonGrid grid(layout, 30.0f, window.getSize());
    //obtienes la lista de adyacencia
    auto& adj = grid.getAdjacencyList();
    //obtienes nodo inicio
    int from = grid.getPlayerNodeId();
    //obtienes nodo final
    int to = grid.getEndNodeId();
    //exportar lista de adyacencia como txt
    grid.exportAdjacencyListToFile("adyacencia.txt");


    sf::Event event;
    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
                    grid.handleMouseClick(mousePos);
                }
            }
        }

        window.clear(sf::Color::Black);
        grid.draw(window);
        grid.drawPlayer(window);
        window.display();
    }

    return 0;
}
