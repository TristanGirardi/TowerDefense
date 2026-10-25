#include "MapObserver.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

// Size of one tile in the window, in pixels.
const float tileSize = 60.0f;

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
void MapObserver::draw(sf::RenderWindow &window) const
{
    for (int y = 0; y < lastSeen.getHeight(); ++y)
    {
        for (int x = 0; x < lastSeen.getWidth(); ++x)
        {
            // Same colours as the game window.
            sf::RectangleShape tile({tileSize, tileSize});
            tile.setPosition({x * tileSize, y * tileSize});
            tile.setOutlineThickness(-1.0f);
            tile.setOutlineColor(sf::Color(0, 0, 0, 40));

            TileType type = lastSeen.getTile(x, y);
            if (type == TileType::SCENERY)
            {
                tile.setFillColor(sf::Color(92, 150, 74)); // grass
            }
            else if (type == TileType::PATH)
            {
                tile.setFillColor(sf::Color(210, 185, 130)); // sand
            }
            else if (type == TileType::ENTRY)
            {
                tile.setFillColor(sf::Color(70, 170, 220)); // blue
            }
            else
            {
                tile.setFillColor(sf::Color(210, 70, 70)); // red exit
            }
            window.draw(tile);

            // A tower is a blue diamond in the middle of its tile.
            if (lastSeen.hasTower(x, y))
            {
                sf::CircleShape tower(20.0f, 4);
                tower.setOrigin({20.0f, 20.0f});
                tower.setPosition({(x + 0.5f) * tileSize, (y + 0.5f) * tileSize});
                tower.setFillColor(sf::Color(60, 110, 220));
                tower.setOutlineColor(sf::Color(20, 20, 30));
                tower.setOutlineThickness(2.0f);
                window.draw(tower);
            }
        }
    }
}

// What each press of Space does, in order.
const std::vector<std::string> mapSteps = {
    "Place a tower at (1, 1)",
    "Move the tower to (4, 3)",
    "Place a second tower at (6, 1)",
    "Try to move a tower onto the path",
    "Remove the tower at (6, 1)",
    "Make tile (1, 0) scenery again (path is cut)",
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

    sf::RenderWindow window(sf::VideoMode({480, 300}), "Part 2: Map observer",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
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

        // The title tells you what the next Space press will do.
        if (nextStep < mapSteps.size())
        {
            window.setTitle("Part 2: Map observer - Space: " + mapSteps[nextStep]);
        }
        else
        {
            window.setTitle("Part 2: Map observer - done, press Space to close");
        }

        window.clear(sf::Color(20, 20, 24));
        view.draw(window);
        window.display();
    }

    map.detach(&view); // in case the window was closed early
    std::cout << "Map observer demo finished." << std::endl << std::endl;
}
