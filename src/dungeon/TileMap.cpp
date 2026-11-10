#include "dungeon/TileMap.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS
// Why: Default constructor leaves the grid empty with zero
// dimensions; it is valid to call loadFromGrid() later.
// ============================================================
TileMap::TileMap()
    : m_cols(0), m_rows(0), m_tileSize(48.0f), m_verticesDirty(true) {}

TileMap::TileMap(const std::vector<std::string>& asciiLayout, float tileSize)
    : m_cols(0), m_rows(0), m_tileSize(tileSize), m_verticesDirty(true) {
    loadFromGrid(asciiLayout, tileSize);
}

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: Converts a human-readable ASCII grid into a 2D Tile array.
// The character encoding is an implementation detail hidden here.
//
// ASCII legend:
//   '#' = WallMid (solid)      '@' = WallTop (solid)
//   '+' = WallCornerTL         '=' = WallCornerTR
//   '[' = WallCornerBL         ']' = WallCornerBR
//   '|' = WallSide             'D' = DoorClosed (not solid)
//   '.' = Floor                'f' = FloorAlt
//   'T' = Torch                'B' = Banner
//   'E' = Enemy spawn (floor)  'P' = Player spawn (floor)
//   ' ' = Empty
// ============================================================
void TileMap::loadFromGrid(const std::vector<std::string>& layout, float tileSize) {
    m_tileSize = tileSize;
    m_rows     = static_cast<int>(layout.size());
    m_cols     = m_rows > 0 ? static_cast<int>(layout[0].size()) : 0;

    m_grid.clear();
    m_grid.resize(m_rows, std::vector<Tile>(m_cols));

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            char ch = (c < static_cast<int>(layout[r].size())) ? layout[r][c] : ' ';
            m_grid[r][c] = createTile(charToTileType(ch));
        }
    }
    m_verticesDirty = true;
}

// Convert ASCII character → TileType
TileType TileMap::charToTileType(char c) {
    switch (c) {
        case '#': return TileType::WallMid;
        case '@': return TileType::WallTop;
        case '+': return TileType::WallCornerTL;
        case '=': return TileType::WallCornerTR;
        case '[': return TileType::WallCornerBL;
        case ']': return TileType::WallCornerBR;
        case '<': return TileType::WallInnerCornerTL;
        case '>': return TileType::WallInnerCornerTR;
        case '|': return TileType::WallSide;
        case 'D': return TileType::DoorOpen;   // door tile (open arch = passable)
        case '.': return TileType::Floor;
        case 'f': return TileType::FloorAlt;
        case 'T': return TileType::Torch;
        case 'B': return TileType::Banner;
        case '^': return TileType::Trap;
        case 'X': return TileType::Exit;
        case '$': return TileType::Floor;      // chest spawn location
        case 'K': return TileType::Floor;      // key altar location
        case 'E': return TileType::Floor;      // enemy spawn → rendered as floor
        case 'P': return TileType::Floor;      // player spawn → rendered as floor
        default:  return TileType::Empty;
    }
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION & PERFORMANCE CACHING
// Why: rebuildVertexArrays batches all tile geometry into a single sf::VertexArray.
// Instead of issuing 600-1000 separate draw calls with individual sf::Sprite instances,
// all quads are submitted to the GPU in exactly 2 batched draw calls, giving 60 FPS in WebAssembly.
// ============================================================
void TileMap::rebuildVertexArrays() const {
    m_tileVertices.setPrimitiveType(sf::Quads);
    m_tileVertices.clear();

    m_shadowVertices.setPrimitiveType(sf::Quads);
    m_shadowVertices.clear();

    // 1. Build Base Tile Quads
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            const Tile& tile = m_grid[r][c];
            if (tile.type == TileType::Empty) continue;

            sf::Color tileColor;
            if (tile.isSolid) {
                if (tile.type == TileType::WallTop || tile.type == TileType::WallCornerTL || tile.type == TileType::WallCornerTR) {
                    tileColor = sf::Color(100, 115, 145);
                } else {
                    tileColor = sf::Color(75, 85, 115);
                }
            } else if (tile.type == TileType::Floor) {
                tileColor = sf::Color(215, 222, 235);
            } else if (tile.type == TileType::FloorAlt) {
                tileColor = sf::Color(185, 195, 218);
            } else if (tile.type == TileType::DoorOpen || tile.type == TileType::DoorClosed) {
                tileColor = sf::Color(255, 215, 100);
            } else if (tile.type == TileType::Trap) {
                tileColor = sf::Color(255, 95, 80);
            } else if (tile.type == TileType::Torch) {
                tileColor = sf::Color(255, 225, 130);
            } else if (tile.type == TileType::Banner) {
                tileColor = sf::Color(235, 70, 80);
            } else if (tile.type == TileType::Exit) {
                tileColor = sf::Color(120, 240, 255);
            } else {
                tileColor = sf::Color::White;
            }

            float px = c * m_tileSize;
            float py = r * m_tileSize;
            float u0 = static_cast<float>(tile.textureRect.left);
            float v0 = static_cast<float>(tile.textureRect.top);
            float u1 = u0 + static_cast<float>(tile.textureRect.width);
            float v1 = v0 + static_cast<float>(tile.textureRect.height);

            // Add 4 vertices for the tile quad
            m_tileVertices.append(sf::Vertex(sf::Vector2f(px, py), tileColor, sf::Vector2f(u0, v0)));
            m_tileVertices.append(sf::Vertex(sf::Vector2f(px + m_tileSize, py), tileColor, sf::Vector2f(u1, v0)));
            m_tileVertices.append(sf::Vertex(sf::Vector2f(px + m_tileSize, py + m_tileSize), tileColor, sf::Vector2f(u1, v1)));
            m_tileVertices.append(sf::Vertex(sf::Vector2f(px, py + m_tileSize), tileColor, sf::Vector2f(u0, v1)));
        }
    }

    // 2. Build Wall Drop-Shadow Quads
    const sf::Color shadowColor(15, 18, 30, 120);
    const float shadowHeight = 6.0f;
    for (int r = 0; r < m_rows - 1; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (m_grid[r][c].isSolid && !m_grid[r + 1][c].isSolid) {
                float px = c * m_tileSize;
                float py = (r + 1) * m_tileSize;

                m_shadowVertices.append(sf::Vertex(sf::Vector2f(px, py), shadowColor));
                m_shadowVertices.append(sf::Vertex(sf::Vector2f(px + m_tileSize, py), shadowColor));
                m_shadowVertices.append(sf::Vertex(sf::Vector2f(px + m_tileSize, py + shadowHeight), shadowColor));
                m_shadowVertices.append(sf::Vertex(sf::Vector2f(px, py + shadowHeight), shadowColor));
            }
        }
    }

    m_verticesDirty = false;
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION & ABSTRACTION
// Why: Renders the entire room's tiles and shadows using GPU-batched vertex arrays.
// Reduces WebGL draw calls from ~1000 down to 2, ensuring smooth 60 FPS performance.
// ============================================================
void TileMap::render(sf::RenderTarget& target, const sf::Texture& tileset) const {
    if (m_verticesDirty) {
        rebuildVertexArrays();
    }
    target.draw(m_tileVertices, &tileset);
    if (m_shadowVertices.getVertexCount() > 0) {
        target.draw(m_shadowVertices);
    }
}

// ============================================================
// OOP CONCEPT: CONST-CORRECTNESS
// Why: isSolid and getTileAt are pure queries and are const.
// ============================================================
bool TileMap::isSolid(int col, int row) const {
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return true; // out of bounds = solid
    return m_grid[row][col].isSolid;
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION & ABSTRACTION
// Why: isWalkable provides high-level queries for navigation and path validation.
// ============================================================
bool TileMap::isWalkable(int col, int row) const {
    return !isSolid(col, row);
}

bool TileMap::isWalkable(sf::Vector2f worldPos) const {
    sf::Vector2i tile = worldToTile(worldPos);
    return isWalkable(tile.x, tile.y);
}

bool TileMap::isAreaWalkable(const sf::FloatRect& bounds) const {
    int minCol = static_cast<int>(bounds.left / m_tileSize);
    int maxCol = static_cast<int>((bounds.left + bounds.width - 0.001f) / m_tileSize);
    int minRow = static_cast<int>(bounds.top / m_tileSize);
    int maxRow = static_cast<int>((bounds.top + bounds.height - 0.001f) / m_tileSize);

    for (int r = minRow; r <= maxRow; ++r) {
        for (int c = minCol; c <= maxCol; ++c) {
            if (isSolid(c, r)) return false;
        }
    }
    return true;
}

const Tile& TileMap::getTileAt(int col, int row) const {
    // Clamp to valid range to avoid out-of-bounds access
    int r = std::max(0, std::min(row, m_rows - 1));
    int c = std::max(0, std::min(col, m_cols - 1));
    return m_grid[r][c];
}

sf::Vector2i TileMap::worldToTile(sf::Vector2f worldPos) const {
    return sf::Vector2i(
        static_cast<int>(worldPos.x / m_tileSize),
        static_cast<int>(worldPos.y / m_tileSize)
    );
}

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: Callers request solid AABBs without knowing the grid's
// internal structure. Used by CollisionSystem.
// ============================================================
std::vector<sf::FloatRect> TileMap::getSolidRects() const {
    std::vector<sf::FloatRect> rects;
    rects.reserve(m_rows * m_cols / 4); // rough estimate

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (m_grid[r][c].isSolid) {
                rects.emplace_back(
                    c * m_tileSize, r * m_tileSize,
                    m_tileSize,     m_tileSize
                );
            }
        }
    }
    return rects;
}

// Returns world-space positions of all 'E' (enemy spawn) cells
std::vector<sf::Vector2f> TileMap::getEnemySpawnPositions() const {
    // NOTE: enemy spawn cells are stored as Floor tiles after loading;
    // we do NOT track the original character. The Room class stores
    // spawn positions separately from the parsed layout. This method
    // is a no-op here — Room::buildRooms() captures spawn positions
    // while parsing the ASCII grid before calling loadFromGrid().
    return {};
}

sf::Vector2f TileMap::getPlayerSpawnPosition() const {
    // Similarly delegated to Room. Returns (0,0) as a safe default.
    return sf::Vector2f(0.f, 0.f);
}

