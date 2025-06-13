#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
using namespace std;

const float PI = 3.14159265f;

sf::ConvexShape createPentagon(float x, float y, float radius, sf::Color color) {
    sf::ConvexShape pentagon;
    pentagon.setPointCount(5);
    for (int i = 0; i < 5; i++) {
        float angle = 2 * PI * i / 5 - PI / 2;
        float px = x + radius * cos(angle);
        float py = y + radius * sin(angle);
        pentagon.setPoint(i, sf::Vector2f(px, py));
    }
    pentagon.setFillColor(color);
    return pentagon;
}

std::vector<sf::ConvexShape> generatePentagonMap(int rows, int cols, float radius) {
    std::vector<sf::ConvexShape> map;
    float dx = radius * 1.75f;
    float dy = radius * 1.5f;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float x = j * dx + (i % 2) * (dx / 2); // desplazamiento tipo mosaico
            float y = i * dy;
            sf::Color color = sf::Color::White;

            if ((i + j) % 7 == 0)
                color = sf::Color::Red;

            map.push_back(createPentagon(x + 100, y + 100, radius, color));
        }
    }
    return map;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML 2.5.1 funcionando");

    // 🔹 Generar mapa una vez
    std::vector<sf::ConvexShape> pentagonMap = generatePentagonMap(10, 10, 30.0f);

    sf::Event event;

    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        // 🔹 Dibujar cada pentágono en pantalla
        for (auto& pentagon : pentagonMap) {
            window.draw(pentagon);
        }

        window.display();
    }

    return 0;
}
