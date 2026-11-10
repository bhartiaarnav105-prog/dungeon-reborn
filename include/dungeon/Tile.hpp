#pragma once

#include <SFML/Graphics.hpp>

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: TileType is a strongly-typed enum class (a named enumeration)
// that makes tile classification readable and type-safe. Using an
// enum class prevents accidental mixing of integer types.
// ============================================================
enum class TileType {
    Empty,
    Floor,
    FloorAlt,
    WallTop,
    WallMid,
    WallSide,
    WallCornerTL,
    WallCornerTR,
    WallCornerBL,
    WallCornerBR,
    WallInnerCornerTL,
    WallInnerCornerTR,
    DoorClosed,
    DoorOpen,
    Torch,
    Banner,
    Exit,       // Dungeon exit / stone stairs
    Trap,       // Hidden / armed spike trap
    Treasure    // Chest marker or special loot tile
};

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: Tile is a value type (struct) that bundles all data
// describing a single tile cell: its type, whether it blocks
// movement, and the sub-rectangle of the tileset texture.
// Consumers cannot accidentally change isSolid independently
// of type because it is set only through createTile().
// ============================================================
struct Tile {
    TileType    type;
    bool        isSolid;
    sf::IntRect textureRect; // sub-rectangle in the tileset atlas
};

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: createTile() is a factory function that hides all the
// tileset coordinate look-up logic from consumers. Callers only
// say "give me a WallTop tile" and receive a fully configured Tile.
// ============================================================
Tile createTile(TileType t);

