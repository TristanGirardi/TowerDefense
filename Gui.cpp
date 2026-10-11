// The playable version of the game. It uses the same Map, Tower and Critter
// classes as the tests, and SFML does the drawing.
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
#include <fstream>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace
{
constexpr int mapWidth = 16;
constexpr int mapHeight = 10;
constexpr float cellSize = 56.0f;
constexpr float hudHeight = 120.0f;
constexpr unsigned windowWidth = static_cast<unsigned>(mapWidth * cellSize);
constexpr unsigned windowHeight = static_cast<unsigned>(mapHeight * cellSize + hudHeight);

constexpr int startingCoins = 300;
constexpr int startingLives = 10;
constexpr float spawnInterval = 0.8f;
constexpr float shotDuration = 0.08f;
constexpr float messageDuration = 2.5f;

// The tower the player currently has picked. Don't reorder these: clicking
// a swatch turns its slot number (0, 1, 2) straight into one of these.
enum class TowerKind
{
    Direct,
    Splash,
    Hunter
};

// What a tower looks like on screen, so the three types are easy to tell apart.
struct TowerLook
{
    sf::Color color;
    std::size_t points; // corners: 3 is a triangle, 4 a diamond, 30 is basically a circle
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

// Builds the right Tower subclass for whatever the player picked.
std::unique_ptr<Tower> makeTower(TowerKind kind, int x, int y)
{
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
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

// Our game map. We list the corners of the path and fill in the straight
// lines between them, which is easier than setting every tile by hand.
Map buildMap()
{
    Map map(mapWidth, mapHeight);
    const sf::Vector2i waypoints[] = {
        {0, 1}, {12, 1}, {12, 4}, {3, 4}, {3, 7}, {15, 7}};
    std::size_t count = sizeof(waypoints) / sizeof(waypoints[0]);

    for (std::size_t i = 1; i < count; ++i)
    {
        sf::Vector2i current = waypoints[i - 1];
        sf::Vector2i target = waypoints[i];
        while (true)
        {
            map.setTile(current.x, current.y, TileType::PATH);
            if (current == target)
            {
                break;
            }
            // Each comparison is 0 or 1, so this moves us +1, -1 or 0 toward
            // the next corner.
            current.x += (target.x > current.x) - (target.x < current.x);
            current.y += (target.y > current.y) - (target.y < current.y);
        }
    }
    map.setTile(waypoints[0].x, waypoints[0].y, TileType::ENTRY);
    map.setTile(waypoints[count - 1].x, waypoints[count - 1].y, TileType::EXIT);
    return map;
}

// The game thinks in grid cells but SFML wants pixels, so this gives the
// middle of a cell in pixels.
sf::Vector2f cellCenter(float x, float y)
{
    return {(x + 0.5f) * cellSize, (y + 0.5f) * cellSize};
}

// Critters move a whole cell at a time, so to make them look smooth we
// remember where each one came from and slide it across.
struct CritterVisual
{
    Position from{0, 0};
    Position to{0, 0};
    float progress = 1.0f;
    int maxHitPoints = 1;
    bool escapeCounted = false;
};

// The quick flash of a line from a tower to whatever it just hit, so you can
// actually see the towers shooting.
struct Shot
{
    sf::Vector2f from;
    sf::Vector2f to;
    sf::Color color;
    float timeLeft;
};

// Everything about one game in progress: coins, lives, towers and the current
// wave. The other classes only report what happened, and this is where we
// actually add or take away the coins and lives.
class Game
{
public:
    Game() : map(buildMap()) { reset(); }

    // Wipes everything and starts over. Also used when you press R.
    void reset()
    {
        wave = CritterGroup();
        visuals.clear();
        towers.clear();
        towerKinds.clear();
        shots.clear();
        coins = startingCoins;
        lives = startingLives;
        waveLevel = 0;
        waveActive = false;
        gameOver = false;
        selected = TowerKind::Direct;
        say("Build some towers, then press Space to send the first wave.");
    }

    // Sends the next wave (each one is harder). Pressing Space mid-wave does
    // nothing, so you can't stack waves on top of each other.
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

    // Called from the 1/2/3 keys and the swatches.
    void select(TowerKind kind) { selected = kind; }

    // Left-clicking does two jobs: if there's already a tower on the cell we
    // upgrade it, otherwise we try to build the picked tower there.
    void leftClick(int x, int y)
    {
        if (gameOver)
        {
            return;
        }
        int index = towerAt(x, y);
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

    // Right-clicking a tower sells it and gives you back half of what you put in.
    void rightClick(int x, int y)
    {
        if (gameOver)
        {
            return;
        }
        int index = towerAt(x, y);
        if (index < 0)
        {
            return;
        }
        int refund = towers[index]->getRefundValue();
        coins += refund;
        say(towers[index]->getName() + " sold for " + std::to_string(refund) + " coins.");
        towers.erase(towers.begin() + index);
        towerKinds.erase(towerKinds.begin() + index);
    }

    // Runs once per frame. In order: let a new critter in, move everyone,
    // let the towers shoot, pay out rewards, then see if the wave is over.
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

        // Critters come in one at a time with a small gap, not all at once.
        spawnTimer -= dt;
        if (spawnTimer <= 0.0f && wave.getNextCritterIndex() < wave.size())
        {
            Critter *spawned = wave.spawnNext(map);
            CritterVisual &visual = visuals[wave.getNextCritterIndex() - 1];
            visual.from = visual.to = spawned->getPosition();
            visual.progress = 1.0f;
            visual.maxHitPoints = spawned->getHitPoints();
            spawnTimer = spawnInterval;
        }

        std::size_t spawnedCount = wave.getNextCritterIndex();
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

        // Let each tower fire. The Tower class doesn't tell us who it hit, so
        // to draw the beams we check whose HP went down.
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
                                     shotDuration});
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

    // Draws the whole screen. The order matters since later things go on top.
    // font is null if we couldn't find one on the computer.
    void draw(sf::RenderWindow &window, const sf::Font *font, sf::Vector2i mouse) const
    {
        drawMap(window, mouse);
        drawTowers(window);
        drawCritters(window);
        drawShots(window);
        drawHud(window, font);
    }

    // We also put the stats in the window title, in case there's no font
    // and the HUD can't show text.
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
    std::vector<TowerKind> towerKinds; // same order as towers, so we know how to draw each one
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

    // Pops a message up in the HUD for a couple of seconds.
    void say(const std::string &text)
    {
        message = text;
        messageTime = messageDuration;
    }

    // Finds which tower is sitting on this cell. -1 means there isn't one.
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

    // Where to draw a critter this frame. It's somewhere between the cell it
    // left and the one it's heading to, depending on how far along it is.
    sf::Vector2f critterPixel(std::size_t index) const
    {
        const CritterVisual &visual = visuals[index];
        float t = visual.progress;
        float x = visual.from.x + (visual.to.x - visual.from.x) * t;
        float y = visual.from.y + (visual.to.y - visual.from.y) * t;
        return cellCenter(x, y);
    }

    // The see-through circle that shows how far a tower can reach.
    static void drawRangeCircle(sf::RenderWindow &window, float x, float y, float range)
    {
        sf::CircleShape circle(range * cellSize, 60);
        circle.setOrigin({range * cellSize, range * cellSize});
        circle.setPosition(cellCenter(x, y));
        circle.setFillColor(sf::Color(255, 255, 255, 30));
        circle.setOutlineColor(sf::Color(255, 255, 255, 120));
        circle.setOutlineThickness(1.5f);
        window.draw(circle);
    }

    // Draws the grid (grass, path, entry, exit), then highlights the cell
    // under the mouse.
    void drawMap(sf::RenderWindow &window, sf::Vector2i mouse) const
    {
        sf::RectangleShape tile({cellSize, cellSize});
        tile.setOutlineThickness(-1.0f);
        for (int y = 0; y < mapHeight; ++y)
        {
            for (int x = 0; x < mapWidth; ++x)
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
                tile.setPosition({x * cellSize, y * cellSize});
                window.draw(tile);
            }
        }

        // Hovering shows the range: of the tower that's there, or of the one
        // you'd build if the cell is empty.
        int mx = static_cast<int>(mouse.x / cellSize);
        int my = static_cast<int>(mouse.y / cellSize);
        if (mouse.x < 0 || mouse.y < 0 || mx >= mapWidth || my >= mapHeight)
        {
            return;
        }
        sf::RectangleShape hover({cellSize, cellSize});
        hover.setPosition({mx * cellSize, my * cellSize});
        hover.setFillColor(sf::Color(255, 255, 255, 40));
        hover.setOutlineColor(sf::Color(255, 255, 255, 160));
        hover.setOutlineThickness(-2.0f);
        window.draw(hover);

        int index = towerAt(mx, my);
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

    // Draws the towers. Each type has its own shape and colour.
    void drawTowers(sf::RenderWindow &window) const
    {
        for (std::size_t i = 0; i < towers.size(); ++i)
        {
            const Tower &tower = *towers[i];
            TowerLook look = lookFor(towerKinds[i]);
            sf::Vector2f center = cellCenter(tower.getX(), tower.getY());

            float radius = cellSize * 0.36f;
            sf::CircleShape body(radius, look.points);
            body.setOrigin({radius, radius});
            body.setPosition({center.x, center.y - 4.0f});
            body.setFillColor(look.color);
            body.setOutlineColor(sf::Color(20, 20, 30));
            body.setOutlineThickness(2.0f);
            window.draw(body);

            // Little yellow dots under the tower, one for each level.
            float pip = 3.5f;
            int levels = tower.getLevel();
            float startX = center.x - (levels - 1) * pip * 1.4f;
            for (int l = 0; l < levels; ++l)
            {
                sf::CircleShape dot(pip);
                dot.setOrigin({pip, pip});
                dot.setPosition({startX + l * pip * 2.8f, center.y + cellSize * 0.36f});
                dot.setFillColor(sf::Color(255, 230, 90));
                window.draw(dot);
            }
        }
    }

    // Draws the critters that are on the map and still alive. Tougher
    // critters are a darker red so you can spot them.
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
            sf::Vector2f center = critterPixel(i);

            float radius = cellSize * 0.22f;
            sf::CircleShape body(radius);
            body.setOrigin({radius, radius});
            body.setPosition(center);
            int shade = std::max(60, 230 - critter.getLevel() * 18);
            body.setFillColor(sf::Color(200, static_cast<std::uint8_t>(shade / 3),
                                        static_cast<std::uint8_t>(shade / 3)));
            body.setOutlineColor(sf::Color(40, 10, 10));
            body.setOutlineThickness(2.0f);
            window.draw(body);

            // Health bar above the critter. It goes yellow under half HP.
            float barWidth = cellSize * 0.6f;
            float fraction = static_cast<float>(critter.getHitPoints()) /
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

    // SFML doesn't have thick lines, so each beam is a thin rectangle turned
    // to point at the critter.
    void drawShots(sf::RenderWindow &window) const
    {
        for (const Shot &shot : shots)
        {
            sf::Vector2f delta = shot.to - shot.from;
            float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            sf::RectangleShape beam({length, 3.0f});
            beam.setOrigin({0.0f, 1.5f});
            beam.setPosition(shot.from);
            beam.setRotation(sf::radians(std::atan2(delta.y, delta.x)));
            beam.setFillColor(shot.color);
            window.draw(beam);
        }
    }

    // The panel at the bottom. On the left you pick a tower and see its stats,
    // and on the right are the wave, coins, lives, controls and messages.
    void drawHud(sf::RenderWindow &window, const sf::Font *font) const
    {
        float top = mapHeight * cellSize;
        sf::RectangleShape panel({static_cast<float>(windowWidth), hudHeight});
        panel.setPosition({0.0f, top});
        panel.setFillColor(sf::Color(30, 32, 40));
        window.draw(panel);

        // The tower swatches are just shapes, so they still work without a font.
        const TowerKind kinds[] = {TowerKind::Direct, TowerKind::Splash, TowerKind::Hunter};
        for (int k = 0; k < 3; ++k)
        {
            float x = 20.0f + k * 70.0f;
            sf::RectangleShape slot({56.0f, 56.0f});
            slot.setPosition({x, top + 12.0f});
            slot.setFillColor(sf::Color(50, 54, 66));
            slot.setOutlineThickness(kinds[k] == selected ? 3.0f : 1.0f);
            slot.setOutlineColor(kinds[k] == selected ? sf::Color(255, 230, 90)
                                                      : sf::Color(90, 90, 100));
            window.draw(slot);

            TowerLook look = lookFor(kinds[k]);
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

        for (int k = 0; k < 3; ++k)
        {
            drawText(window, *font, std::to_string(k + 1), 24.0f + k * 70.0f, top + 13.0f,
                     14, sf::Color::White);
        }

        std::unique_ptr<Tower> preview = makeTower(selected, 0, 0);
        const TowerStats &stats = preview->getStats();
        std::ostringstream summary;
        summary << preview->getName() << "  cost " << preview->getBuyCost()
                << "  range " << std::fixed << std::setprecision(1) << stats.range
                << "  power " << stats.power;
        drawText(window, *font, summary.str(), 20.0f, top + 80.0f, 14,
                 sf::Color(220, 220, 230));

        float infoX = 380.0f;
        drawText(window, *font,
                 "Wave " + std::to_string(waveLevel) + "    Coins " + std::to_string(coins) +
                     "    Lives " + std::to_string(lives),
                 infoX, top + 12.0f, 22, sf::Color(255, 230, 90));
        drawText(window, *font, "Left-click grass: build    Left-click tower: upgrade",
                 infoX, top + 44.0f, 14, sf::Color(190, 190, 200));
        drawText(window, *font,
                 std::string("Right-click tower: sell    ") +
                     (waveActive ? "Wave in progress" : "Space: next wave"),
                 infoX, top + 62.0f, 14, sf::Color(190, 190, 200));
        if (messageTime > 0.0f || gameOver)
        {
            drawText(window, *font, message, infoX, top + 88.0f, 16,
                     gameOver ? sf::Color(255, 110, 110) : sf::Color::White);
        }
    }

    // Writes one line of text on the screen at (x, y).
    static void drawText(sf::RenderWindow &window, const sf::Font &font,
                         const std::string &str, float x, float y, unsigned size,
                         sf::Color color)
    {
        sf::Text label(font, str, size);
        label.setFillColor(color);
        label.setPosition({x, y});
        window.draw(label);
    }
};

// SFML doesn't come with a font, so we borrow one from the computer. If none
// of these exist the game still works, there's just no text in the HUD.
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

// Opens the window and keeps the game going until you close it. Every frame
// we handle the keyboard and mouse, update the game, then redraw it.
void runGui()
{
    sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}), "Tower Defense",
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
                int x = static_cast<int>(click->position.x / cellSize);
                int y = static_cast<int>(click->position.y / cellSize);
                if (click->position.y >= mapHeight * cellSize)
                {
                    // Clicked the bottom panel, so check if it was on a swatch.
                    int slot = (click->position.x - 20) / 70;
                    if (click->position.x >= 20 && slot < 3)
                    {
                        game.select(static_cast<TowerKind>(slot));
                    }
                }
                else if (x >= 0 && x < mapWidth && y >= 0)
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

        // Dragging the window freezes the game for a moment, and without this
        // cap the critters would jump way ahead when it comes back.
        float dt = std::min(clock.restart().asSeconds(), 0.05f);
        game.update(dt);

        std::string newTitle = game.titleText();
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
