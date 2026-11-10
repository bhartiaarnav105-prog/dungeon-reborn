#include "dungeon/DungeonGenerator.hpp"
#include "interactables/Door.hpp"
#include "interactables/SpikeTrap.hpp"
#include "interactables/FireTrap.hpp"
#include "interactables/KeyPickup.hpp"
#include "items/Key.hpp"
#include "items/Weapon.hpp"
#include "items/Potion.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: ABSTRACTION & MODULARITY
// Why: Blueprints define tile-based maze topology with corridors,
// intersections, loops, and dead-ends. Keeping layout blueprints in
// dedicated member functions allows future procedural or file-based
// generators to replace them without altering Game or Player code.
// ============================================================

std::vector<std::string> DungeonGenerator::getEntranceLayout() {
    return {
        "+=========DD===========+", // row 0  (North passage to Room 3 at cols 10,11)
        "#.T.......ff........T..#", // row 1
        "#..####...ff...####....#", // row 2
        "#..#......ff......#....#", // row 3
        "#..#..##..ff..##..#....#", // row 4
        "#..#..##......##..#....D", // row 5  (East passage to Room 2 at col 23, rows 5-6)
        "#.P...f...##...f.......D", // row 6  (East passage to Room 2; player spawn at 2,6)
        "#.....f...##...f.......#", // row 7
        "#..#..##......##..#....#", // row 8
        "#..#..##..ff..##..#....#", // row 9
        "#..#......ff......#....#", // row 10
        "#..####...ff...####....#", // row 11
        "#.T.......ff........T..#", // row 12
        "[=========DD===========]"  // row 13 (South locked gate to Room 1 at cols 10,11)
    };
}

std::vector<std::string> DungeonGenerator::getBossSanctumLayout() {
    return {
        "+===========DD===========+", // row 0  (North entrance from Room 0 at cols 12,13)
        "#.B.........ff.........B.#", // row 1
        "#...........ff...........#", // row 2
        "#...###............###...#", // row 3  (North-West & North-East Cover Pillars)
        "#...###.....X......###...#", // row 4  (Exit stairs at 12,4 behind demon throne)
        "#...........T............#", // row 5
        "#........................#", // row 6
        "#...........E............#", // row 7  (Demon Boss at arena center 12,7)
        "#........................#", // row 8
        "#...###............###...#", // row 9  (South-West & South-East Cover Pillars)
        "#...###............###...#", // row 10
        "#........................#", // row 11
        "#.T.........ff.........T.#", // row 12
        "#...........ff...........#", // row 13
        "#...........ff...........#", // row 14
        "[========================]"  // row 15 (bottom boundary wall)
    };
}

std::vector<std::string> DungeonGenerator::getWarrenLayout() {
    return {
        "+==========================+", // row 0
        "#.T.......#......#.......T.#", // row 1  (upper loop corridor)
        "#..####...#..##..#...####..#", // row 2
        "#..#......#..##..#......#..#", // row 3
        "#..#.####.########.####.#..#", // row 4
        "D....#................#....D", // row 5  (West door from Room 0; East door to Room 4)
        "D..#.#.##...##...##.#.#.##.D", // row 6
        "#..#...##...##...##...#....#", // row 7
        "#.####.##############.####.#", // row 8  (central barrier requiring maze navigation)
        "#......#............#......#", // row 9  (long branch corridors)
        "#.####.#.####..####.#.####.#", // row 10
        "#.#....#....^^^^....#....#.#", // row 11 (hazardous spike trap shortcut)
        "#.#.##.####.####.####.##.#.#", // row 12
        "#...##................##...#", // row 13 (gathering hall before key altar)
        "#.T.......#..KK..#.......T.#", // row 14 (Boss Chamber Key at cols 13,14)
        "[==========================]"  // row 15
    };
}

std::vector<std::string> DungeonGenerator::getArmoryLayout() {
    return {
        "+====================+", // row 0
        "#.T.......$........T.#", // row 1  (Armory chest with Hunter's Bow at 10,1)
        "#..####...##...####..#", // row 2
        "#..#..............#..#", // row 3
        "#..#.####.##.####.#..#", // row 4
        "#....#....E.....#....#", // row 5  (Skeleton / Goblin patrol)
        "###..#.##.ff.##.#..###", // row 6  (tactical corridor chokepoints)
        "#....#....ff....#....#", // row 7
        "#..#.####.ff.####.#..#", // row 8
        "#..#......ff......#..#", // row 9
        "#..####...ff...####..#", // row 10
        "#.T.......ff.......T.#", // row 11
        "#.........ff.........#", // row 12
        "[=========DD=========]"  // row 13 (South exit to Room 0 at cols 10,11)
    };
}

std::vector<std::string> DungeonGenerator::getCryptLayout() {
    return {
        "+====================+", // row 0
        "#.B........$.......B.#", // row 1  (Master Crypt Chest at col 11, row 1)
        "#..####...####...##..#", // row 2
        "#..#..............#..#", // row 3
        "#..#.####.####.##.#..#", // row 4
        "D....#..........#....#", // row 5  (West entrance from Room 2 at col 0, rows 5-6)
        "D..#.#.########.#.##.#", // row 6
        "#..#.#....##....#.#..#", // row 7
        "#..#.####.##.####.#..#", // row 8
        "#..#..............#..#", // row 9
        "#..####...####...##..#", // row 10
        "#.T................T.#", // row 11
        "#..........T.........#", // row 12
        "[====================]"  // row 13
    };
}

// ============================================================
// OOP CONCEPT: FACTORY PATTERN & ABSTRACTION
// Why: generateRooms constructs concrete Room objects of diverse RoomTypes
// (Entrance, Boss, Combat, Treasure), connects doorways, and populates loot chests,
// locked doors, traps, and key pickups polymorphically.
//
// OOP CONCEPT: COMPOSITION
// Why: Each Room aggregates a collection of polymorphic Interactables
// (std::shared_ptr<Interactable>) whose lifecycles are tied to the Room.
// ============================================================
std::vector<Room> DungeonGenerator::generateRooms(float T, const sf::Texture* tileset) {
    std::vector<Room> rooms;

    // ------------------------------------------------------------------
    // Room 0: The Ancient Crossroads / Entry Crypt (RoomType::Entrance, 24x14)
    // EXPLORATION TOPOLOGY:
    // - North: Door to Room 3 ("The Forgotten Armory") at cols 10,11 row 0.
    // - East:  Door to Room 2 ("The Goblin Warren & Maze") at col 23, rows 5-6.
    // - South: LOCKED Boss Gate to Room 1 ("The Demon's Sanctum") at cols 10,11 row 13.
    //   Requires "boss_key" located deep in Room 2!
    // ------------------------------------------------------------------
    Room room0(0, "Entry Crypt", RoomType::Entrance, getEntranceLayout(), T);
    {
        // 1. North Doorway to Room 3 (Forgotten Armory)
        DoorInfo toArmoryDoor;
        toArmoryDoor.side         = DoorInfo::Side::North;
        toArmoryDoor.targetRoomId = 3;
        toArmoryDoor.triggerRect  = sf::FloatRect(10.f * T, 0.f, 2.f * T, 1.5f * T);
        toArmoryDoor.spawnOffset  = sf::Vector2f(10.5f * T, 11.5f * T);
        room0.addDoor(toArmoryDoor);

        // 2. East Doorway to Room 2 (Goblin Warren & Key Altar)
        DoorInfo toWarrenDoor;
        toWarrenDoor.side         = DoorInfo::Side::East;
        toWarrenDoor.targetRoomId = 2;
        toWarrenDoor.triggerRect  = sf::FloatRect(22.5f * T, 5.f * T, 1.5f * T, 2.f * T);
        toWarrenDoor.spawnOffset  = sf::Vector2f(2.0f * T, 5.5f * T);
        room0.addDoor(toWarrenDoor);

        // 3. South Doorway to Room 1 (Boss Sanctum) - Barred by Locked Boss Gate!
        DoorInfo toBossDoor;
        toBossDoor.side         = DoorInfo::Side::South;
        toBossDoor.targetRoomId = 1;
        toBossDoor.triggerRect  = sf::FloatRect(10.f * T, 12.f * T, 2.f * T, 2.f * T);
        toBossDoor.spawnOffset  = sf::Vector2f(12.5f * T, 2.0f * T);
        room0.addDoor(toBossDoor);
    }

    if (tileset) {
        // OOP CONCEPT: RUNTIME POLYMORPHISM & COMPOSITION
        // Why: Locked Door interactable blocks the South passage to the Boss Sanctum.
        // It requires the player to carry "boss_key" in inventory to unlock.
        auto bossGate = std::make_shared<Door>(
            "Boss Sanctum Gate",
            sf::Vector2f(10.f * T, 12.f * T),
            sf::Vector2f(2.f * T, 2.f * T),
            1,
            sf::Vector2f(12.5f * T, 2.0f * T),
            DoorState::Locked,
            "boss_key",
            tileset
        );
        room0.addInteractable(bossGate);

        // Ambient spike hazard in crossroads intersection
        room0.addInteractable(std::make_shared<SpikeTrap>(
            sf::Vector2f(11.f * T, 7.f * T),
            *tileset,
            10,
            1.5f
        ));
    }
    rooms.push_back(std::move(room0));

    // ------------------------------------------------------------------
    // Room 1: The Demon's Sanctum (RoomType::Boss, 26x16)
    // EXPLORATION TOPOLOGY:
    // - Unlocked only after claiming the Boss Key from Room 2!
    // - Grand hall with 4 tactical cover pillars and exit portal.
    // ------------------------------------------------------------------
    Room room1(1, "Boss Sanctum", RoomType::Boss, getBossSanctumLayout(), T);
    {
        // North Doorway back to Room 0 (Entry Crossroads)
        DoorInfo northDoor;
        northDoor.side         = DoorInfo::Side::North;
        northDoor.targetRoomId = 0;
        northDoor.triggerRect  = sf::FloatRect(11.5f * T, 0.f, 2.f * T, 1.5f * T);
        northDoor.spawnOffset  = sf::Vector2f(10.5f * T, 11.0f * T);
        room1.addDoor(northDoor);
    }

    if (tileset) {
        // Flame trap guarding the demonic throne dais
        room1.addInteractable(std::make_shared<FireTrap>(
            sf::Vector2f(12.5f * T, 8.5f * T),
            *tileset,
            18,
            2.0f,
            1.2f
        ));
    }
    rooms.push_back(std::move(room1));

    // ------------------------------------------------------------------
    // Room 2: The Goblin Warren & Key Altar (RoomType::Combat, 28x16)
    // EXPLORATION TOPOLOGY:
    // - Sprawling Pac-Man style maze with T-junctions, loops, and dead ends.
    // - West: Doorway back to Room 0.
    // - East: Doorway forward to Room 4 ("The Crypt of Shadows").
    // - Contains the Boss Chamber Key (boss_key) at the southern altar!
    // ------------------------------------------------------------------
    Room room2(2, "Goblin Warren", RoomType::Combat, getWarrenLayout(), T);
    {
        // 1. West Doorway to Room 0
        DoorInfo westDoor;
        westDoor.side         = DoorInfo::Side::West;
        westDoor.targetRoomId = 0;
        westDoor.triggerRect  = sf::FloatRect(0.f, 5.f * T, 1.5f * T, 2.f * T);
        westDoor.spawnOffset  = sf::Vector2f(21.5f * T, 5.5f * T);
        room2.addDoor(westDoor);

        // 2. East Doorway to Room 4 (Crypt of Shadows)
        DoorInfo eastDoor;
        eastDoor.side         = DoorInfo::Side::East;
        eastDoor.targetRoomId = 4;
        eastDoor.triggerRect  = sf::FloatRect(26.5f * T, 5.f * T, 1.5f * T, 2.f * T);
        eastDoor.spawnOffset  = sf::Vector2f(2.0f * T, 5.5f * T);
        room2.addDoor(eastDoor);
    }

    if (tileset) {
        // Spike traps guarding the dangerous corridor shortcut
        room2.addInteractable(std::make_shared<SpikeTrap>(
            sf::Vector2f(12.f * T, 11.f * T),
            *tileset,
            12,
            1.2f
        ));
        room2.addInteractable(std::make_shared<SpikeTrap>(
            sf::Vector2f(15.f * T, 11.f * T),
            *tileset,
            12,
            1.2f
        ));

        // Periodic fire trap
        room2.addInteractable(std::make_shared<FireTrap>(
            sf::Vector2f(13.5f * T, 7.f * T),
            *tileset,
            15,
            2.2f,
            1.4f
        ));

        // OOP CONCEPT: INHERITANCE, COMPOSITION & POLYMORPHISM
        // Why: KeyPickup is an Interactable holding a Key (which is an Item).
        // Placed at the key altar at (13.5, 14) for the player to collect.
        room2.addInteractable(std::make_shared<KeyPickup>(
            sf::Vector2f(13.5f * T, 14.f * T),
            Key::createBossKey(),
            *tileset
        ));
    }
    rooms.push_back(std::move(room2));

    // ------------------------------------------------------------------
    // Room 3: The Forgotten Armory (RoomType::Combat, 22x14)
    // EXPLORATION TOPOLOGY:
    // - North chamber with weapon supplies and ambushing skeletons.
    // - South: Doorway returning to Room 0.
    // - Armory chest holds the Hunter's Bow and a Health Potion.
    // ------------------------------------------------------------------
    Room room3(3, "Forgotten Armory", RoomType::Combat, getArmoryLayout(), T);
    {
        // South Doorway back to Room 0
        DoorInfo southDoor;
        southDoor.side         = DoorInfo::Side::South;
        southDoor.targetRoomId = 0;
        southDoor.triggerRect  = sf::FloatRect(10.f * T, 12.5f * T, 2.f * T, 1.5f * T);
        southDoor.spawnOffset  = sf::Vector2f(10.5f * T, 2.0f * T);
        room3.addDoor(southDoor);
    }

    if (tileset) {
        // Spike trap in the armory bottleneck
        room3.addInteractable(std::make_shared<SpikeTrap>(
            sf::Vector2f(10.5f * T, 7.f * T),
            *tileset,
            12,
            1.4f
        ));

        // Armory Chest containing Hunter's Bow + Health Potion
        std::vector<std::shared_ptr<Item>> armoryItems;
        armoryItems.push_back(Weapon::createHunterBow());
        armoryItems.push_back(Potion::createHealthPotion());
        room3.addChest(TreasureChest(sf::Vector2f(10.5f * T, 1.5f * T), *tileset, 40, std::move(armoryItems)));
    }
    rooms.push_back(std::move(room3));

    // ------------------------------------------------------------------
    // Room 4: The Crypt of Shadows (RoomType::Treasure, 22x14)
    // EXPLORATION TOPOLOGY:
    // - Far eastern high-tier treasure vault guarded by undead guards.
    // - West: Doorway back to Room 2.
    // - Master Crypt Chest holds the Golden Broadsword, Potions, and Gold.
    // ------------------------------------------------------------------
    Room room4(4, "Crypt of Shadows", RoomType::Treasure, getCryptLayout(), T);
    {
        // West Doorway back to Room 2
        DoorInfo westDoor;
        westDoor.side         = DoorInfo::Side::West;
        westDoor.targetRoomId = 2;
        westDoor.triggerRect  = sf::FloatRect(0.f, 5.f * T, 1.5f * T, 2.f * T);
        westDoor.spawnOffset  = sf::Vector2f(25.5f * T, 5.5f * T);
        room4.addDoor(westDoor);
    }

    if (tileset) {
        // Fire trap guarding the high crypt vault
        room4.addInteractable(std::make_shared<FireTrap>(
            sf::Vector2f(11.f * T, 7.f * T),
            *tileset,
            18,
            2.0f,
            1.2f
        ));

        // Master Crypt Chest with Golden Broadsword and 2 Health Potions
        std::vector<std::shared_ptr<Item>> cryptItems;
        cryptItems.push_back(Weapon::createGoldenBroadsword());
        cryptItems.push_back(Potion::createHealthPotion());
        cryptItems.push_back(Potion::createHealthPotion());
        room4.addChest(TreasureChest(sf::Vector2f(11.f * T, 1.5f * T), *tileset, 100, std::move(cryptItems)));
    }
    rooms.push_back(std::move(room4));

    return rooms;
}
