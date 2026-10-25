
#include "CritterObserver.h"

#include "CritterGroup.h"
#include "DemoText.h"
#include "Map.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

// Constructor for observing a group of critters.
CritterObserver::CritterObserver(CritterGroup &group)
    : group(&group)
{
}

// Constructor for observing an individual critter.
CritterObserver::CritterObserver(Critter &critter)
    : critter(&critter)
{
    readCritter(); // so there's something to show before the first change
    maxHitPoints = shownHitPoints;
}

// Copies the critter's current values so draw() can show them.
void CritterObserver::readCritter()
{
    shownLevel = critter->getLevel();
    shownHitPoints = critter->getHitPoints();
    shownPosition = critter->getPosition();

    if (critter->hasReachedExit())
    {
        shownStatus = "Reached the exit";
    }
    else if (shownHitPoints == 0)
    {
        shownStatus = "Killed";
    }
    else
    {
        shownStatus = "Walking";
    }
}

// Returns the number of observer updates received.
int CritterObserver::getUpdateCount() const
{
    return updateCount;
}

void CritterObserver::update()
{
    ++updateCount;

    // Called when an individual critter moves or takes damage.
    if (critter != nullptr)
    {
        readCritter();
        Position position = critter->getPosition();

        std::cout << "CritterObserver: individual critter changed"
                  << " (update #" << updateCount << ")"
                  << std::endl;

        std::cout << "  Level: " << critter->getLevel()
                  << ", HP: " << critter->getHitPoints()
                  << ", position: (" << position.x << ", "
                  << position.y << ")"
                  << ", alive: "
                  << (critter->isAlive() ? "yes" : "no")
                  << std::endl
                  << std::endl;

        return;
    }

    // Called automatically after
    // addCritter, removeCritter, or spawnNext.
    if (group != nullptr)
    {
        std::cout << "CritterObserver: wave changed"
                  << " (update #" << updateCount << ", "
                  << group->size() << " critters, "
                  << group->getNextCritterIndex() << " on map)"
                  << std::endl;

        const std::vector<Critter> &critters = group->getCritters();

        for (std::size_t i = 0; i < critters.size(); ++i)
        {
            const Critter &c = critters[i];
            Position position = c.getPosition();

            std::cout << "  critter " << i
                      << ": level " << c.getLevel()
                      << ", HP " << c.getHitPoints()
                      << ", at (" << position.x << ", "
                      << position.y << ")" << std::endl;
        }

        std::cout << std::endl;
    }
}

void CritterObserver::draw(sf::RenderWindow &window, const sf::Font *font,
                           sf::Vector2f topLeft) const
{
    if (critter == nullptr)
    {
        return; // only the individual critter view has a panel
    }

    const float left = topLeft.x;
    const float top = topLeft.y;

    sf::RectangleShape panel({320.0f, 250.0f});
    panel.setPosition(topLeft);
    panel.setFillColor(sf::Color(30, 32, 40));
    panel.setOutlineColor(sf::Color(90, 90, 100));
    panel.setOutlineThickness(1.0f);
    window.draw(panel);

    // The critter itself: a red circle, grey once it's dead.
    sf::CircleShape icon(22.0f);
    icon.setOrigin({22.0f, 22.0f});
    icon.setPosition({left + 44.0f, top + 44.0f});
    icon.setFillColor(shownHitPoints > 0 ? sf::Color(220, 70, 70) : sf::Color(90, 90, 90));
    icon.setOutlineColor(sf::Color(20, 20, 30));
    icon.setOutlineThickness(2.0f);
    window.draw(icon);

    // Health bar: the green part is the hit points left.
    sf::RectangleShape barBack({200.0f, 10.0f});
    barBack.setPosition({left + 94.0f, top + 72.0f});
    barBack.setFillColor(sf::Color(70, 30, 30));
    window.draw(barBack);
    sf::RectangleShape bar({200.0f * shownHitPoints / maxHitPoints, 10.0f});
    bar.setPosition({left + 94.0f, top + 72.0f});
    bar.setFillColor(sf::Color(80, 200, 90));
    window.draw(bar);

    if (font == nullptr)
    {
        return;
    }

    drawDemoText(window, *font, "Critter (level " + std::to_string(shownLevel) + ")",
                 left + 94.0f, top + 18.0f, 20, sf::Color::White);
    drawDemoText(window, *font, shownStatus, left + 94.0f, top + 46.0f, 14,
                 sf::Color(255, 230, 90));

    // One row per value: the label on the left, the value on the right.
    const std::string rows[][2] = {
        {"Position", "(" + std::to_string(shownPosition.x) + ", " +
                         std::to_string(shownPosition.y) + ")"},
        {"Hit points", std::to_string(shownHitPoints) + " / " +
                           std::to_string(maxHitPoints)},
        {"Strength", "steals " + std::to_string(critter->getStrength()) +
                         " coins at the exit"},
        {"Reward", std::to_string(critter->getReward()) + " coins when killed"},
    };
    float rowY = top + 110.0f;
    for (const auto &row : rows)
    {
        drawDemoText(window, *font, row[0], left + 20.0f, rowY, 14, sf::Color(170, 170, 185));
        drawDemoText(window, *font, row[1], left + 110.0f, rowY, 14, sf::Color::White);
        rowY += 22.0f;
    }

    drawDemoText(window, *font, "Updates received: " + std::to_string(updateCount),
                 left + 20.0f, top + 220.0f, 14, sf::Color(255, 230, 90));
}

// What each press of Space does in the critter window, in order.
const std::vector<std::string> critterSteps = {
    "Move the critter one step",
    "A tower hits it for 10 damage",
    "Move the critter one step",
    "Detach the view, then move the critter",
    "Attach the view again, then hit it for 20 (killed)",
    "Try to move the dead critter",
};

// Does one step. The driver only changes the critter; the view is updated
// by the critter's notify(), never by the driver.
void doCritterStep(std::size_t step, Critter &critter, CritterObserver &view,
                   const Map &map)
{
    std::cout << "Driver: " << critterSteps[step] << std::endl;
    switch (step)
    {
    case 0:
    case 2:
        critter.move(map);
        break;
    case 1:
        critter.takeDamage(10);
        break;
    case 3:
        critter.detach(&view);
        critter.move(map);
        std::cout << "Driver: the critter is at (3, 0) now, but the view still"
                     " shows (2, 0) because it isn't attached anymore." << std::endl;
        break;
    case 4:
        critter.attach(&view);
        critter.takeDamage(20); // the view catches up on this notify
        break;
    case 5:
        critter.move(map);
        std::cout << "Driver: a dead critter doesn't move. Nothing changed,"
                     " so no notify and no update." << std::endl;
        break;
    }
}

// Opens the window for one critter. Each Space press does one step.
void runCritterWindow()
{
    // A straight path: entry at (0, 0), exit at (5, 0).
    Map path(6, 1);
    path.setTile(0, 0, TileType::ENTRY);
    for (int x = 1; x < 5; ++x)
    {
        path.setTile(x, 0, TileType::PATH);
    }
    path.setTile(5, 0, TileType::EXIT);

    // reward 5, 30 HP, strength 3, speed 1, level 2, starting on the entry
    Critter critter(5, 30, 3, 1.0, 2, Position{0, 0});
    CritterObserver view(critter);
    critter.attach(&view); // plug the view into the critter
    std::cout << "Driver: made a critter on the entry and attached a CritterObserver."
              << std::endl;
    std::cout << "Press Space in the window to do the next step." << std::endl;

    sf::RenderWindow window(sf::VideoMode({800, 290}), "Part 3: Critter observer",
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
                    if (nextStep < critterSteps.size())
                    {
                        doCritterStep(nextStep, critter, view, path);
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
            float x = 370.0f;
            drawDemoText(window, font, "The critter is the Observable. The panel on the",
                         x, 20.0f, 14, sf::Color(170, 170, 185));
            drawDemoText(window, font, "left is a CritterObserver attached to it. When",
                         x, 38.0f, 14, sf::Color(170, 170, 185));
            drawDemoText(window, font, "the critter moves or takes damage it calls",
                         x, 56.0f, 14, sf::Color(170, 170, 185));
            drawDemoText(window, font, "notify(), and the panel redraws.",
                         x, 74.0f, 14, sf::Color(170, 170, 185));

            drawDemoText(window, font,
                         nextStep < critterSteps.size() ? "Press Space to:" : "Done. Press Space to close.",
                         x, 108.0f, 16, sf::Color::White);
            for (std::size_t i = 0; i < critterSteps.size(); ++i)
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
                drawDemoText(window, font, std::to_string(i + 1) + ". " + critterSteps[i],
                             x, 138.0f + i * 22.0f, 14, color);
            }
        }
        else
        {
            window.setTitle("Part 3: Critter observer - press Space (step " +
                            std::to_string(nextStep + 1) + ")");
        }
        window.display();
    }

    critter.detach(&view); // in case the window was closed early
}

void runCritterObserverDemo()
{
    std::cout << "Critter observer demo"
              << std::endl;

    Map map(4, 2);
    map.setTile(0, 0, TileType::ENTRY);
    map.setTile(1, 0, TileType::PATH);
    map.setTile(2, 0, TileType::PATH);
    map.setTile(3, 0, TileType::EXIT);

    // PART A: Individual Critter Observer, shown in a window.
    std::cout << "\nIndividual Critter Observer: "
              << std::endl;
    runCritterWindow();

    // PART B: Existing CritterGroup Observer
    std::cout << "\nCritterGroup Observer: "
              << std::endl;

    CritterGroup group;
    CritterObserver groupObserver(group);
    group.attach(&groupObserver);

    Critter first(4, 20, 2, 1.0, 1);
    Critter second(6, 25, 2, 1.0, 2);

    std::cout << "Adding two critters to the wave: "
              << std::endl;

    group.addCritter(first);
    group.addCritter(second);

    std::cout << "Spawning the first critter on the entry: "
              << std::endl;

    group.spawnNext(map);

    std::cout << "Removing the last critter: "
              << std::endl;

    group.removeCritter(group.size() - 1);

    std::cout << "Total group observer updates: "
              << groupObserver.getUpdateCount()
              << std::endl;

    group.detach(&groupObserver);

    std::cout << "Critter observer demo finished."
              << std::endl
              << std::endl;
}
