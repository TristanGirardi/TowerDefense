We certify that this submission is the original work of members of the group and meets the Faculty's Expectations of Originality.

# TowerDefense

Matthew Benkhellat - 40325455

Tristan Girardi - 40272203

Youssef Yassa - 40265083

Youssef Youssef - 40285384

10/11/2026

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
- `Critter`: stores its stats and position. Each move runs a BFS to find the
  next step toward the exit. It also inherits from `Observable` and notifies
  its observers whenever its health or position changes.
- `CritterGroup`: one wave. Critters enter the map one after the other.
  The group also inherits from `Observable` and notifies observers when
  critters are added, removed or spawned.
- `CritterGroupGenerator`: makes the wave for a given level, with more
  critters and stronger stats each time.

None of these classes keep track of the player's coins or lives. They just
report what happened (a kill's reward, the coins a critter stole), and the
`Game` class in `Gui.cpp` keeps the totals while you play. That way the same
classes work in both the console tests and the game window. The code
comments go into more detail.

### Observer pattern

- `Observable` / `Observer`: `attach`, `detach`, and `notify()` when something
  changes (same idea as in the course notes).
- `Tower` (part 1): notifies on move, upgrade, and after firing.
- `Map` (part 2): notifies on `setTile`, `placeTower`, `moveTower` and
  `removeTower`. The map now remembers which scenery cells have a tower, so
  "moving the tower on the map" is a map change. `MapObserver` keeps a copy
  of the map from its last `update()`, prints it in the console, and draws
  it in an SFML window (T = tower in the console view). Refused moves (onto
  the path, off the map) don't notify.
- `Critter` (part 3): inherits from `Observable` and calls `notify()` when
  it takes damage or moves to another cell. This allows an observer to
  automatically receive updates when an individual critter changes.
- `CritterGroup` (part 3): notifies on `addCritter`, `removeCritter`, and
  `spawnNext`. We kept the group observable because adding and removing
  critters affects the entire wave.
- `CritterObserver` (part 3): can observe either an individual `Critter`
  or a `CritterGroup`. Its `update()` method prints the current stats,
  including health, position and level. It also counts the number of
  updates received so the driver can verify that notifications occur.
  The driver demonstrates individual critter movement and damage,
  group additions and removals, and observer attachment and detachment.

## Tools

- CMake, because CLion uses it and it downloads SFML on its own, so you
  don't need to install anything extra to build the project.
- SFML for the game window and the Tower and Map observer views. It's a
  simple C++ library for 2D drawing and mouse/keyboard input, which is
  all the game needed.
- The Critter Observer uses console output to display changes to
  individual critters and groups.

## Compiling and running

You need CMake 3.16+ and a C++17 compiler. The first build downloads SFML 3
from GitHub, so it needs an internet connection; after that it builds offline.

In CLion: open the folder and run the `TowerDefenseDriver` target.

From a terminal:

```bash
cmake -S . -B build
cmake --build build
```

Then run `build/TowerDefenseDriver` (or `build\Debug\TowerDefenseDriver.exe`
with Visual Studio).

The driver runs the Map, Tower and Critter tests (and the observer tests)
first and prints `[PASS]` or `[FAIL]` for each one.

It then runs the three observer demonstrations:

- **Tower Observer:** opens an SFML window where each `Space` press changes
  the tower and automatically updates its graphical view.
- **Map Observer:** opens an SFML window where each `Space` press modifies
  the map and automatically updates its graphical view.
- **Critter Observer:** prints changes to the console. The driver first
  creates an individual critter, attaches an observer, damages and moves
  the critter, and verifies that notifications stop after detaching.
  It then creates a group, attaches an observer, adds and removes critters,
  and spawns a critter to demonstrate group notifications.

After the observer demonstrations, the driver opens the playable SFML game.

Controls: `1`/`2`/`3` pick a tower, left-click grass to build, left-click a
tower to upgrade, right-click to sell, `Space` for the next wave, `R` to
restart after losing.