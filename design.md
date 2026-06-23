# Clan Destiny - Design Document

Clan Destiny is a lightweight 4X game (eXplore, eXpand, eXploit, eXterminate) inspired by classics such as Civilization 1, Master of Magic, and Master of Orion. 

The core design goal is **speed**. A full game should be playable in approximately 90 minutes, significantly faster than traditional 4X titles.

The gameplay is intended to revolve around three main views:
- The Map View
- The Combat View
- The Upgrade View

Currently, only the **Map View** exists in code.

---

## High-Level Architecture

- **Engine**: Raylib 5.x (statically linked via prebuilt libraries in `ThirdParty/raylib`)
- **Rendering Target**: Fixed 640x360 internal resolution, upscaled 2x to 1280x720
- **Language**: C++17
- **Build System**: CMake (modeled after the U7Revisited project for consistency)
- **Asset Management**: Assets live in `Redist/` and are copied next to the executable at build time

### Core Data Model

The game is built around a small number of primary structs:

- **SquareTile** (`Map.h`): Represents one cell on the 74×46 world grid.
  - Position, terrain type, village ownership flags.

- **Terrain** (enum in `Game.h`): `WATER, DESERT, GRASSLAND, FOREST, SWAMP, HILLS, MOUNTAIN`

- **Clan** (`Clan.h`):
  - Name, color, stockpiles (`gold`, `knowledge`, `worship`)
  - List of owned village indices
  - `villageTile` Rectangle used for rendering clan icons on the map

- **Village** (`Clan.h`):
  - Location, name, owning clan
  - Population (starts at 4, max 12)
  - Resource outputs and storehouses (`foodStorehouse`, `productionStorehouse`)
  - Worker assignment state (`std::vector<bool> workers`)
  - List of `Building`s

- **Building** (`Clan.h`):
  - Type (`FARM, LOGGING_CAMP, MINE, WORSHIP_SITE, LIBRARY`)
  - Bonuses to food/production/gold/knowledge/worship
  - Production and upkeep costs
  - Assigned worker index and occupied map tile

- **Unit** (`Clan.h`):
  - Basic combat stats (`attackStrength`, `defenseStrength`, `movementPoints`)
  - List of `SpecialAbility` (currently defined: `BUILD_VILLAGE, FLY, CAST_SPELL, BUFF_STACK`)
  - Currently defined but not yet used in gameplay

- **SpecialAbility** (enum in `Game.h`)

### Constants (Game.h)

- World size: 74×46 tiles
- Internal resolution: 640×360 (scaled ×2 to 1280×720)
- Viewport: 15×11 tiles
- Minimap: 2×2 pixels per tile

---

## Current Systems

### Map Generation (`Map.cpp`)

- Cellular automata based generation:
  1. Seed ~50% of cells as land.
  2. Run 5 iterations of smoothing (cell becomes land if ≥4 land neighbors).
  3. Assign terrain types based on:
     - Number of land neighbors
     - Proximity to water
     - Random chance

- Currently places 4 hardcoded clans, each with 3 villages (1 "capital" + 2 outlying).
- Villages are placed with minimum distance rules.
- No units are placed during generation yet.

### Rendering (`Render.cpp` + `main.cpp`)

- Off-screen `RenderTexture2D` (640×360) is drawn to, then upscaled to the window.
- **Minimap**: Simple colored rectangles (clan color for villages, hardcoded blue/green for water/land).
- **Main View**: 15×11 tile window using a 16×16 tileset (`Images/tiles.png`).
  - Water is animated (4-frame cycle).
  - Terrain layers are drawn (base + overlay for hills/forest/etc.).
  - Village icons are drawn using the owning clan's `villageTile` rectangle from the tileset.
- **Clan Panel**: Currently hard-coded to show only the first clan ("Red Claw") stockpile information and per-turn income.

### Input

- Left-click and drag on the minimap pans the main view (with clamping to keep the view inside the map).
- No keyboard controls or other UI interaction implemented yet.

### Game Loop (`main.cpp`)

- Fixed 60 FPS.
- Currently a pure rendering + input loop. There is **no simulation, no turns, no resource accumulation, and no time progression**.

### Building System (partially implemented in `Clan.cpp`)

- `canBuild()` and `buildBuilding()` functions exist.
- They enforce:
  - Available worker
  - Production cost
  - Max 8 buildings per village (one per adjacent tile)
  - Terrain requirements per building type
  - No overlapping buildings
- When a building is constructed, it deducts production and assigns a worker.
- These functions are defined but **never called** from the current game loop.

---

## Planned Views (from design intent)

### THE MAP VIEW (Current + Future)

**Current State:**
- Exploration of a procedurally generated world.
- Village placement and basic rendering.
- Minimap navigation.

**Intended Future Features (inferred from structs and design goal):**
- Resource production and stockpiling over time.
- Building construction around villages.
- Unit movement and exploration.
- Expansion (founding new villages via `BUILD_VILLAGE` ability or similar).
- Multiple clans with different starting positions and possibly asymmetric abilities.

### THE COMBAT VIEW

Not yet implemented.

Expected to be a separate tactical layer (similar to Master of Magic or Civilization combat screens) where units fight on a smaller grid or using a simplified resolution system.

Will likely use the existing `Unit` struct with its attack/defense values and special abilities.

### THE UPGRADE VIEW

Not yet implemented.

Intended for technology, culture, or unit/building upgrades using the `knowledge` and `worship` resources.

---

## Technical Notes

- **Raylib Usage**: The project vendors a specific version of Raylib (headers + prebuilt static libs) in `ThirdParty/raylib`. The CMake configuration is deliberately kept simple and matches the pattern used in the related U7Revisited project.
- **No External Dependencies** beyond the vendored Raylib and the C++ standard library.
- **State Management**: Almost all game state currently lives in global-scope vectors in `main.cpp` (`map`, `clans`, `villages`, `units`). There is no central `GameState` or `World` class yet.
- **No Serialization**: Saving/loading does not exist.

---

## Current Limitations / Work in Progress

- No game loop / turn structure.
- No resource simulation over time.
- Building system is implemented in isolation but not connected to gameplay.
- Unit system is declared but unused.
- Only one clan's resources are displayed.
- No exploration mechanics, fog of war, or diplomacy.
- Hardcoded clan data and starting positions.

This document should be updated as new systems are implemented.

VILLAGES AND UNITS

All villages start with a population of 1.  All villages produce the following per turn:
1 Gold (goes into global store)
1 Knowledge (goes into global store)
1 Worship (goes into global store)
1 Food (goes into village store)
1 Production (goes into village store)

When the food store gets to 10 * the current villager amount, it empties and another villager is added to the village, up to 8.  So the village starts with 1 villager, and when it produces 10 food it goes to 2, and when it produces 20 food it goes to 3, etc.

Production is used to build all buildings around a village.  The buildings should be:
Farm - Requires grassland - produces 1 extra food per turn (2 for Glendwellers)
Logging Camp - Requires forest tile - produces 1 extra production per turn
Mine - Requires hills - produces 1 extra gold per turn (2 for Gilded)
Worship Site - Requires hills - produces 1 extra Worship per turn
Library - Requires grassland - produces 1 extra Knowledcge per turn

Each building requires the approprate terrain tile and a certain amount of production to be built.  A village cannot have more buildings than it has population.

Villages can also make units.  Units are:

Settlers - Use to make new villages.
Spearmen - Basic infantry.  Unarmored, but fast.  Good counter to archers.
Archers - Basic ranged unit.  Can shoot arrows across the battlefield but don't wear armor and don't have a good melee attack.  Good for countering swordsmen, since they are slow.
Swordsmen - Armed and armored warriors.  Slow, but do a lot of damage. Can easily defeat any other type of unit...if they can get close enough to them.  Natural counter to spearmen.
Shaman - A mystic who can help out in battle and can be upgraded using Worship to become more powerful.

All units cost gold.

Clicking on a village on the map shows a panel on the left below the clan information with information on that particular village - its population and how much it's making of each production per turn.

When a village is first created by a settler, the settler disappears and the village has a starting population of 1.  So it can handle one additional building until its population grows to 2, etc.

All units can move across the main map at 1 tile per turn, but on the combat map, different units will have different move rates.
