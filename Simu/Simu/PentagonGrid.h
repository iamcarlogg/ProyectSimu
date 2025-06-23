#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <queue>
#include <array>

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
    sf::Vector2u windowSize;
    std::vector<std::string> originalLayout;
    sf::RenderWindow* windowRef = nullptr;
    int rows = 0, cols = 0;
    float radius = 0.f;
    bool gameFinished = false;
    int playerNodeId = -1;
    int endNodeId = -1;
    PentagonCell* playerCell = nullptr;
    int moveCounter = 0;
    int movesToBreak = 4;
    int turnCounter = 0;

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
        // 1) Toggle de células volátiles
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
        // 2) Limpiar vecinos y lista
        for (auto& row : grid)
            for (auto& cell : row)
                cell.neighbors.clear();
        for (auto& lst : adjacencyList)
            lst.clear();
        // 3) Reconstruir según blocked + jugador siempre navegable
        auto isNav = [&](const PentagonCell& c) {
            if (&c == playerCell) return true;
            return !c.blocked;
            };
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                auto& c = grid[i][j];
                if (c.id < 0 || !isNav(c)) continue;
                std::vector<std::pair<int, int>> dirs = {
                    {i, j - 1},{i, j + 1},{i - 1, j},{i + 1, j}
                };
                if (i % 2 == 0) { dirs.push_back({ i - 1,j - 1 }); dirs.push_back({ i + 1,j - 1 }); }
                else { dirs.push_back({ i - 1,j + 1 }); dirs.push_back({ i + 1,j + 1 }); }
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
        if (!f.is_open())
            throw std::runtime_error("No se pudo abrir el archivo de mapa");
        std::vector<std::string> lay;
        std::string line;
        while (std::getline(f, line))
            if (!line.empty())
                lay.push_back(line);
        return lay;
    }

    PentagonGrid(const std::vector<std::string>& layout, float r, sf::Vector2u wsize, sf::RenderWindow* w)
        : radius(r), windowSize(wsize), originalLayout(layout), windowRef(w)
    {
        fontLoaded_ = font_.loadFromFile("arial.ttf");
        if (!fontLoaded_)
            std::cerr << "Warning: no se pudo cargar arial.ttf\n";

        rows = layout.size();
        cols = layout.empty() ? 0 : layout[0].size();
        float dx = r * 2.1f, dy = r * 1.8f;
        float gw = cols * dx + dx / 2, gh = rows * dy;
        float offX = (wsize.x - gw) / 2, offY = (wsize.y - gh) / 2;

        grid.assign(rows, std::vector<PentagonCell>(cols));
        int idcnt = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                char ch = layout[i][j];
                bool blk = (ch == '0'),
                    st = (ch == 'a'),
                    en = (ch == 'b'),
                    vc = (ch == 'v');
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
                    if (en) endNodeId = idcnt;
                    ++idcnt;
                }
            }
        }
        adjacencyList.assign(idcnt, {});
        rebuildAdjacency();
    }

    void resetGame() {
        *this = PentagonGrid(originalLayout, radius, windowSize, windowRef);
        gameFinished = false;
    }

    void draw(sf::RenderWindow& w) {
        for (auto& row : grid) {
            for (auto& c : row) {
                if (c.volatileCell) {
                    // color volátil se asigna en rebuildAdjacency
                }
                else if (c.blocked)
                    c.shape.setFillColor(sf::Color(28, 34, 50));
                else if (c.isStart)
                    c.shape.setFillColor(sf::Color(0, 245, 212));
                else if (c.isEnd)
                    c.shape.setFillColor(sf::Color(255, 78, 205));
                else
                    c.shape.setFillColor(sf::Color(46, 58, 89));

                if (&c == playerCell) {
                    c.shape.setOutlineColor(sf::Color(0, 255, 255));
                    c.shape.setOutlineThickness(4.f);
                }
                else {
                    c.shape.setOutlineColor(sf::Color(100, 100, 100, 120));
                    c.shape.setOutlineThickness(1.5f);
                }
                w.draw(c.shape);
            }
        }
    }

    void drawPlayer(sf::RenderWindow& w) {
        if (!playerCell || gameFinished) return;
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
        if (gameFinished) return;

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
                if (n->isEnd) {
                    gameFinished = true;
                }
                return;
            }
        }
    }

    // --- Métodos utilitarios ---
    PentagonCell& getCell(int r, int c) { return grid[r][c]; }
    int getPlayerNodeId() const { return playerNodeId; }
    int getEndNodeId()    const { return endNodeId; }
    int getTurnCounter()  const { return turnCounter; }
    int getMoveCounter()  const { return moveCounter; }
    int getMovesToBreak() const { return movesToBreak; }
    bool isGameFinished() const { return gameFinished; }
    void setGameFinished(bool val) { gameFinished = val; }

    // --- BFS ---
    std::vector<sf::Vector2i> solveWithBFS() {
        struct State { int r, c, tsb, par; };
        struct Par { int pr, pc, ptsb, ppar; };

        int sr = 0, sc = 0, er = 0, ec = 0;
        for (int i = 0;i < rows;i++) for (int j = 0;j < cols;j++) {
            if (grid[i][j].isStart) { sr = i; sc = j; }
            if (grid[i][j].isEnd) { er = i; ec = j; }
        }
        int MTB = movesToBreak;
        std::vector vis(rows, std::vector(cols,
            std::vector(MTB + 1, std::array<bool, 2>{false, false})
        ));
        std::vector parents(rows, std::vector(cols,
            std::vector(MTB + 1, std::array<Par, 2>{})
        ));
        std::queue<State> Q;
        State init{ sr,sc, moveCounter, turnCounter % 2 };
        vis[sr][sc][init.tsb][init.par] = true;
        Q.push(init);

        auto neighbors = [&](int i, int j) {
            std::vector<std::pair<int, int>> d = { {i,j - 1},{i,j + 1},{i - 1,j},{i + 1,j} };
            if (i % 2 == 0) { d.push_back({ i - 1,j - 1 }); d.push_back({ i + 1,j - 1 }); }
            else { d.push_back({ i - 1,j + 1 }); d.push_back({ i + 1,j + 1 }); }
            return d;
            };

        while (!Q.empty()) {
            auto [r, c, tsb, par] = Q.front(); Q.pop();
            if (r == er && c == ec) {
                std::vector<sf::Vector2i> path;
                int cr = r, cc = c, ctsb = tsb, cpar = par;
                while (!(cr == sr && cc == sc && ctsb == init.tsb && cpar == init.par)) {
                    path.push_back({ cr,cc });
                    auto p = parents[cr][cc][ctsb][cpar];
                    cr = p.pr; cc = p.pc; ctsb = p.ptsb; cpar = p.ppar;
                }
                path.push_back({ sr,sc });
                std::reverse(path.begin(), path.end());
                return path;
            }
            // 1) mover sin romper
            for (auto [nr, nc] : neighbors(r, c)) {
                if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
                auto& cell = grid[nr][nc];
                bool statBlk = cell.blocked && !cell.volatileCell;
                bool volBlk = cell.volatileCell && par == 0;
                if (statBlk || volBlk) continue;
                int ntsb = std::min(tsb + 1, MTB);
                int npar = 1 - par;
                if (!vis[nr][nc][ntsb][npar]) {
                    vis[nr][nc][ntsb][npar] = true;
                    parents[nr][nc][ntsb][npar] = { r,c,tsb,par };
                    Q.push({ nr,nc,ntsb,npar });
                }
            }
            // 2) romper muro
            if (tsb >= MTB) {
                for (auto [nr, nc] : neighbors(r, c)) {
                    if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
                    auto& cell = grid[nr][nc];
                    if (!cell.blocked || cell.volatileCell) continue;
                    int ntsb = 0, npar = 1 - par;
                    if (!vis[nr][nc][ntsb][npar]) {
                        vis[nr][nc][ntsb][npar] = true;
                        parents[nr][nc][ntsb][npar] = { r,c,tsb,par };
                        Q.push({ nr,nc,ntsb,npar });
                    }
                }
            }
        }
        return {}; // sin ruta
    }

    void setPlayerPosition(int id) {
        if (auto it = idToCell.find(id); it != idToCell.end()) {
            for (auto& row : grid)
                for (auto& c : row)
                    c.isStart = false;
            it->second->isStart = true;
            playerCell = it->second;
            playerNodeId = id;
        }
    }
};
