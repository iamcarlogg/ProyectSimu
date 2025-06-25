#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>

// --- Estructura de celda ---
struct PentagonCell {
    int row = 0, col = 0;
    int id = -1;
    bool blocked = false;
    bool isStart = false;
    bool isEnd = false;
    bool volatileCell = false;
    sf::ConvexShape shape;
    std::vector<PentagonCell*> neighbors;
};

// --- Clase principal ---
class PentagonGrid {
public:
    // Métodos públicos
    static std::vector<std::string> loadLayoutFromFile(const std::string& filename);
    PentagonGrid(const std::vector<std::string>& layout, float r, sf::Vector2u wsize, sf::RenderWindow* w);
    void resetGame();
    void draw(sf::RenderWindow& w);
    void drawPlayer(sf::RenderWindow& w);
    void handleMouseClick(sf::Vector2f mp);
    PentagonCell& getCell(int r, int c);
    int getPlayerNodeId() const;
    int getEndNodeId() const;
    int getTurnCounter() const;
    int getMoveCounter() const;
    int getMovesToBreak() const;
    bool isGameFinished() const;
    void setGameFinished(bool val);
    std::vector<sf::Vector2i> solveWithBFS();
    void setPlayerPosition(int id);

private:
    // Métodos privados
    sf::ConvexShape createPentagon(float x, float y, float r, sf::Color color);
    void rebuildAdjacency();

    // Atributos
    float radius;
    sf::Vector2u windowSize;
    std::vector<std::string> originalLayout;
    sf::RenderWindow* windowRef = nullptr;

    int rows = 0, cols = 0;
    std::vector<std::vector<PentagonCell>> grid;
    std::unordered_map<int, PentagonCell*> idToCell;
    PentagonCell* playerCell = nullptr;
    int playerNodeId = -1;
    int endNodeId = -1;
    std::vector<std::vector<int>> adjacencyList;

    int turnCounter = 0;
    int moveCounter = 0;
    int movesToBreak = 3;
    bool gameFinished = false;
};