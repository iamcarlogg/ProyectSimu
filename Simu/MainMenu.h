#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
class MainMenu {
	private:
        sf::Font font;
        sf::Text title;
        std::vector<sf::RectangleShape> buttons;
        std::vector<sf::Text> labels;
        std::vector<std::function<void()>> actions;

        float buttonWidth = 200.f;
        float buttonHeight = 50.f;
        float spacing = 20.f;


	public:
        MainMenu(sf::Vector2u windowSize);

        void addButton(const std::string& label, std::function<void()> onClick);
        void render(sf::RenderWindow& window);
        void handleClick(sf::Vector2f mousePos);
};