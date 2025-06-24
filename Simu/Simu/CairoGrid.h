#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>


class CairoGrid {
private:
    std::vector<sf::ConvexShape> pentagons;
    float size;

    sf::ConvexShape createCairoPentagon(float x, float y, sf::Color color) {
        sf::ConvexShape pent;
        pent.setPointCount(5);

        float a = size;
        float b = size / 2.0f;
        float h = size * 0.87f;

        // Puntos de un pent�gono de El Cairo
        pent.setPoint(0, sf::Vector2f(x, y));
        pent.setPoint(1, sf::Vector2f(x + a, y));
        pent.setPoint(2, sf::Vector2f(x + a + b, y + h));
        pent.setPoint(3, sf::Vector2f(x + b, y + h));
        pent.setPoint(4, sf::Vector2f(x - b, y + h / 2));

        pent.setFillColor(color);
        return pent;
    }

public:
    const float PI = 3.14159265f;

    CairoGrid(int cols, int rows, float size) : size(size) {
        float dx = size * 1.5f;
        float dy = size * 0.87f;

        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                float offsetX = col * dx;
                float offsetY = row * dy;

                if (row % 2 == 1)
                    offsetX += dx / 2;

                sf::Color color = sf::Color::White;
                if ((row + col) % 7 == 0)
                    color = sf::Color::Red;

                pentagons.push_back(createCairoPentagon(offsetX + 100, offsetY + 100, color));
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& pent : pentagons)
            window.draw(pent);
    }
};
#pragma once
