#include "TowerObserver.h"

#include "Critter.h"

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>


// The observer


TowerObserver::TowerObserver(const Tower &tower) : tower(tower), updateCount(0)
{
    readTower(); // so there's something to show before the first change
}

void TowerObserver::update()
{
    ++updateCount;
    readTower();

    std::cout << "  TowerObserver: update #" << updateCount << ", " << name
              << " is now level " << level << "/" << maxLevel << " at (" << x
              << ", " << y << ") with power " << stats.power << std::endl;
}

int TowerObserver::getUpdateCount() const
{
    return updateCount;
}

void TowerObserver::readTower()
{
    name = tower.getName();
    level = tower.getLevel();
    maxLevel = tower.getMaxLevel();
    x = tower.getX();
    y = tower.getY();
    stats = tower.getStats();
    upgradeCost = tower.getUpgradeCost();
    refundValue = tower.getRefundValue();
}


// Drawing


namespace
{
// Same colours and shapes as the towers in the game window.
struct Look
{
    sf::Color color;
    std::size_t points; // 3 = triangle, 4 = diamond, 30 = circle
};

Look lookFor(const std::string &name)
{
    if (name == "Splash Tower")
    {
        return {sf::Color(240, 140, 40), 30};
    }
    if (name == "Hunter Tower")
    {
        return {sf::Color(160, 70, 200), 3};
    }
    return {sf::Color(60, 110, 220), 4}; // Direct Tower
}

// Writes one line of text with its top-left corner at (x, y).
void drawText(sf::RenderWindow &window, const sf::Font &font,const std::string &text, float x, float y, unsigned size,sf::Color color)
{
    sf::Text label(font, text, size);
    label.setFillColor(color);
    label.setPosition({x, y});
    window.draw(label);
}

// Turns a number into text: 3 stays "3", 3.5 stays "3.5".
std::string toText(float value)
{
    std::ostringstream out;
    out << value;
    return out.str();
}
}

void TowerObserver::draw(sf::RenderWindow &window, const sf::Font *font,sf::Vector2f position) const
{
    const float left = position.x;
    const float top = position.y;

    sf::RectangleShape panel({320.0f, 250.0f});
    panel.setPosition(position);
    panel.setFillColor(sf::Color(30, 32, 40));
    panel.setOutlineColor(sf::Color(90, 90, 100));
    panel.setOutlineThickness(1.0f);
    window.draw(panel);

    // The tower itself.
    Look look = lookFor(name);
    sf::CircleShape icon(24.0f, look.points);
    icon.setOrigin({24.0f, 24.0f});
    icon.setPosition({left + 44.0f, top + 44.0f});
    icon.setFillColor(look.color);
    icon.setOutlineColor(sf::Color(20, 20, 30));
    icon.setOutlineThickness(2.0f);
    window.draw(icon);

    // One dot per level: yellow up to the current level, grey after that.
    for (int i = 0; i < maxLevel; ++i)
    {
        sf::CircleShape dot(4.0f);
        dot.setOrigin({4.0f, 4.0f});
        float dotX = left + 44.0f + (i - (maxLevel - 1) / 2.0f) * 11.0f;
        dot.setPosition({dotX, top + 84.0f});
        dot.setFillColor(i < level ? sf::Color(255, 230, 90) : sf::Color(80, 80, 90));
        window.draw(dot);
    }

    if (font == nullptr)
    {
        return; 
    }

    drawText(window, *font, name, left + 84.0f, top + 18.0f, 20, sf::Color::White);
    std::string levelText = "Level " + std::to_string(level) + " / " +
                            std::to_string(maxLevel);
    if (level == maxLevel)
    {
        levelText += "  (MAX)";
    }
    drawText(window, *font, levelText, left + 84.0f, top + 48.0f, 14,
             sf::Color(255, 230, 90));

    // One row per value: the label on the left, the value on the right.
    const std::string rows[][2] = {
        {"Position", "(" + toText(x) + ", " + toText(y) + ")"},
        {"Range", toText(stats.range) + " cells"},
        {"Power", std::to_string(stats.power) + " damage per shot"},
        {"Fire rate", toText(stats.fireRate) + " shots per second"},
        {"Upgrade", upgradeCost < 0 ? std::string("max level")
                                    : std::to_string(upgradeCost) + " coins"},
        {"Sell for", std::to_string(refundValue) + " coins"},
    };
    float rowY = top + 104.0f;
    for (const auto &row : rows)
    {
        drawText(window, *font, row[0], left + 20.0f, rowY, 14, sf::Color(170, 170, 185));
        drawText(window, *font, row[1], left + 110.0f, rowY, 14, sf::Color::White);
        rowY += 20.0f;
    }

    drawText(window, *font, "Updates received: " + std::to_string(updateCount),
             left + 20.0f, top + 224.0f, 12, sf::Color(130, 130, 145));
}


// Part 1 driver


namespace
{

std::optional<sf::Font> loadFont()
{
    const char *candidates[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
    };
    for (const char *path : candidates)
    {
        sf::Font font;
        if (std::ifstream(path).good() && font.openFromFile(path))
        {
            return font;
        }
    }
    return std::nullopt;
}

// What each press of Space does, in order.
const std::vector<std::string> steps = {
    "Upgrade the tower to level 2",
    "Move the tower to (5, 3)",
    "Fire at the critter",
    "Upgrade the tower to level 3",
    "Upgrade the tower to level 4",
    "Try to upgrade past the max level",
    "Detach the view, then move the tower",
};

// Does one step. The driver only ever changes the tower; the view is
// updated by the tower's notify(), never by the driver.
void doStep(std::size_t step, Tower &tower, TowerObserver &view,
            std::vector<Critter *> &critters)
{
    std::cout << "Driver: " << steps[step] << std::endl;
    switch (step)
    {
    case 0:
    case 3:
    case 4:
        tower.upgrade();
        break;
    case 1:
        tower.setPosition(5.0f, 3.0f);
        break;
    case 2:
        tower.update(1.0f, critters); // the critter at (4, 2) is in range
        std::cout << "Driver: the critter has " << critters[0]->getHitPoints()
                  << " HP left" << std::endl;
        break;
    case 5:
        if (!tower.upgrade())
        {
            std::cout << "Driver: refused, already at max level. Nothing changed,"
                         " so no notify and no update." << std::endl;
        }
        break;
    case 6:
        tower.detach(&view);
        tower.setPosition(1.0f, 1.0f);
        std::cout << "Driver: the tower is at (1, 1) now, but the view still shows"
                     " (5, 3) because it isn't attached anymore." << std::endl;
        break;
    }
}
}

void runTowerObserverDemo()
{
    std::cout << "========== Part 1: Tower observer demo ==========" << std::endl;

    DirectTower tower(2.0f, 2.0f);
    TowerObserver view(tower);
    tower.attach(&view); // plug the view into the tower
    std::cout << "Driver: made a Direct Tower at (2, 2) and attached a TowerObserver."
              << std::endl;

    // Something for the tower to shoot at.
    Critter target(2, 60, 1, 1.0, 1, Position{4, 2});
    std::vector<Critter *> critters = {&target};

    sf::RenderWindow window(sf::VideoMode({720, 300}), "Part 1: Tower observer",sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    const std::optional<sf::Font> font = loadFont();
    const sf::Font *fontPtr = font ? &*font : nullptr;
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
                    if (nextStep < steps.size())
                    {
                        doStep(nextStep, tower, view, critters);
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

        // The step list on the right is the driver's own text, not the view.
        if (fontPtr != nullptr)
        {
            float x = 370.0f;
            drawText(window, *fontPtr,
                     nextStep < steps.size() ? "Press Space to:" : "Done. Press Space to close.",
                     x, 20.0f, 16, sf::Color::White);
            for (std::size_t i = 0; i < steps.size(); ++i)
            {
                sf::Color color = i < nextStep    ? sf::Color(110, 110, 120): i == nextStep ? sf::Color(255, 230, 90): sf::Color(200, 200, 210);
                drawText(window, *fontPtr,std::to_string(i + 1) + ". " + steps[i], x, 52.0f + i * 22.0f,14, color);
            }
            drawText(window, *fontPtr,"Critter at (4, 2): " + std::to_string(target.getHitPoints()) + " HP",x, 226.0f, 14, sf::Color(230, 120, 120));
        }
        else
        {
            window.setTitle("Part 1: Tower observer - press Space (step " +
                            std::to_string(nextStep + 1) + ")");
        }
        window.display();
    }

    tower.detach(&view); // in case the window was closed early
    std::cout << "Tower observer demo finished." << std::endl << std::endl;
}
