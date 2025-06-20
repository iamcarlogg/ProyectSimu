#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

static constexpr float PI = 3.14159265f;

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

class PentagonGrid {
private:
    std::vector<std::vector<PentagonCell>> grid;
    std::vector<std::vector<int>> adjacencyList;
    std::unordered_map<int, PentagonCell*> idToCell;
    int rows = 0, cols = 0;
    float radius = 0.f;
    int playerNodeId = -1;
    int endNodeId = -1;
    PentagonCell* playerCell = nullptr;
    int moveCounter = 1;
    int movesToBreak = 10;
    int turnCounter = 0;

    // **Font** cargada una sola vez
    sf::Font font_;
    bool fontLoaded_ = false;

    sf::ConvexShape createPentagon(float x, float y, float r, sf::Color color) {
        sf::ConvexShape pent;
        pent.setPointCount(5);
        for (int i = 0; i < 5; ++i) {
            float angle = 2 * PI * i / 5 - PI / 2;
            pent.setPoint(i, { x + r * std::cos(angle), y + r * std::sin(angle) });
        }
        pent.setFillColor(color);
        pent.setOutlineColor(sf::Color::White);
        pent.setOutlineThickness(1.f);
        return pent;
    }

    void rebuildAdjacency() {
        // 1) Actualizar células volátiles
        for (auto& row : grid) {
            for (auto& cell : row) {
                if (cell.volatileCell) {
                    if (turnCounter % 2 == 0) {
                        cell.blocked = true;
                        cell.shape.setFillColor(sf::Color(100, 0, 0));
                    }
                    else {
                        cell.blocked = false;
                        cell.shape.setFillColor(sf::Color(150, 150, 255));
                    }
                }
            }
        }

        // 2) Limpiar
        for (auto& row : grid)
            for (auto& cell : row)
                cell.neighbors.clear();
        for (auto& lst : adjacencyList)
            lst.clear();

        // 3) Reconstruir
        auto isNav = [&](const PentagonCell& c) {
            if (&c == playerCell) return true;
            return !c.blocked;
            };

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                auto& c = grid[i][j];
                if (c.id < 0 || !isNav(c)) continue;

                std::vector<std::pair<int, int>> dirs = {
                    {i, j - 1}, {i, j + 1},
                    {i - 1, j}, {i + 1, j}
                };
                if (i % 2 == 0) {
                    dirs.push_back({ i - 1, j - 1 });
                    dirs.push_back({ i + 1, j - 1 });
                }
                else {
                    dirs.push_back({ i - 1, j + 1 });
                    dirs.push_back({ i + 1, j + 1 });
                }

                for (auto [ni, nj] : dirs) {
                    if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                        auto& n = grid[ni][nj];
                        if (n.id >= 0 && isNav(n)) {
                            c.neighbors.push_back(&n);
                            adjacencyList[c.id].push_back(n.id);
                        }
                    }
                }
            }
        }
    }

public:
    static std::vector<std::string> loadLayoutFromFile(const std::string& filename) {
        std::ifstream f(filename);
        if (!f.is_open()) throw std::runtime_error("No se pudo abrir el archivo");
        std::vector<std::string> lay; std::string line;
        while (std::getline(f, line))
            if (!line.empty()) lay.push_back(line);
        return lay;
    }

    PentagonGrid(const std::vector<std::string>& layout, float r, sf::Vector2u wsize)
        : radius(r)
    {
        // Carga de fuente **UNA VEZ**
        fontLoaded_ = font_.loadFromFile("arial.ttf");
        if (!fontLoaded_) {
            std::cerr << "Warning: no se pudo cargar arial.ttf\n";
        }

        rows = layout.size();
        cols = layout.empty() ? 0 : layout[0].size();

        float dx = r * 1.75f, dy = r * 1.5f;
        float gw = cols * dx + dx / 2, gh = rows * dy;
        float offX = (wsize.x - gw) / 2, offY = (wsize.y - gh) / 2;

        grid.assign(rows, std::vector<PentagonCell>(cols));
        int idcnt = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                char ch = layout[i][j];
                bool blk = (ch == '0');
                bool st = (ch == 'a');
                bool en = (ch == 'b');
                bool vc = (ch == 'v');

                float x = j * dx + (i % 2) * (dx / 2) + offX;
                float y = i * dy + offY;
                sf::Color col = blk ? sf::Color::Red : sf::Color::White;
                if (st) col = sf::Color::Blue;
                if (en) col = sf::Color::Green;
                if (vc) col = sf::Color(150, 150, 255);

                auto& cell = grid[i][j];
                cell.row = i; cell.col = j;
                cell.blocked = blk; cell.volatileCell = vc;
                cell.isStart = st; cell.isEnd = en;
                cell.id = (!blk || vc) ? idcnt : -1;
                cell.shape = createPentagon(x, y, r, col);

                if (cell.id >= 0) {
                    idToCell[idcnt] = &cell;
                    if (st) { playerCell = &cell; playerNodeId = idcnt; }
                    if (en)  endNodeId = idcnt;
                    ++idcnt;
                }
            }
        }
        adjacencyList.assign(idcnt, {});
        rebuildAdjacency();
    }

    void draw(sf::RenderWindow& w) {
        // Pentágonos
        for (auto& row : grid)
            for (auto& c : row)
                w.draw(c.shape);

        // Barra de carga
        float pct = std::min(1.f, moveCounter / (float)movesToBreak);
        sf::RectangleShape bg({ 200,20 }), fg({ 200 * pct,20 });
        bg.setPosition(20, 20); fg.setPosition(20, 20);
        bg.setFillColor({ 50,50,50 }); fg.setFillColor(sf::Color::Yellow);
        w.draw(bg); w.draw(fg);

        // Texto de turno
        if (fontLoaded_) {
            sf::Text t;
            t.setFont(font_);
            t.setCharacterSize(16);
            t.setFillColor(sf::Color::White);
            t.setString("Turno: " + std::to_string(turnCounter));
            t.setPosition(10, 40);
            w.draw(t);
        }
    }

    void drawPlayer(sf::RenderWindow& w) {
        if (!playerCell) return;
        sf::CircleShape pl(radius / 2);
        pl.setFillColor(sf::Color::Yellow);
        pl.setOrigin(pl.getRadius(), pl.getRadius());
        sf::Vector2f ctr = playerCell->shape.getPoint(0);
        for (int i = 1; i < 5; ++i) ctr += playerCell->shape.getPoint(i);
        ctr /= 5.f;
        pl.setPosition(ctr);
        w.draw(pl);
    }

    void handleMouseClick(sf::Vector2f mp) {
        // Romper muro
        for (auto& row : grid) {
            for (auto& c : row) {
                if (c.shape.getGlobalBounds().contains(mp)
                    && c.blocked && moveCounter >= movesToBreak) {
                    c.blocked = false;
                    c.shape.setFillColor(sf::Color::White);
                    c.id = adjacencyList.size();
                    idToCell[c.id] = &c;
                    adjacencyList.emplace_back();
                    moveCounter = 0;
                    rebuildAdjacency();
                    return;
                }
            }
        }
        // Mover jugador
        for (int nid : adjacencyList[playerNodeId]) {
            auto* n = idToCell[nid];
            if (n->shape.getGlobalBounds().contains(mp)) {
                playerCell = n;
                playerNodeId = n->id;
                ++moveCounter;
                ++turnCounter;
                rebuildAdjacency();
                return;
            }
        }
    }
};
