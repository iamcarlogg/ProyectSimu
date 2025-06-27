#include "./Simu/PentagonGrid.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>

std::string wrapText(const std::string& text, const sf::Font& font, unsigned int charSize, float maxWidth) {
    std::istringstream iss(text);
    std::string word, output, line;
    while (iss >> word) {
        std::string testLine = line.empty() ? word : line + " " + word;
        sf::Text testText(testLine, font, charSize);
        if (testText.getLocalBounds().width > maxWidth) {
            if (!output.empty()) output += "\n";
            output += line;
            line = word;
        }
        else {
            line = testLine;
        }
    }
    if (!line.empty()) {
        if (!output.empty()) output += "\n";
        output += line;
    }
    return output;
}

int main() {
    sf::RenderWindow window({ 1920, 1080 }, "Laberinto");
    sf::View view = window.getDefaultView();
    float zoomLevel = 1.0f;

    std::string mapPath;
    std::cout << "Ingrese la ruta absoluta o relativa del archivo de mapa (ej: C:\\Users\\tu_usuario\\Documents\\map.txt): ";
    std::getline(std::cin, mapPath);

    if (mapPath.empty()) {
        std::cerr << "No se proporcionó ninguna ruta. Saliendo...\n";
        return 1;
    }

    auto layout = PentagonGrid::loadLayoutFromFile(mapPath);
    PentagonGrid grid(layout, 15.f, window.getSize(), &window);

    // Fuentes y HUD
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
        std::cerr << "No pude cargar arial.ttf\n";

    std::string historia =
        "Es el 2225, en Hellador.\n"
        "Eres Pepe el cyberdinosaurio rescatado de la muerte causada por algoritmos en 2024.\n"
        "Estás modificado genéticamente porque tienes que ayudar a la resistencia para acabar con la dictadura del ejército del dictador, ya que en tus genes tienes todos los datos necesarios para acabar con esta dictadura.\n"
        "Luego de tu captura lograste llegar a los ductos de ventilación del KEKOT, pero cada cierto tiempo hay unas plataformas que suben y te tapan tu camino, pero si subes a uno de ellos puedes volver a bajar.\n"
        "A su vez, al ser un dinosaurio modificado genéticamente tienes la habilidad de generar pequeños overflows cada 4 turnos, lo que te permitirá romper un muro.\n"
        "Trata de llegar al fin del laberinto que se forma en los ductos de ventilación.\n\n"
        "Buena suerte, recuerda los consejos de Roque Dalton y salva a nuestra patria.";

    float modalW = 750.f, modalH = 500.f;
    sf::RectangleShape storyModal({ modalW, modalH });
    storyModal.setFillColor(sf::Color(28, 34, 50));
    storyModal.setOutlineColor(sf::Color(90, 200, 250));
    storyModal.setOutlineThickness(3.f);
    storyModal.setPosition((window.getSize().x - modalW) / 2.f, (window.getSize().y - modalH) / 2.f);

    sf::Text titulo("CiberDinosaurio", font, 40);
    titulo.setFillColor(sf::Color(0, 245, 212));
    titulo.setStyle(sf::Text::Bold);
    sf::FloatRect tRect = titulo.getLocalBounds();
    titulo.setPosition(
        storyModal.getPosition().x + (modalW - tRect.width) / 2.f,
        storyModal.getPosition().y + 24.f
    );

    float marginX = 38.f;
    float marginY = 80.f; // deja espacio arriba para el título
    std::string historiaWrap = wrapText(historia, font, 22, modalW - 2 * marginX);

    sf::Text storyText(historiaWrap, font, 22);
    storyText.setFillColor(sf::Color(245, 245, 245));
    storyText.setLineSpacing(1.18f);
    storyText.setPosition(storyModal.getPosition().x + marginX, storyModal.getPosition().y + marginY);

    // Botón ahora va DEBAJO del recuadro
    float btnWidth = 270.f, btnHeight = 58.f;
    float btnX = storyModal.getPosition().x + (modalW - btnWidth) / 2.f;
    float btnY = storyModal.getPosition().y + modalH + 20.f;
    sf::RectangleShape storyBtn({ btnWidth, btnHeight });
    storyBtn.setPosition(btnX, btnY);
    storyBtn.setFillColor(sf::Color::White);
    storyBtn.setOutlineColor(sf::Color(0, 245, 212));
    storyBtn.setOutlineThickness(2.2f);

    sf::Text storyBtnText("Comenzar aventura", font, 26);
    storyBtnText.setFillColor(sf::Color(0, 245, 212));
    sf::FloatRect btnTextRect = storyBtnText.getLocalBounds();
    storyBtnText.setPosition(
        btnX + (btnWidth - btnTextRect.width) / 2.f,
        btnY + (btnHeight - btnTextRect.height) / 2.f - 6.f
    );

    bool showStory = true;

    // Botón “Autocompletar”
    sf::RectangleShape btn({ 150, 40 });
    btn.setPosition(10, 100);
    btn.setFillColor({ 50,50,50 });
    sf::Text btnText("Autocompletar", font, 18);
    btnText.setPosition(15, 102);
    btnText.setFillColor(sf::Color::White);

    // Botón de reinicio
    sf::RectangleShape restartButton({ 120.f, 30.f });
    restartButton.setPosition(window.getSize().x - 140.f, 20.f);
    sf::Text restartText("Reiniciar", font, 16);
    restartText.setFillColor(sf::Color(0, 245, 212));
    restartText.setPosition(window.getSize().x - 120.f, 26.f);

    // Modal de victoria
    sf::RectangleShape modal({ 400.f, 200.f });
    modal.setFillColor(sf::Color(28, 34, 50));
    modal.setOutlineColor(sf::Color(90, 200, 250));
    modal.setOutlineThickness(3.f);
    modal.setPosition((window.getSize().x - 400.f) / 2.f, (window.getSize().y - 200.f) / 2.f);

    sf::Text msg("¡Has escapado del laberinto!", font, 20);
    msg.setFillColor(sf::Color(0, 245, 212));
    msg.setStyle(sf::Text::Bold);
    sf::FloatRect msgBounds = msg.getLocalBounds();
    msg.setOrigin(msgBounds.width / 2, msgBounds.height / 2);
    msg.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 40.f);

    sf::RectangleShape replayBtn({ 180.f, 40.f });
    replayBtn.setPosition(window.getSize().x / 2.f - 90.f, window.getSize().y / 2.f + 20.f);
    sf::Text replayText("Jugar de nuevo", font, 16);
    replayText.setFillColor(sf::Color(0, 245, 212));
    sf::FloatRect tb = replayText.getLocalBounds();
    replayText.setOrigin(tb.width / 2.f, tb.height / 2.f);
    replayText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f + 40.f);

    sf::RectangleShape exitBtn({ 180.f, 40.f });
    exitBtn.setPosition(window.getSize().x / 2.f - 90.f, window.getSize().y / 2.f + 70.f);
    sf::Text exitText("Salir del juego", font, 16);
    exitText.setFillColor(sf::Color(255, 80, 80));
    sf::FloatRect etb = exitText.getLocalBounds();
    exitText.setOrigin(etb.width / 2.f, etb.height / 2.f);
    exitText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f + 90.f);

    bool autoMode = false;
    std::vector<sf::Vector2i> path;
    std::size_t pathStep = 0;
    sf::Clock autoClock;

    while (window.isOpen()) {
        window.setView(view); 
        sf::Event ev;
        while (window.pollEvent(ev)) {

            if (showStory && ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mp = window.mapPixelToCoords({ ev.mouseButton.x, ev.mouseButton.y });
                if (storyBtn.getGlobalBounds().contains(mp)) {
                    showStory = false;
                    continue;
                }
            }
            if (showStory)
                continue;

            if (ev.type == sf::Event::Closed)
                window.close();

            else if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
                // 1. Coordenada de pantalla (pixel, para botones/HUD)
                sf::Vector2i mpScreen(ev.mouseButton.x, ev.mouseButton.y);

                // 2. HUD/Botones
                if (grid.isGameFinished()) {
                    if (replayBtn.getGlobalBounds().contains(sf::Vector2f(mpScreen))) {
                        grid.resetGame();
                        continue;
                    }
                    else if (exitBtn.getGlobalBounds().contains(sf::Vector2f(mpScreen))) {
                        window.close();
                        continue;
                    }
                }
                if (restartButton.getGlobalBounds().contains(sf::Vector2f(mpScreen))) {
                    grid.resetGame();
                    continue;
                }
                if (btn.getGlobalBounds().contains(sf::Vector2f(mpScreen))) {
                    path = grid.solveWithBFS();
                    if (path.empty())
                        std::cout << "¡No hay ruta!\n";
                    else {
                        autoMode = true;
                        pathStep = 1;
                        autoClock.restart();
                    }
                }
                // 3. Click en el grid/laberinto (convertir a coords de mundo)
                else if (!grid.isGameFinished()) {
                    sf::Vector2f worldPos = window.mapPixelToCoords(mpScreen);
                    grid.handleMouseClick(worldPos);
                }
            }

            // --- ZOOM CON RUEDA DEL RATÓN ---
            else if (ev.type == sf::Event::MouseWheelScrolled) {
                if (ev.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    float delta = ev.mouseWheelScroll.delta;
                    if (delta > 0) zoomLevel *= 0.9f;
                    else if (delta < 0) zoomLevel *= 1.1f;
                    if (zoomLevel < 0.1f) zoomLevel = 0.1f;
                    if (zoomLevel > 3.0f) zoomLevel = 3.0f;
                    view.setSize(window.getDefaultView().getSize());
                    view.zoom(zoomLevel);
                }
            }
        }

        if (showStory) {
            window.clear(sf::Color(10, 15, 30));
            sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
            overlay.setFillColor(sf::Color(0, 0, 0, 180));
            window.draw(overlay);
            window.draw(storyModal);
            window.draw(titulo);
            window.draw(storyText);
            window.draw(storyBtn);
            window.draw(storyBtnText);
            window.display();
            continue;
        }

        // Modo automático: avanzamos un paso cada 0.5s
        if (autoMode && pathStep < path.size()) {
            if (autoClock.getElapsedTime().asSeconds() > 0.5f) {
                auto [r, c] = path[pathStep];
                auto& cell = grid.getCell(r, c);
                auto getCenter = [&](const sf::ConvexShape& s) {
                    sf::Vector2f ctr = s.getPoint(0);
                    for (int i = 1; i < s.getPointCount(); ++i)
                        ctr += s.getPoint(i);
                    return ctr / static_cast<float>(s.getPointCount());
                    };
                sf::Vector2f clickPos = getCenter(cell.shape);

                if (cell.blocked) {
                    grid.handleMouseClick(clickPos);
                }
                else {
                    grid.handleMouseClick(clickPos);
                    ++pathStep;
                }
                if (grid.getEndNodeId() == grid.getPlayerNodeId()) {
                    autoMode = false;
                }
                autoClock.restart();
            }
        }

        // --- DIBUJAR ---
        window.setView(view); // Laberinto con zoom
        window.clear(sf::Color(10, 15, 30));
        grid.draw(window);
        grid.drawPlayer(window);

        window.setView(window.getDefaultView()); // HUD SIEMPRE FIJO
        window.draw(btn);
        window.draw(btnText);
        window.draw(restartButton);
        window.draw(restartText);

        // Barra de carga (HUD)
        float pct = std::min(1.f, grid.getMoveCounter() / float(grid.getMovesToBreak()));
        sf::RectangleShape bg({ 200, 20 }), fg({ 200 * pct, 20 });
        bg.setPosition(20, 20); fg.setPosition(20, 20);
        bg.setFillColor({ 50, 50, 50 }); fg.setFillColor(sf::Color::Yellow);
        window.draw(bg); window.draw(fg);

        // Texto de turno (HUD)
        sf::Text turnText("Turno: " + std::to_string(grid.getTurnCounter()), font, 16);
        turnText.setFillColor(sf::Color::White);
        turnText.setPosition(10, 40);
        window.draw(turnText);

        if (grid.isGameFinished()) {
            // Fondo semi-transparente
            sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
            overlay.setFillColor(sf::Color(0, 0, 0, 180));
            window.draw(overlay);
            window.draw(modal);
            window.draw(msg);
            window.draw(replayBtn);
            window.draw(replayText);
            window.draw(exitBtn);
            window.draw(exitText);
        }

        window.display();
    }
    return 0;
}
