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

    PentagonGrid(const std::vector<std::string>& layout, float r, sf::Vector2u wsize, sf::RenderWindow* w)
        : radius(r), windowSize(wsize), originalLayout(layout), windowRef(w)
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
                sf::Color col = blk ? sf::Color(28, 34, 50) : sf::Color(46, 58, 89);
                if (st) col = sf::Color(0, 245, 212);
                if (en) col = sf::Color(255, 78, 205);
                if (vc) col = sf::Color(90, 200, 250);

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

    void resetGame() {
        *this = PentagonGrid(originalLayout, radius, windowSize, windowRef);
        gameFinished = false;
    }

    void draw(sf::RenderWindow& w) {
        sf::Vector2i mousePosI = sf::Mouse::getPosition(*windowRef);
        sf::Vector2f mousePos = w.mapPixelToCoords(mousePosI);

        // Pentagonos
        for (auto& row : grid) {
            for (auto& c : row) {
                // Resaltado si es la celda actual del jugador
                if (&c == playerCell) {
                    c.shape.setOutlineColor(sf::Color(0, 255, 255)); // cian brillante
                    c.shape.setOutlineThickness(4.f);
                }
                else {
                    c.shape.setOutlineColor(sf::Color(100, 100, 100));
                    c.shape.setOutlineThickness(1.5f);
                }

                w.draw(c.shape);
            }
        }

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

        // Boton de autocompletado
        sf::RectangleShape autoButton({ 150.f, 30.f });
        autoButton.setPosition(windowSize.x - 300.f, 20.f); 

        bool hoveringAuto = autoButton.getGlobalBounds().contains(mousePos);

        autoButton.setFillColor(hoveringAuto ? sf::Color(38, 44, 60) : sf::Color(28, 34, 50));
        autoButton.setOutlineColor(hoveringAuto ? sf::Color::White : sf::Color(90, 200, 250));
        autoButton.setOutlineThickness(2.f);
        w.draw(autoButton);

        sf::Text autoText("Autocompletado", font_, 16);
        autoText.setFillColor(sf::Color(0, 245, 212));
        autoText.setPosition(windowSize.x - 280.f, 26.f); 
        w.draw(autoText);

        // Boton de reinicio 
        sf::RectangleShape restartButton({ 120.f, 30.f });
        restartButton.setPosition(windowSize.x - 140.f, 20.f);

        bool hoveringRestart = restartButton.getGlobalBounds().contains(mousePos);

        restartButton.setFillColor(hoveringRestart ? sf::Color(38, 44, 60) : sf::Color(28, 34, 50));
        restartButton.setOutlineColor(hoveringRestart ? sf::Color::White : sf::Color(90, 200, 250));
        restartButton.setOutlineThickness(2.f);
        w.draw(restartButton);

        sf::Font f; f.loadFromFile("arial.ttf");
        sf::Text restartText("Reiniciar", f, 16);
        restartText.setFillColor(sf::Color(0, 245, 212));
        restartText.setPosition(windowSize.x - 120.f, 26.f); 
        w.draw(restartText);

        if (gameFinished) {
            sf::Font f;
            f.loadFromFile("arial.ttf");

            // Fondo semi-transparente
            sf::RectangleShape overlay(sf::Vector2f(windowSize.x, windowSize.y));
            overlay.setFillColor(sf::Color(0, 0, 0, 180)); 
            w.draw(overlay);

            // Ventana centrada estilo modal
            sf::RectangleShape modal({ 400.f, 200.f });
            modal.setFillColor(sf::Color(28, 34, 50));
            modal.setOutlineColor(sf::Color(90, 200, 250));
            modal.setOutlineThickness(3.f);
            modal.setPosition((windowSize.x - 400.f) / 2.f, (windowSize.y - 200.f) / 2.f);
            w.draw(modal);

            // Mensaje de escape
            sf::Text msg("¡Has escapado del laberinto!", f, 20);
            msg.setFillColor(sf::Color(0, 245, 212));
            msg.setStyle(sf::Text::Bold);
            sf::FloatRect msgBounds = msg.getLocalBounds();
            msg.setOrigin(msgBounds.width / 2, msgBounds.height / 2);
            msg.setPosition(windowSize.x / 2.f, windowSize.y / 2.f - 40.f);
            w.draw(msg);

            // Botón jugar de nuevo
            sf::RectangleShape replayBtn({ 180.f, 40.f });
            replayBtn.setPosition(windowSize.x / 2.f - 90.f, windowSize.y / 2.f + 20.f);
            bool hoveringReplay = replayBtn.getGlobalBounds().contains(mousePos);
            replayBtn.setFillColor(hoveringReplay ? sf::Color(32, 40, 60) : sf::Color(20, 25, 40));
            replayBtn.setOutlineColor(hoveringReplay ? sf::Color::White : sf::Color(90, 200, 250));
            replayBtn.setOutlineThickness(2.f);
            w.draw(replayBtn);

            sf::Text replayText("Jugar de nuevo", f, 16);
            replayText.setFillColor(sf::Color(0, 245, 212));
            sf::FloatRect tb = replayText.getLocalBounds();
            replayText.setOrigin(tb.width / 2.f, tb.height / 2.f);
            replayText.setPosition(windowSize.x / 2.f, windowSize.y / 2.f + 40.f);
            w.draw(replayText);

            // Botón para salir del juego
            sf::RectangleShape exitBtn({ 180.f, 40.f });
            exitBtn.setPosition(windowSize.x / 2.f - 90.f, windowSize.y / 2.f + 70.f);
            bool hoveringExit = exitBtn.getGlobalBounds().contains(mousePos);
            exitBtn.setFillColor(hoveringExit ? sf::Color(40, 20, 20) : sf::Color(20, 25, 40));
            exitBtn.setOutlineColor(hoveringExit ? sf::Color::White : sf::Color(255, 80, 80));
            exitBtn.setOutlineThickness(2.f);
            w.draw(exitBtn);

            sf::Text exitText("Salir del juego", f, 16);
            exitText.setFillColor(sf::Color(255, 80, 80));
            sf::FloatRect etb = exitText.getLocalBounds();
            exitText.setOrigin(etb.width / 2.f, etb.height / 2.f);
            exitText.setPosition(windowSize.x / 2.f, windowSize.y / 2.f + 90.f);
            w.draw(exitText);
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

        if (gameFinished) {
            sf::FloatRect replayBounds(windowSize.x / 2.f - 90.f, windowSize.y / 2.f + 20.f, 180.f, 40.f);
            sf::FloatRect exitBounds(windowSize.x / 2.f - 90.f, windowSize.y / 2.f + 70.f, 180.f, 40.f);

            if (replayBounds.contains(mp)) {
                resetGame();
            }
            else if (exitBounds.contains(mp)) {
                if (windowRef)
                    windowRef->close();
            }
            return;
        }

        // Reiniciar juego
        sf::FloatRect restartBounds(windowSize.x - 140.f, 20.f, 120.f, 30.f);
        if (restartBounds.contains(mp)) {
            resetGame();
            return;
        }

        // Romper muro
        for (auto& row : grid) {
            for (auto& c : row) {
                if (c.shape.getGlobalBounds().contains(mp)
                    && c.blocked && moveCounter >= movesToBreak) {
                    c.blocked = false;
                    c.shape.setFillColor(sf::Color(46, 58, 89)); 
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
};

