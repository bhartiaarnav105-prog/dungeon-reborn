// test_dungeon.cpp
// Phase 3 unit tests for the dungeon system.
// Tests: CollisionSystem, TileMap, Room, and Dungeon.

#include "dungeon/Tile.hpp"
#include "dungeon/TileMap.hpp"
#include "dungeon/Room.hpp"
#include "dungeon/DungeonGenerator.hpp"
#include "dungeon/CollisionSystem.hpp"
#include "dungeon/Dungeon.hpp"
#include <iostream>
#include <cassert>
#include <string>

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: Each test function encapsulates a specific scenario so
// main() reads as a checklist rather than a wall of assertions.
// ============================================================

// ----------------------------------------------------------------
// Test 1: createTile factory correctness
// ----------------------------------------------------------------
static void testTileFactory() {
    std::cout << "  [Test] Tile factory...\n";

    Tile wall = createTile(TileType::WallMid);
    assert(wall.isSolid  && "WallMid must be solid");
    assert(wall.type == TileType::WallMid);

    Tile floor = createTile(TileType::Floor);
    assert(!floor.isSolid && "Floor must not be solid");
    assert(floor.type == TileType::Floor);

    Tile door = createTile(TileType::DoorOpen);
    assert(!door.isSolid && "DoorOpen must not be solid (player passes through)");

    Tile trap = createTile(TileType::Trap);
    assert(!trap.isSolid && "Trap must not be solid (triggerable floor hazard)");
    assert(trap.type == TileType::Trap);

    Tile exitTile = createTile(TileType::Exit);
    assert(!exitTile.isSolid && "Exit must not be solid (passable portal)");
    assert(exitTile.type == TileType::Exit);

    Tile treasure = createTile(TileType::Treasure);
    assert(!treasure.isSolid && "Treasure must not be solid");
    assert(treasure.type == TileType::Treasure);

    std::cout << "  [PASS] Tile factory\n";
}

// ----------------------------------------------------------------
// Test 2: TileMap isSolid() responds correctly to ASCII layout
// ----------------------------------------------------------------
static void testTileMapSolid() {
    std::cout << "  [Test] TileMap isSolid...\n";

    std::vector<std::string> layout = {
        "###",
        "#.#",
        "###"
    };
    TileMap map(layout, 48.f);

    // Corners and edges are '#' = WallMid = solid
    assert(map.isSolid(0, 0) && "Top-left corner must be solid");
    assert(map.isSolid(2, 2) && "Bottom-right corner must be solid");

    // Centre is '.' = Floor = not solid
    assert(!map.isSolid(1, 1) && "Centre floor must not be solid");
    assert(map.isWalkable(1, 1) && "Centre floor must be walkable");
    assert(!map.isWalkable(0, 0) && "Wall must not be walkable");

    // Out-of-bounds should be treated as solid (safe default)
    assert(map.isSolid(-1, 0)  && "Out-of-bounds col must be solid");
    assert(map.isSolid(0, -1)  && "Out-of-bounds row must be solid");
    assert(map.isSolid(99, 99) && "Far out-of-bounds must be solid");

    std::cout << "  [PASS] TileMap isSolid\n";
}

// ----------------------------------------------------------------
// Test 3: TileMap getSolidRects() returns correct count
// ----------------------------------------------------------------
static void testTileMapSolidRects() {
    std::cout << "  [Test] TileMap getSolidRects count...\n";

    // 3x3 grid: 8 wall tiles, 1 floor tile
    std::vector<std::string> layout = {
        "###",
        "#.#",
        "###"
    };
    TileMap map(layout, 48.f);
    auto rects = map.getSolidRects();
    // 8 solid tiles expected
    assert(rects.size() == 8 && "3x3 box with 1 floor: 8 solid rects expected");

    std::cout << "  [PASS] TileMap getSolidRects count\n";
}

// ----------------------------------------------------------------
// Test 4: CollisionSystem — no movement into a solid wall
// ----------------------------------------------------------------
static void testCollisionNoPassThrough() {
    std::cout << "  [Test] CollisionSystem blocks movement into wall...\n";

    // A wall tile at (96, 0, 48, 48)
    std::vector<sf::FloatRect> solids = {
        sf::FloatRect(96.f, 0.f, 48.f, 48.f)
    };

    // Player bounds just left of the wall, moving right into it
    sf::FloatRect playerBounds(50.f, 10.f, 32.f, 32.f); // right edge at 82
    sf::Vector2f  desiredMove(50.f, 0.f);                 // would put right edge at 132

    sf::Vector2f safe = CollisionSystem::resolveCollision(playerBounds, desiredMove, solids);

    // Right edge after safe move: 50 + safe.x + 32 <= 96
    float newRight = playerBounds.left + safe.x + playerBounds.width;
    assert(newRight <= 96.f + 0.5f && "Character must not penetrate wall to the right");
    assert(safe.x >= 0.f           && "Safe move X must not be negative (not moving left)");

    std::cout << "  [PASS] CollisionSystem blocks rightward movement\n";
}

// ----------------------------------------------------------------
// Test 5: CollisionSystem — free movement when no walls nearby
// ----------------------------------------------------------------
static void testCollisionFreeMovement() {
    std::cout << "  [Test] CollisionSystem allows free movement...\n";

    std::vector<sf::FloatRect> solids; // empty — open space

    sf::FloatRect playerBounds(100.f, 100.f, 32.f, 32.f);
    sf::Vector2f  desiredMove(30.f, 20.f);

    sf::Vector2f safe = CollisionSystem::resolveCollision(playerBounds, desiredMove, solids);

    // With no solids, safe move should equal desired move
    assert(std::abs(safe.x - desiredMove.x) < 0.01f && "Free X movement must be unaffected");
    assert(std::abs(safe.y - desiredMove.y) < 0.01f && "Free Y movement must be unaffected");

    std::cout << "  [PASS] CollisionSystem free movement\n";
}

// ----------------------------------------------------------------
// Test 6: Room spawn positions are parsed correctly
// ----------------------------------------------------------------
static void testRoomSpawnPositions() {
    std::cout << "  [Test] Room spawn position parsing...\n";

    std::vector<std::string> layout = {
        "#####",
        "#P.E#",
        "#####"
    };
    Room room(0, "Test Room", layout, 48.f);

    sf::Vector2f pSpawn = room.getPlayerSpawnPosition();
    // 'P' is at col 1, row 1 → centre = (1*48 + 24, 1*48 + 24) = (72, 72)
    assert(std::abs(pSpawn.x - 72.f) < 1.f && "Player spawn X should be col 1 centre");
    assert(std::abs(pSpawn.y - 72.f) < 1.f && "Player spawn Y should be row 1 centre");

    const auto& eSpawns = room.getEnemySpawnPositions();
    assert(eSpawns.size() == 1 && "One 'E' should produce one enemy spawn position");
    // 'E' is at col 3, row 1 → centre = (3*48 + 24, 1*48 + 24) = (168, 72)
    assert(std::abs(eSpawns[0].x - 168.f) < 1.f && "Enemy spawn X should be col 3 centre");

    std::cout << "  [PASS] Room spawn positions\n";
}

// ----------------------------------------------------------------
// Test 7: Dungeon has the correct number of rooms (5-Room Architecture)
// ----------------------------------------------------------------
static void testDungeonRoomCount() {
    std::cout << "  [Test] Dungeon has 5 rooms...\n";
    // Note: Dungeon requires the SFML window subsystem for the camera view only —
    // constructing it in a test is safe as long as we don't open a window.
    Dungeon dungeon(960, 640);
    assert(dungeon.getRoomCount() == 5 && "Dungeon must have exactly 5 rooms");
    std::cout << "  [PASS] Dungeon room count\n";
}

// ----------------------------------------------------------------
// Test 8: Dungeon door trigger rects are non-empty
// ----------------------------------------------------------------
static void testDungeonDoorTriggers() {
    std::cout << "  [Test] Dungeon door trigger rects are non-empty...\n";
    Dungeon dungeon(960, 640);

    // Check all rooms have at least one door with a non-zero trigger rect
    for (int id = 0; id < dungeon.getRoomCount(); ++id) {
        const Room* room = dungeon.getRoomById(id);
        assert(room != nullptr && "getRoomById must return valid room");
        const auto& doors = room->getDoors();
        // Room 0 and 1 and 2 each have at least one door
        assert(!doors.empty() && "Every room must have at least one door");
        for (const auto& d : doors) {
            assert(d.triggerRect.width  > 0.f && "Door trigger width must be > 0");
            assert(d.triggerRect.height > 0.f && "Door trigger height must be > 0");
        }
    }
    std::cout << "  [PASS] Door trigger rects\n";
}

// ----------------------------------------------------------------
// Test 9: CollisionSystem::isOverlapping
// ----------------------------------------------------------------
static void testIsOverlapping() {
    std::cout << "  [Test] CollisionSystem::isOverlapping...\n";

    sf::FloatRect a(0.f, 0.f, 50.f, 50.f);
    sf::FloatRect b(25.f, 25.f, 50.f, 50.f);
    sf::FloatRect c(100.f, 100.f, 50.f, 50.f);

    assert( CollisionSystem::isOverlapping(a, b) && "Overlapping rects must report true");
    assert(!CollisionSystem::isOverlapping(a, c) && "Non-overlapping rects must report false");
    std::cout << "  [PASS] isOverlapping\n";
}

// ----------------------------------------------------------------
// Test 10: DungeonGenerator and RoomType categorization
// ----------------------------------------------------------------
static void testDungeonGeneratorAndRoomTypes() {
    std::cout << "  [Test] DungeonGenerator and RoomType classification...\n";
    auto rooms = DungeonGenerator::generateRooms(48.f);
    assert(rooms.size() == 5 && "DungeonGenerator must generate 5 rooms");
    assert(rooms[0].getType() == RoomType::Entrance && "Room 0 must be Entrance");
    assert(rooms[1].getType() == RoomType::Boss     && "Room 1 must be Boss");
    assert(rooms[2].getType() == RoomType::Combat   && "Room 2 must be Combat");
    assert(rooms[3].getType() == RoomType::Combat   && "Room 3 must be Combat");
    assert(rooms[4].getType() == RoomType::Treasure && "Room 4 must be Treasure");
    assert(rooms[0].isWalkable(rooms[0].getPlayerSpawnPosition()) && "Spawn position must be walkable");

    Dungeon dungeon(960, 640);
    assert(dungeon.isWalkable(rooms[0].getPlayerSpawnPosition()) && "Dungeon::isWalkable must return true for player spawn");
    std::cout << "  [PASS] DungeonGenerator and RoomType classification\n";
}

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: main() is a simple test runner that calls each test
// function; it does not contain any test logic itself.
// ============================================================
int main() {
    std::cout << "\n========================================\n";
    std::cout << "  PHASE 3 DUNGEON SYSTEM TESTS\n";
    std::cout << "========================================\n\n";

    testTileFactory();
    testTileMapSolid();
    testTileMapSolidRects();
    testCollisionNoPassThrough();
    testCollisionFreeMovement();
    testRoomSpawnPositions();
    testDungeonRoomCount();
    testDungeonDoorTriggers();
    testIsOverlapping();
    testDungeonGeneratorAndRoomTypes();

    std::cout << "\n========================================\n";
    std::cout << "  ALL PHASE 3 DUNGEON TESTS PASSED!\n";
    std::cout << "========================================\n\n";
    return 0;
}

