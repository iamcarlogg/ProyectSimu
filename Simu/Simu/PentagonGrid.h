#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <unordered_map>

const float PI = 3.14159265f;

struct PentagonCell {
    int row, col;
    int id = -1;
    bool blocked = false;
    bool isStart = false;
    bool isEnd = false;
    sf::ConvexShape shape;
    std::vector<PentagonCell*> neighbors;
};

class PentagonGrid {
private:
    std::vector<std::vector<PentagonCell>> grid;
    std::vector<std::vector<int>> adjacencyList;
    std::unordered_map<int, PentagonCell*> idToCell;
    int rows, cols;
    float radius;
    int playerNodeId = -1;
    int endNodeId = -1;
    PentagonCell* startCell = nullptr;
    PentagonCell* playerCell = nullptr;

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

public:
    static std::vector<std::string> loadLayoutFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo de mapa");
        }
        std::vector<std::string> layout;
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) layout.push_back(line);
        }
        return layout;
    }

    PentagonGrid(const std::vector<std::string>& layout, float radius, sf::Vector2u windowSize)
        : radius(radius) {

        rows = layout.size();
        cols = layout.empty() ? 0 : layout[0].size();

        float dx = radius * 1.75f;
        float dy = radius * 1.5f;

        float gridWidth = cols * dx + dx / 2;
        float gridHeight = rows * dy;

        float offsetX = (windowSize.x - gridWidth) / 2.0f;
        float offsetY = (windowSize.y - gridHeight) / 2.0f;

        grid.resize(rows, std::vector<PentagonCell>(cols));
        int currentId = 0;

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (j >= layout[i].size()) throw std::runtime_error("Layout string is too short");

                char ch = layout[i][j];
                bool blocked = ch == '0';
                bool start = ch == 'a';
                bool end = ch == 'b';

                float x = j * dx + (i % 2) * (dx / 2);
                float y = i * dy;

                sf::Color color = blocked ? sf::Color::Red : sf::Color::White;
                if (start) color = sf::Color::Blue;
                if (end) color = sf::Color::Green;

                PentagonCell cell;
                cell.row = i;
                cell.col = j;
                cell.blocked = blocked;
                cell.isStart = start;
                cell.isEnd = end;
                cell.id = blocked ? -1 : currentId;
                cell.shape = createPentagon(x + offsetX, y + offsetY, radius, color);

                if (!blocked) {
                    idToCell[currentId] = &grid[i][j];
                    if (start) {
                        startCell = &grid[i][j];
                        playerCell = startCell;
                        playerNodeId = currentId;
                    }
                    if (end) {
                        endNodeId = currentId;
                    }
                    currentId++;
                }

                grid[i][j] = cell;
            }
        }

        adjacencyList.resize(currentId);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (grid[i][j].blocked) continue;

                std::vector<std::pair<int, int>> dirs = {
                    {i, j - 1}, {i, j + 1}, {i - 1, j}, {i + 1, j}
                };

                if (i % 2 == 0) {
                    dirs.push_back({ i - 1, j - 1 });
                    dirs.push_back({ i + 1, j - 1 });
                }
                else {
                    dirs.push_back({ i - 1, j + 1 });
                    dirs.push_back({ i + 1, j + 1 });
                }

                for (auto& [ni, nj] : dirs) {
                    if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                        if (!grid[ni][nj].blocked) {
                            grid[i][j].neighbors.push_back(&grid[ni][nj]);
                            adjacencyList[grid[i][j].id].push_back(grid[ni][nj].id);
                        }
                    }
                }
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (const auto& row : grid) {
            for (const auto& cell : row) {
                window.draw(cell.shape);
            }
        }
    }

    void drawPlayer(sf::RenderWindow& window) {
        if (playerCell) {
            sf::CircleShape player(radius * 0.5f);
            player.setFillColor(sf::Color::Yellow);
            player.setOrigin(player.getRadius(), player.getRadius());

            sf::Vector2f center = playerCell->shape.getPoint(0);
            for (int i = 1; i < 5; i++)
                center += playerCell->shape.getPoint(i);
            center.x /= 5;
            center.y /= 5;

            player.setPosition(center);
            window.draw(player);
        }
    }

    void handleMouseClick(sf::Vector2f mousePos) {
        for (auto& row : grid) {
            for (auto& cell : row) {
                if (!cell.blocked && playerCell) {
                    if (cell.shape.getGlobalBounds().contains(mousePos)) {
                        for (PentagonCell* neighbor : playerCell->neighbors) {
                            if (neighbor == &cell) {
                                playerCell = &cell;
                                playerNodeId = cell.id;
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    const std::vector<std::vector<int>>& getAdjacencyList() const {
        return adjacencyList;
    }

    int getPlayerNodeId() const {
        return playerNodeId;
    }

    int getEndNodeId() const {
        return endNodeId;
    }

    void exportAdjacencyListToFile(const std::string& filename) const {
        std::ofstream out(filename);
        if (!out.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo para escritura");
        }

        for (size_t i = 0; i < adjacencyList.size(); ++i) {
            out << i << ": ";
            for (int neighbor : adjacencyList[i]) {
                out << neighbor << " ";
            }
            out << "\n";
        }

        out.close();
    }
};
