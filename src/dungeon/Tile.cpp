#include "dungeon/Tile.hpp"

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: This factory centralises all tileset coordinate knowledge.
// The 0x72 Dungeon Tileset II (CC0) uses a 16x16 pixel grid.
// Each sf::IntRect is (left, top, width, height) in that atlas.
// ============================================================
Tile createTile(TileType t) {
    Tile tile;
    tile.type    = t;
    tile.isSolid = false;

    switch (t) {
        // ---- Floors (walkable) ----
        case TileType::Floor:
            tile.textureRect = sf::IntRect(16, 64, 16, 16); // floor centre variant
            tile.isSolid     = false;
            break;

        case TileType::FloorAlt:
            tile.textureRect = sf::IntRect(32, 64, 16, 16); // floor alt variant
            tile.isSolid     = false;
            break;

        // ---- Walls (solid) ----
        case TileType::WallTop:
            tile.textureRect = sf::IntRect(32, 0,  16, 16); // top of wall
            tile.isSolid     = true;
            break;

        case TileType::WallMid:
            tile.textureRect = sf::IntRect(32, 16, 16, 16); // mid wall face
            tile.isSolid     = true;
            break;

        case TileType::WallSide:
            tile.textureRect = sf::IntRect(16, 16, 16, 16); // side wall
            tile.isSolid     = true;
            break;

        case TileType::WallCornerTL:
            tile.textureRect = sf::IntRect(32, 112, 16, 16);
            tile.isSolid     = true;
            break;

        case TileType::WallCornerTR:
            tile.textureRect = sf::IntRect(48, 112, 16, 16);
            tile.isSolid     = true;
            break;

        case TileType::WallCornerBL:
            tile.textureRect = sf::IntRect(32, 128, 16, 16);
            tile.isSolid     = true;
            break;

        case TileType::WallCornerBR:
            tile.textureRect = sf::IntRect(48, 128, 16, 16);
            tile.isSolid     = true;
            break;

        case TileType::WallInnerCornerTL:
            tile.textureRect = sf::IntRect(32, 144, 16, 16);
            tile.isSolid     = true;
            break;

        case TileType::WallInnerCornerTR:
            tile.textureRect = sf::IntRect(48, 144, 16, 16);
            tile.isSolid     = true;
            break;

        // ---- Doors (not solid — player walks through) ----
        case TileType::DoorClosed:
            tile.textureRect = sf::IntRect(32, 224, 16, 16); // door leaf closed (left half)
            tile.isSolid     = false;
            break;

        case TileType::DoorOpen:
            tile.textureRect = sf::IntRect(80, 224, 16, 16); // door leaf open (left half)
            tile.isSolid     = false;
            break;

        // ---- Torch (decorative, not solid) ----
        case TileType::Torch:
            tile.textureRect = sf::IntRect(64, 16, 16, 16); // wall_fountain_mid_red frame 0
            tile.isSolid     = false;
            break;

        // ---- Banner (decorative wall decoration) ----
        case TileType::Banner:
            tile.textureRect = sf::IntRect(16, 32, 16, 16); // red banner
            tile.isSolid     = false;
            break;

        // ---- Dungeon Exit / Stairs (walkable portal) ----
        case TileType::Exit:
            tile.textureRect = sf::IntRect(48, 96, 16, 16); // descending stone stairs
            tile.isSolid     = false;
            break;

        // ---- Spike Trap (dangerous obstacle, walkable) ----
        case TileType::Trap:
            tile.textureRect = sf::IntRect(32, 208, 16, 16); // floor spike trap
            tile.isSolid     = false;
            break;

        // ---- Treasure marker (chest / pedestal) ----
        case TileType::Treasure:
            tile.textureRect = sf::IntRect(304, 288, 16, 16); // golden chest
            tile.isSolid     = false;
            break;

        // ---- Empty / unknown ----
        case TileType::Empty:
        default:
            tile.textureRect = sf::IntRect(0, 0, 16, 16);
            tile.isSolid     = false;
            break;
    }

    return tile;
}

