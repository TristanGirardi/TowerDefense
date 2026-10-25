#include "MapObserver.h"

#include "DemoText.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

// Size of one tile in the window, in pixels.
const float tileSize = 50.0f;

// Same colours as the game window.
const sf::Color grassColor(92, 150, 74);
const sf::Color pathColor(210, 185, 130);
const sf::Color entryColor(70, 170, 220);
const sf::Color exitColor(210, 70, 70);
const sf::Color towerColor(60, 110, 220);

MapObserver::MapObserver(const Map &map)
    : map(map), lastSeen(map), updateCount(0)
{
}

void MapObserver::update()
{
    ++updateCount;
    lastSeen = map; // remember the new map so draw() shows it
    std::cout << "  MapObserver: update #" << updateCount
              << ", the map is now:" << std::endl;
    lastSeen.display();
}

int MapObserver::getUpdateCount() const
{
    return updateCount;
}

// Draws the copy from the last update(), so a detached view stops changing.
void MapObserver::draw(sf::RenderWindow &window, const sf::Font *font,
                       sf::Vector2f position) const
{
    for (int y = 0; y < lastSeen.getHeight(); ++y)
    {
        for (int x = 0; x < lastSeen.getWidth(); ++x)
        {
            sf::RectangleShape tile({tileSize, tileSize});
            tile.setPosition({position.x + x * tileSize, position.y + y * tileSize});
            tile.setOutlineThickness(-1.0f);
            tile.setOutlineColor(sf::Color(0, 0, 0, 40));

            TileType type = lastSeen.getTile(x, y);
            if (type == TileType::SCENERY)
            {
                tile.setFillColor(grassColor);
            }
            else if (type == TileType::PATH)
            {
                tile.setFillColor(pathColor);
            }
            else if (type == TileType::ENTRY)
            {
                tile.setFillColor(entryColor);
            }
            else
            {
                tile.setFillColor(exitColor);
            }
            window.draw(tile);

            // A tower is a blue diamond in the middle of its tile.
            if (lastSeen.hasTower(x, y))
            {
                sf::CircleShape tower(16.0f, 4);
                tower.setOrigin({16.0f, 16.0f});
                tower.setPosition({position.x + (x + 0.5f) * tileSize,
                                   position.y + (y + 0.5f) * tileSize});
                tower.setFillColor(towerColor);
                tower.setOutlineColor(sf::Color(20, 20, 30));
                tower.setOutlineThickness(2.0f);
                window.draw(tower);
            }
        }
    }

    if (font == nullptr)
    {
        return;
    }

    // Legend under the map: a coloured square, then its name.
    float legendY = position.y + lastSeen.getHeight() * tileSize + 12.0f;
    const sf::Color colors[] = {grassColor, pathColor, entryColor, exitColor, towerColor};
    const std::string names[] = {"Grass", "Path", "Entry", "Exit", "Tower"};
    for (int i = 0; i < 5; ++i)
    {
        float legendX = position.x + i * 80.0f;
        sf::RectangleShape square({14.0f, 14.0f});
        square.setPosition({legendX, legendY + 2.0f});
        square.setFillColor(colors[i]);
        window.draw(square);
        drawDemoText(window, *font, names[i], legendX + 20.0f, legendY, 14,
                     sf::Color(200, 200, 210));
    }

    drawDemoText(window, *font, "Updates received: " + std::to_string(updateCount),
                 position.x, legendY + 26.0f, 14, sf::Color(255, 230, 90));
}

// What each press of Space does, in order.
const std::vector<std::string> mapSteps = {
    "Place a tower at (1, 1)",
    "Move the tower to (4, 3)",
    "Place a second tower at (6, 1)",
    "Try to move a tower onto the path",
    "Remove the tower at (6, 1)",
    "Turn tile (1, 0) into grass (cuts the path)",
    "Detach the view, then move the tower",
};

// Does one step. The driver only changes the map; the view is updated by
// the map's notify(), never by the driver.
void doMapStep(std::size_t step, Map &map, MapObserver &view)
{
    std::cout << "Driver: " << mapSteps[step] << std::endl;
    switch (step)
    {
    case 0:
        map.placeTower(1, 1);
        break;
    case 1:
        map.moveTower(1, 1, 4, 3);
        break;
    case 2:
        map.placeTower(6, 1);
        break;
    case 3:
        if (!map.moveTower(4, 3, 3, 2))
        {
            std::cout << "Driver: refused, (3, 2) is path. Nothing changed,"
                         " so no notify and no update." << std::endl;
        }
        break;
    case 4:
        map.removeTower(6, 1);
        break;
    case 5:
    {
        map.setTile(1, 0, TileType::SCENERY);
        bool valid = map.validate();
        std::cout << "Driver: is the map still valid? " << (valid ? "yes" : "no")
                  << std::endl;
        break;
    }
    case 6:
        map.detach(&view);
        map.moveTower(4, 3, 0, 4);
        std::cout << "Driver: the tower is at (0, 4) now, but the view still shows"
                     " (4, 3) because it isn't attached anymore." << std::endl;
        break;
    }
}

void runMapObserverDemo()
{
    std::cout << "========== Part 2: Map observer demo ==========" << std::endl;

    // 8 x 5 map with a path from the entry (top-left) to the exit.
    Map map(8, 5);
    map.setTile(0, 0, TileType::ENTRY);
    map.setTile(1, 0, TileType::PATH);
    map.setTile(2, 0, TileType::PATH);
    map.setTile(2, 1, TileType::PATH);
    map.setTile(2, 2, TileType::PATH);
    map.setTile(3, 2, TileType::PATH);
    map.setTile(4, 2, TileType::PATH);
    map.setTile(5, 2, TileType::PATH);
    map.setTile(5, 3, TileType::PATH);
    map.setTile(5, 4, TileType::PATH);
    map.setTile(6, 4, TileType::PATH);
    map.setTile(7, 4, TileType::EXIT);

    MapObserver view(map);
    map.attach(&view); // plug the view into the map
    std::cout << "Driver: made an 8x5 map and attached a MapObserver." << std::endl;
    std::cout << "Press Space in the window to do the next step." << std::endl;

    sf::RenderWindow window(sf::VideoMode({860, 360}), "Part 2: Map observer",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    sf::Font font;
    const sf::Font *fontPtr = loadDemoFont(font) ? &font : nullptr;
    std::size_t nextStep = 0;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto *key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
                else if (key->code == sf::Keyboard::Key::Space)
                {
                    if (nextStep < mapSteps.size())
                    {
                        doMapStep(nextStep, map, view);
                        ++nextStep;
                    }
                    else
                    {
                        window.close(); // all done
                    }
                }
            }
        }

        window.clear(sf::Color(20, 20, 24));
        view.draw(window, fontPtr, {20.0f, 20.0f});

        // The text on the right is the driver's own, not part of the view.
        if (fontPtr != nullptr)
        {
            float x = 450.0f;
            drawDemoText(window, font, "The map is the Observable. The picture on the",
                         x, 20.0f, 14, sf::Color(170, 170, 185));
            drawDemoText(window, font, "left is a MapObserver attached to it. Each step",
                         x, 38.0f, 14, sf::Color(170, 170, 185));
            drawDemoText(window, font, "changes the map, the map calls notify(), and",
                         x, 56.0f, 14, sf::Color(170, 170, 185));
            drawDemoText(window, font, "the view redraws (the console prints it too).",
                         x, 74.0f, 14, sf::Color(170, 170, 185));

            drawDemoText(window, font,
                         nextStep < mapSteps.size() ? "Press Space to:" : "Done. Press Space to close.",
                         x, 108.0f, 16, sf::Color::White);
            for (std::size_t i = 0; i < mapSteps.size(); ++i)
            {
                sf::Color color = sf::Color(200, 200, 210); // still to do
                if (i < nextStep)
                {
                    color = sf::Color(110, 110, 120); // already done
                }
                else if (i == nextStep)
                {
                    color = sf::Color(255, 230, 90); // next one
                }
                drawDemoText(window, font, std::to_string(i + 1) + ". " + mapSteps[i],
                             x, 138.0f + i * 22.0f, 14, color);
            }
        }
        else
        {
            window.setTitle("Part 2: Map observer - press Space (step " +
                            std::to_string(nextStep + 1) + ")");
        }
        window.display();
    }

    map.detach(&view); // in case the window was closed early
    std::cout << "Map observer demo finished." << std::endl << std::endl;
}
