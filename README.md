# TowerDefense

The critter model is provided by:

- `Critter`: stores combat and movement statistics, follows the shortest
  traversable route to the map exit, receives tower damage, and reports reward
  or exit theft.
- `CritterGroup`: owns a wave's critters and spawns them at the map entry in
  order.
- `CritterGroupGenerator`: creates progressively harder waves. By default,
  each wave adds one critter, increases hit points and reward with the wave
  level, and gradually increases strength and speed.

The project builds the classes as the `TowerDefense` CMake library. For
example:

```cpp
Map map(5, 1);
map.setTile(0, 0, TileType::ENTRY);
map.setTile(1, 0, TileType::PATH);
map.setTile(2, 0, TileType::PATH);
map.setTile(3, 0, TileType::PATH);
map.setTile(4, 0, TileType::EXIT);

CritterGroupGenerator generator;
CritterGroup wave = generator.generate(1);
Critter *critter = wave.spawnNext(map);
critter->move(map);
int reward = wave.attack(0, 100);
```

## Running the driver

Select the `TowerDefenseDriver` target in CLion and run it. It first runs the
Map, Tower and Critter tests in the console, printing `[PASS]` or `[FAIL]` for
each check and a summary at the end, and then opens the game window drawn
with SFML 3. CMake downloads and builds SFML automatically the first time the
project is configured, so an internet connection is needed once.

Game controls: `1`/`2`/`3` choose Direct, Splash or Hunter tower; left-click
grass to build; left-click a tower to upgrade it; right-click a tower to sell
it; `Space` starts the next wave; `R` restarts after game over.
