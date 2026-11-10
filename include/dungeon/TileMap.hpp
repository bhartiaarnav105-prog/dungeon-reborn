#pragma once

#include "dungeon/Tile.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: TileMap is a self-contained class responsible for a 2D
// grid of Tile objects. It manages loading, rendering, and
// spatial queries. This gives it a single, clear responsibility.
//
// OOP CONCEPT: COMPOSITION
// Why: TileMap has-a 2D vector of Tile objects (m_grid). The
// relationship is composition: the Tiles exist as part of TileMap
// and are destroyed when TileMap is destroyed.
//
// OOP CONCEPT: ENCAPSULATION
// Why: The raw grid (m_grid) and dimensions are private. All
// interaction is through typed public methods that enforce
// invariants (e.g., bounds checking in getTileAt).
// ============================================================
class TileMap {
private:
    std::vector<std::vector<Tile>> m_grid; // row-major: m_grid[row][col]
    int   m_cols;
    int   m_rows;
    float m_tileSize; // display size in pixels (16 * scale)

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Default constructor creates an empty map. The parameterised
    // overload loads a layout immediately via loadFromGrid(), keeping
    // TileMap in a valid state from the moment of construction.
    // ============================================================
    TileMap();
    TileMap(const std::vector<std::string>& asciiLayout, float tileSize);

    // ============================================================
    // OOP CONCEPT: ABSTRACTION
    // Why: loadFromGrid() hides the character-to-TileType mapping
    // so callers design maps using simple ASCII strings instead of
    // constructing Tile objects manually.
    // ============================================================
    void loadFromGrid(const std::vector<std::string>& asciiLayout, float tileSize);

    // Render all tiles using the provided tileset texture.
    void render(sf::RenderTarget& target, const sf::Texture& tileset) const;

    // ============================================================
    // OOP CONCEPT: ENCAPSULATION (CONST QUERIES)
    // Why: These are pure queries — they inspect state without
    // modifying it, so they are marked const for const-correctness.
    // ============================================================
    bool          isSolid(int col, int row) const;
    bool          isWalkable(int col, int row) const;
    bool          isWalkable(sf::Vector2f worldPos) const;
    bool          isAreaWalkable(const sf::FloatRect& bounds) const;
    const Tile&   getTileAt(int col, int row) const;
    sf::Vector2i  worldToTile(sf::Vector2f worldPos) const;

    // Returns all AABB rects of solid tiles for collision resolution.
    std::vector<sf::FloatRect> getSolidRects() const;

    // Returns spawn positions encoded as 'E' cells in the ASCII layout.
    std::vector<sf::Vector2f> getEnemySpawnPositions() const;

    // Returns the first 'P' cell position, or (0,0) if not found.
    sf::Vector2f getPlayerSpawnPosition() const;

    int   getCols()     const { return m_cols; }
    int   getRows()     const { return m_rows; }
    float getTileSize() const { return m_tileSize; }

    // Total pixel dimensions of this map
    float getPixelWidth()  const { return m_cols * m_tileSize; }
    float getPixelHeight() const { return m_rows * m_tileSize; }

private:
    // Helper: convert ASCII character to TileType
    static TileType charToTileType(char c);
};

