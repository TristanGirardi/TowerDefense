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