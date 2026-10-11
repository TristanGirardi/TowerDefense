# TowerDefense

Matthew Benkhellat - 40325455

Tristan Girardi - 40272203

Youssef Yassa - 40265083

Youssef Youssef - 40285384

## Game rules

Critters come in waves and walk a path from an entry to an exit, and you
build towers next to the path to stop them [1]. Like in Desktop Tower
Defense, critters take the shortest way to the exit and each wave is harder
than the one before [2]. Killing a critter gives you coins [1], more for
higher-level critters, and you spend them on towers and upgrades. If a
critter reaches the exit it steals coins based on its strength (from the
assignment handout), and in the GUI it also costs a life, like in Desktop
Tower Defense [2].

Towers have levels, and upgrading costs more coins. You can sell a tower for
part of what you spent on it, upgrades included, like in Bloons [3]. We give
back half. A tower finds the critters in its range, picks the closest one,
and shoots it. We have three types:

- Direct: hits one critter.
- Splash: also hits the critters near its target, like the Bloons Bomb
  Shooter [4].
- Hunter: does double damage to critters of level 3 and up, similar to the
  Bloons MOAB Mauler, which does extra damage to the big MOAB bloons [5].

Sources:

1. https://en.wikipedia.org/wiki/Tower_defense
2. https://en.wikipedia.org/wiki/Desktop_Tower_Defense
3. https://bloons.fandom.com/wiki/Selling
4. https://bloons.fandom.com/wiki/Bomb_Shooter
5. https://bloons.fandom.com/wiki/MOAB_Mauler

## Design

- `Map`: a grid of tiles (scenery, path, entry, exit). `validate()` checks
  that there's one entry and one exit, then runs a DFS to make sure a path
  connects them.
- `Tower`: an abstract base class that handles cost, upgrades, refunds,
  targeting and reloading. Each subclass (`DirectTower`, `SplashTower`,
  `HunterTower`) gives its own level table and overrides `applyEffect()`,
  which is what the shot does.
- `Critter`: its stats and position. Each move it runs a BFS to find the
  next step toward the exit.
- `CritterGroup`: one wave. Critters enter the map one after the other.
- `CritterGroupGenerator`: makes the wave for a given level, with more
  critters and stronger stats each time.

None of these classes keep track of the player's coins or lives. They just
report what happened (a kill's reward, the coins a critter stole), and the
`Game` class in `Gui.cpp` keeps the totals while you play. That way the same
classes work in both the console tests and the game window. The code
comments go into more detail.

## Tools

- CMake, because CLion uses it and it downloads SFML on its own, so you
  don't need to install anything extra to build the project.
- SFML for the game window. It's a simple C++ library for 2D drawing and
  mouse/keyboard input, which is all the game needed.

## Compiling and running

You need CMake 3.16+ and a C++17 compiler. The first build downloads SFML 3
from GitHub, so it needs an internet connection; after that it builds offline.

In CLion: open the folder and run the `TowerDefenseDriver` target.

From a terminal:

```
cmake -S . -B build
cmake --build build
```

Then run `build/TowerDefenseDriver` (or `build\Debug\TowerDefenseDriver.exe`
with Visual Studio).

The driver runs the Map, Tower and Critter tests first and prints `[PASS]`
or `[FAIL]` for each one, then opens the game.

Controls: `1`/`2`/`3` pick a tower, left-click grass to build, left-click a
tower to upgrade, right-click to sell, `Space` for the next wave, `R` to
restart after losing.
