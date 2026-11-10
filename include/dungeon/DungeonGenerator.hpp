#pragma once

#include "dungeon/Room.hpp"
#include <vector>
#include <string>

// ============================================================
// OOP CONCEPT: SINGLE RESPONSIBILITY PRINCIPLE & ABSTRACTION
// Why: DungeonGenerator isolates dungeon generation, corridor design,
// room layout planning, door linking, and chest population into a dedicated
// class. Dungeon delegates generation to DungeonGenerator, keeping Dungeon
// focused on runtime simulation, entity updates, and rendering.
// ============================================================
class DungeonGenerator {
public:
    // ============================================================
    // OOP CONCEPT: ABSTRACTION
    // Why: generateRooms() completely abstracts the construction of the
    // complex multi-room maze. Callers receive fully initialized, linked Room objects.
    // ============================================================
    static std::vector<Room> generateRooms(float tileSize, const sf::Texture* tileset = nullptr);

    // Layout blueprints (Pac-Man style corridors, loops, intersections, dead ends)
    static std::vector<std::string> getEntranceLayout();
    static std::vector<std::string> getBossSanctumLayout();
    static std::vector<std::string> getWarrenLayout();
    static std::vector<std::string> getArmoryLayout();
    static std::vector<std::string> getCryptLayout();
};

