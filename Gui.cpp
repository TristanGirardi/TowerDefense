// Graphical game built on the Tower Defense classes, drawn with SFML.
//
// Controls:
//   1 / 2 / 3          choose Direct, Splash or Hunter tower
//   Left-click grass   build the chosen tower
//   Left-click tower   upgrade it
//   Right-click tower  sell it
//   Space              start the next wave
//   R                  restart after game over
//   Esc                quit

#include "Critter.h"
#include "CritterGroup.h"
#include "CritterGroupGenerator.h"
#include "Gui.h"
#include "Map.h"
#include "Tower.h"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{
constexpr int MapWidth = 16;
constexpr int MapHeight = 10;
constexpr float Cell = 56.0f;
constexpr float HudHeight = 120.0f;
constexpr unsigned WindowWidth = static_cast<unsigned>(MapWidth * Cell);
constexpr unsigned WindowHeight = static_cast<unsigned>(MapHeight * Cell + HudHeight);

constexpr int StartingCoins = 300;
constexpr int StartingLives = 10;
constexpr float SpawnInterval = 0.8f;
constexpr float ShotDuration = 0.08f;
constexpr float MessageDuration = 2.5f;

enum class TowerKind
{
    Direct,
    Splash,
    Hunter
};

// The tower's on-screen look.
struct TowerLook
{
    sf::Color color;
    std::size_t points; // 3 = triangle, 4 = diamond, 30 = circle.
};

TowerLook lookFor(TowerKind kind)
{
    switch (kind)
    {
    case TowerKind::Direct:
        return {sf::Color(60, 110, 220), 4};
    case TowerKind::Splash:
        return {sf::Color(240, 140, 40), 30};
    case TowerKind::Hunter:
        return {sf::Color(160, 70, 200), 3};
    }
    return {sf::Color::White, 30};
}

std::unique_ptr<Tower> makeTower(TowerKind kind, int x, int y)
{
    const float fx = static_cast<float>(x);
    const float fy = static_cast<float>(y);
    switch (kind)
    {
    case TowerKind::Direct:
        return std::make_unique<DirectTower>(fx, fy);
    case TowerKind::Splash:
        return std::make_unique<SplashTower>(fx, fy);
    case TowerKind::Hunter:
        return std::make_unique<HunterTower>(fx, fy);
    }
    return nullptr;
}

// A winding path from the left edge to the right edge.
Map buildMap()
{
    Map map(MapWidth, MapHeight);
    const sf::Vector2i waypoints[] = {
        {0, 1}, {12, 1}, {12, 4}, {3, 4}, {3, 7}, {15, 7}};
    const std::size_t count = sizeof(waypoints) / sizeof(waypoints[0]);

    for (std::size_t i = 1; i < count; ++i)
    {
        sf::Vector2i current = waypoints[i - 1];
        const sf::Vector2i target = waypoints[i];
        while (true)
        {
            map.setTile(current.x, current.y, TileType::PATH);
            if (current == target)
            {
                break;
            }
            current.x += (target.x > current.x) - (target.x < current.x);
            current.y += (target.y > current.y) - (target.y < current.y);
        }
    }
    map.setTile(waypoints[0].x, waypoints[0].y, TileType::ENTRY);
    map.setTile(waypoints[count - 1].x, waypoints[count - 1].y, TileType::EXIT);
    return map;
}

sf::Vector2f cellCenter(float x, float y)
{
    return {(x + 0.5f) * Cell, (y + 0.5f) * Cell};
}

// Critters move a whole cell at a time; this slides the drawing between cells.
struct CritterVisual
{
    Position from{0, 0};
    Position to{0, 0};
    float progress = 1.0f;
    int maxHitPoints = 1;
    bool escapeCounted = false;
};

struct Shot
{
    sf::Vector2f from;
    sf::Vector2f to;
    sf::Color color;
    float timeLeft;
};

// Keeps the coins, lives, towers and current wave. This is where the money
// rules happen: paying for towers, getting rewards, losing coins and lives.
class Game
{
public:
    Game() : map(buildMap()) { reset(); }

    void reset()
    {
        wave = CritterGroup();
        visuals.clear();
        towers.clear();
        towerKinds.clear();
        shots.clear();
        coins = StartingCoins;
        lives = StartingLives;
        waveLevel = 0;
        waveActive = false;
        gameOver = false;
        selected = TowerKind::Direct;
        say("Build some towers, then press Space to send the first wave.");
    }

    void startWave()
    {
        if (gameOver || waveActive)
        {
            return;
        }
        ++waveLevel;
        wave = generator.generate(waveLevel);
        visuals.assign(wave.size(), CritterVisual());
        spawnTimer = 0.0f;
        waveActive = true;
        say("Wave " + std::to_string(waveLevel) + " incoming: " +
            std::to_string(wave.size()) + " critters.");
    }

    void select(TowerKind kind) { selected = kind; }

    void leftClick(int x, int y)
    {
        if (gameOver)
        {
            return;
        }
        const int index = towerAt(x, y);
        if (index >= 0)
        {
            Tower &tower = *towers[index];
            if (!tower.canUpgrade())
            {
                say(tower.getName() + " is already at max level.");
            }
            else if (coins < tower.getUpgradeCost())
            {
                say("Upgrade costs " + std::to_string(tower.getUpgradeCost()) + " coins.");
            }
            else
            {
                coins -= tower.getUpgradeCost();
                tower.upgrade();
                say(tower.getName() + " upgraded to level " +
                    std::to_string(tower.getLevel()) + ".");
            }
            return;
        }

        if (map.getTile(x, y) != TileType::SCENERY)
        {
            say("Towers can only be built on grass.");
            return;
        }
        std::unique_ptr<Tower> tower = makeTower(selected, x, y);
        if (coins < tower->getBuyCost())
        {
            say(tower->getName() + " costs " + std::to_string(tower->getBuyCost()) + " coins.");
            return;
        }
        coins -= tower->getBuyCost();
        towers.push_back(std::move(tower));
        towerKinds.push_back(selected);
    }

    void rightClick(int x, int y)
    {
        if (gameOver)
        {
            return;
        }
        const int index = towerAt(x, y);
        if (index < 0)
        {
            return;
        }
        const int refund = towers[index]->getRefundValue();
        coins += refund;
        say(towers[index]->getName() + " sold for " + std::to_string(refund) + " coins.");
        towers.erase(towers.begin() + index);
        towerKinds.erase(towerKinds.begin() + index);
    }

    // Each frame: spawn, move, towers shoot, pay rewards, check if the wave is done.
    void update(float dt)
    {
        for (Shot &shot : shots)
        {
            shot.timeLeft -= dt;
        }
        shots.erase(std::remove_if(shots.begin(), shots.end(),
                                   [](const Shot &s) { return s.timeLeft <= 0.0f; }),
                    shots.end());
        messageTime = std::max(0.0f, messageTime - dt);

        if (!waveActive || gameOver)
        {
            return;
        }

        std::vector<Critter> &critters = wave.getCritters();

        // Spawn the next critter at the entry.
        spawnTimer -= dt;
        if (spawnTimer <= 0.0f && wave.getNextCritterIndex() < wave.size())
        {
            Critter *spawned = wave.spawnNext(map);
            CritterVisual &visual = visuals[wave.getNextCritterIndex() - 1];
            visual.from = visual.to = spawned->getPosition();
            visual.progress = 1.0f;
            visual.maxHitPoints = spawned->getHitPoints();
            spawnTimer = SpawnInterval;
        }

        const std::size_t spawnedCount = wave.getNextCritterIndex();
        std::vector<Critter *> active;
        for (std::size_t i = 0; i < spawnedCount; ++i)
        {
            Critter &critter = critters[i];
            critter.move(map, dt);

            CritterVisual &visual = visuals[i];
            if (critter.getPosition() != visual.to)
            {
                visual.from = visual.to;
                visual.to = critter.getPosition();
                visual.progress = 0.0f;
            }
            visual.progress = std::min(
                1.0f, visual.progress + dt * static_cast<float>(critter.getSpeed()));

            if (critter.hasReachedExit() && !visual.escapeCounted)
            {
                visual.escapeCounted = true;
                --lives;
                coins = std::max(0, coins - critter.claimCoinsStolenAtExit());
            }
            active.push_back(&critter);
        }

        // Let each tower fire, and draw a beam to every critter it hurt.
        // Towers don't report who they hit, so we compare HP before and after.
        std::vector<int> before(active.size());
        for (std::size_t t = 0; t < towers.size(); ++t)
        {
            for (std::size_t i = 0; i < active.size(); ++i)
            {
                before[i] = active[i]->getHitPoints();
            }
            towers[t]->update(dt, active);
            for (std::size_t i = 0; i < active.size(); ++i)
            {
                if (active[i]->getHitPoints() < before[i])
                {
                    shots.push_back({cellCenter(towers[t]->getX(), towers[t]->getY()),
                                     critterPixel(i), lookFor(towerKinds[t]).color,
                                     ShotDuration});
                }
            }
        }

        bool anyAlive = false;
        for (std::size_t i = 0; i < spawnedCount; ++i)
        {
            coins += critters[i].claimReward();
            anyAlive = anyAlive || critters[i].isAlive();
        }

        if (lives <= 0)
        {
            lives = 0;
            gameOver = true;
            say("Game over on wave " + std::to_string(waveLevel) + ". Press R to restart.");
        }
        else if (spawnedCount == wave.size() && !anyAlive)
        {
            waveActive = false;
            say("Wave " + std::to_string(waveLevel) +
                " cleared. Press Space for the next one.");
        }
    }

    void draw(sf::RenderWindow &window, const sf::Font *font, sf::Vector2i mouse) const
    {
        drawMap(window, mouse);
        drawTowers(window);
        drawCritters(window);
        drawShots(window);
        drawHud(window, font);
    }

    std::string titleText() const
    {
        return "Tower Defense - Wave " + std::to_string(waveLevel) + " - Coins " +
               std::to_string(coins) + " - Lives " + std::to_string(lives);
    }

private:
    Map map;
    CritterGroupGenerator generator;
    CritterGroup wave;
    std::vector<CritterVisual> visuals;
    std::vector<std::unique_ptr<Tower>> towers;
    std::vector<TowerKind> towerKinds; // Parallel to towers, for drawing.
    std::vector<Shot> shots;

    int coins = 0;
    int lives = 0;
    int waveLevel = 0;
    float spawnTimer = 0.0f;
    bool waveActive = false;
    bool gameOver = false;
    TowerKind selected = TowerKind::Direct;
    std::string message;
    float messageTime = 0.0f;

    void say(const std::string &text)
    {
        message = text;
        messageTime = MessageDuration;
    }

    int towerAt(int x, int y) const
    {
        for (std::size_t i = 0; i < towers.size(); ++i)
        {
            if (static_cast<int>(towers[i]->getX()) == x &&
                static_cast<int>(towers[i]->getY()) == y)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    sf::Vector2f critterPixel(std::size_t index) const
    {
        const CritterVisual &visual = visuals[index];
        const float t = visual.progress;
        const float x = visual.from.x + (visual.to.x - visual.from.x) * t;
        const float y = visual.from.y + (visual.to.y - visual.from.y) * t;
        return cellCenter(x, y);
    }

    static void drawRangeCircle(sf::RenderWindow &window, float x, float y, float range)
    {
        sf::CircleShape circle(range * Cell, 60);
        circle.setOrigin({range * Cell, range * Cell});
        circle.setPosition(cellCenter(x, y));
        circle.setFillColor(sf::Color(255, 255, 255, 30));
        circle.setOutlineColor(sf::Color(255, 255, 255, 120));
        circle.setOutlineThickness(1.5f);
        window.draw(circle);
    }

    void drawMap(sf::RenderWindow &window, sf::Vector2i mouse) const
    {
        sf::RectangleShape tile({Cell, Cell});
        tile.setOutlineThickness(-1.0f);
        for (int y = 0; y < MapHeight; ++y)
        {
            for (int x = 0; x < MapWidth; ++x)
            {
                sf::Color color;
                switch (map.getTile(x, y))
                {
                case TileType::SCENERY:
                    color = ((x + y) % 2 == 0) ? sf::Color(92, 150, 74) : sf::Color(84, 140, 68);
                    break;
                case TileType::PATH:
                    color = sf::Color(210, 185, 130);
                    break;
                case TileType::ENTRY:
                    color = sf::Color(70, 170, 220);
                    break;
                case TileType::EXIT:
                    color = sf::Color(210, 70, 70);
                    break;
                }
                tile.setFillColor(color);
                tile.setOutlineColor(sf::Color(0, 0, 0, 25));
                tile.setPosition({x * Cell, y * Cell});
                window.draw(tile);
            }
        }

        // Hover highlight and range preview.
        const int mx = static_cast<int>(mouse.x / Cell);
        const int my = static_cast<int>(mouse.y / Cell);
        if (mouse.x < 0 || mouse.y < 0 || mx >= MapWidth || my >= MapHeight)
        {
            return;
        }
        sf::RectangleShape hover({Cell, Cell});
        hover.setPosition({mx * Cell, my * Cell});
        hover.setFillColor(sf::Color(255, 255, 255, 40));
        hover.setOutlineColor(sf::Color(255, 255, 255, 160));
        hover.setOutlineThickness(-2.0f);
        window.draw(hover);

        const int index = towerAt(mx, my);
        if (index >= 0)
        {
            drawRangeCircle(window, static_cast<float>(mx), static_cast<float>(my),
                            towers[index]->getStats().range);
        }
        else if (map.getTile(mx, my) == TileType::SCENERY)
        {
            drawRangeCircle(window, static_cast<float>(mx), static_cast<float>(my),
                            makeTower(selected, mx, my)->getStats().range);
        }
    }

    void drawTowers(sf::RenderWindow &window) const
    {
        for (std::size_t i = 0; i < towers.size(); ++i)
        {
            const Tower &tower = *towers[i];
            const TowerLook look = lookFor(towerKinds[i]);
            const sf::Vector2f center = cellCenter(tower.getX(), tower.getY());

            const float radius = Cell * 0.36f;
            sf::CircleShape body(radius, look.points);
            body.setOrigin({radius, radius});
            body.setPosition({center.x, center.y - 4.0f});
            body.setFillColor(look.color);
            body.setOutlineColor(sf::Color(20, 20, 30));
            body.setOutlineThickness(2.0f);
            window.draw(body);

            // One pip per level along the bottom of the cell.
            const float pip = 3.5f;
            const int levels = tower.getLevel();
            const float startX = center.x - (levels - 1) * pip * 1.4f;
            for (int l = 0; l < levels; ++l)
            {
                sf::CircleShape dot(pip);
                dot.setOrigin({pip, pip});
                dot.setPosition({startX + l * pip * 2.8f, center.y + Cell * 0.36f});
                dot.setFillColor(sf::Color(255, 230, 90));
                window.draw(dot);
            }
        }
    }

    void drawCritters(sf::RenderWindow &window) const
    {
        const std::vector<Critter> &critters = wave.getCritters();
        for (std::size_t i = 0; i < wave.getNextCritterIndex(); ++i)
        {
            const Critter &critter = critters[i];
            if (!critter.isAlive())
            {
                continue;
            }
            const sf::Vector2f center = critterPixel(i);

            const float radius = Cell * 0.22f;
            sf::CircleShape body(radius);
            body.setOrigin({radius, radius});
            body.setPosition(center);
            const int shade = std::max(60, 230 - critter.getLevel() * 18);
            body.setFillColor(sf::Color(200, static_cast<std::uint8_t>(shade / 3),
                                        static_cast<std::uint8_t>(shade / 3)));
            body.setOutlineColor(sf::Color(40, 10, 10));
            body.setOutlineThickness(2.0f);
            window.draw(body);

            // Hit point bar.
            const float barWidth = Cell * 0.6f;
            const float fraction = static_cast<float>(critter.getHitPoints()) /
                                   static_cast<float>(visuals[i].maxHitPoints);
            sf::RectangleShape back({barWidth, 5.0f});
            back.setPosition({center.x - barWidth / 2, center.y - radius - 10.0f});
            back.setFillColor(sf::Color(40, 40, 40));
            window.draw(back);
            sf::RectangleShape front({barWidth * fraction, 5.0f});
            front.setPosition(back.getPosition());
            front.setFillColor(fraction > 0.5f ? sf::Color(80, 220, 80)
                                               : sf::Color(240, 200, 50));
            window.draw(front);
        }
    }

    void drawShots(sf::RenderWindow &window) const
    {
        for (const Shot &shot : shots)
        {
            const sf::Vector2f delta = shot.to - shot.from;
            const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            sf::RectangleShape beam({length, 3.0f});
            beam.setOrigin({0.0f, 1.5f});
            beam.setPosition(shot.from);
            beam.setRotation(sf::radians(std::atan2(delta.y, delta.x)));
            beam.setFillColor(shot.color);
            window.draw(beam);
        }
    }

    void drawHud(sf::RenderWindow &window, const sf::Font *font) const
    {
        const float top = MapHeight * Cell;
        sf::RectangleShape panel({static_cast<float>(WindowWidth), HudHeight});
        panel.setPosition({0.0f, top});
        panel.setFillColor(sf::Color(30, 32, 40));
        window.draw(panel);

        // Tower picker swatches, so the HUD is usable even without a font.
        const TowerKind kinds[] = {TowerKind::Direct, TowerKind::Splash, TowerKind::Hunter};
        for (int k = 0; k < 3; ++k)
        {
            const float x = 20.0f + k * 70.0f;
            sf::RectangleShape slot({56.0f, 56.0f});
            slot.setPosition({x, top + 12.0f});
            slot.setFillColor(sf::Color(50, 54, 66));
            slot.setOutlineThickness(kinds[k] == selected ? 3.0f : 1.0f);
            slot.setOutlineColor(kinds[k] == selected ? sf::Color(255, 230, 90)
                                                      : sf::Color(90, 90, 100));
            window.draw(slot);

            const TowerLook look = lookFor(kinds[k]);
            sf::CircleShape icon(18.0f, look.points);
            icon.setOrigin({18.0f, 18.0f});
            icon.setPosition({x + 28.0f, top + 40.0f});
            icon.setFillColor(look.color);
            window.draw(icon);
        }

        if (font == nullptr)
        {
            return;
        }

        auto text = [&](const std::string &str, float x, float y, unsigned size, sf::Color color) {
            sf::Text label(*font, str, size);
            label.setFillColor(color);
            label.setPosition({x, y});
            window.draw(label);
        };

        for (int k = 0; k < 3; ++k)
        {
            text(std::to_string(k + 1), 24.0f + k * 70.0f, top + 13.0f, 14, sf::Color::White);
        }

        const std::unique_ptr<Tower> preview = makeTower(selected, 0, 0);
        const TowerStats &stats = preview->getStats();
        char summary[96];
        std::snprintf(summary, sizeof(summary), "%s  cost %d  range %.1f  power %d",
                      preview->getName().c_str(), preview->getBuyCost(), stats.range,
                      stats.power);
        text(summary, 20.0f, top + 80.0f, 14, sf::Color(220, 220, 230));

        const float infoX = 380.0f;
        text("Wave " + std::to_string(waveLevel) + "    Coins " + std::to_string(coins) +
                 "    Lives " + std::to_string(lives),
             infoX, top + 12.0f, 22, sf::Color(255, 230, 90));
        text("Left-click grass: build    Left-click tower: upgrade",
             infoX, top + 44.0f, 14, sf::Color(190, 190, 200));
        text(std::string("Right-click tower: sell    ") +
                 (waveActive ? "Wave in progress" : "Space: next wave"),
             infoX, top + 62.0f, 14, sf::Color(190, 190, 200));
        if (messageTime > 0.0f || gameOver)
        {
            text(message, infoX, top + 88.0f, 16,
                 gameOver ? sf::Color(255, 110, 110) : sf::Color::White);
        }
    }
};

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
}

void runGui()
{
    sf::RenderWindow window(sf::VideoMode({WindowWidth, WindowHeight}), "Tower Defense",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    const std::optional<sf::Font> font = loadFont();
    Game game;
    sf::Clock clock;
    std::string title;

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
                switch (key->code)
                {
                case sf::Keyboard::Key::Num1:
                    game.select(TowerKind::Direct);
                    break;
                case sf::Keyboard::Key::Num2:
                    game.select(TowerKind::Splash);
                    break;
                case sf::Keyboard::Key::Num3:
                    game.select(TowerKind::Hunter);
                    break;
                case sf::Keyboard::Key::Space:
                    game.startWave();
                    break;
                case sf::Keyboard::Key::R:
                    game.reset();
                    break;
                case sf::Keyboard::Key::Escape:
                    window.close();
                    break;
                default:
                    break;
                }
            }
            else if (const auto *click = event->getIf<sf::Event::MouseButtonPressed>())
            {
                const int x = static_cast<int>(click->position.x / Cell);
                const int y = static_cast<int>(click->position.y / Cell);
                if (click->position.y >= MapHeight * Cell)
                {
                    // Clicking a swatch in the HUD picks that tower.
                    const int slot = (click->position.x - 20) / 70;
                    if (click->position.x >= 20 && slot < 3)
                    {
                        game.select(static_cast<TowerKind>(slot));
                    }
                }
                else if (x >= 0 && x < MapWidth && y >= 0)
                {
                    if (click->button == sf::Mouse::Button::Left)
                    {
                        game.leftClick(x, y);
                    }
                    else if (click->button == sf::Mouse::Button::Right)
                    {
                        game.rightClick(x, y);
                    }
                }
            }
        }

        // Cap the step so dragging the window doesn't teleport critters.
        const float dt = std::min(clock.restart().asSeconds(), 0.05f);
        game.update(dt);

        const std::string newTitle = game.titleText();
        if (newTitle != title)
        {
            title = newTitle;
            window.setTitle(title);
        }

        window.clear(sf::Color(20, 20, 24));
        game.draw(window, font ? &*font : nullptr, sf::Mouse::getPosition(window));
        window.display();
    }
}
