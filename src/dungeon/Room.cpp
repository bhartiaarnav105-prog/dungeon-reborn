#include "dungeon/Room.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS
// Why: The Room constructor delegates tile loading to TileMap
// through its own constructor in the initialiser list, then
// parses spawn positions from the same layout string.
//
// OOP CONCEPT: COMPOSITION
// Why: m_tileMap is constructed here as a member sub-object —
// the Room does not allocate it on the heap; it owns it directly.
// ============================================================
Room::Room(int id,
           const std::string& name,
           RoomType type,
           const std::vector<std::string>& layout,
           float tileSize)
    : m_id(id)
    , m_name(name)
    , m_type(type)
    , m_tileMap(layout, tileSize)
    , m_playerSpawnPosition(96.f, 96.f)
{
    parseSpawnPositions(layout, tileSize);
    std::cout << "[Room] Created room " << m_id << " \"" << m_name << "\" ("
              << m_tileMap.getCols() << "x" << m_tileMap.getRows() << " tiles, "
              << m_enemySpawnPositions.size() << " enemy spawns)\n";
}

Room::Room(int id,
           const std::string& name,
           const std::vector<std::string>& layout,
           float tileSize)
    : Room(id, name, RoomType::Combat, layout, tileSize)
{
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION & CONST-CORRECTNESS
// Why: Provides direct bounds and walkability queries without
// leaking the inner grid representation.
// ============================================================
sf::FloatRect Room::getBounds() const {
    return sf::FloatRect(0.f, 0.f, m_tileMap.getPixelWidth(), m_tileMap.getPixelHeight());
}

bool Room::isWalkable(sf::Vector2f worldPos) const {
    return m_tileMap.isWalkable(worldPos);
}

bool Room::isWalkable(int col, int row) const {
    return m_tileMap.isWalkable(col, row);
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: parseSpawnPositions is a private helper called only from
// the constructor. It extracts 'P' and 'E' positions from the raw
// ASCII layout before the data is discarded, so the rest of the
// system never needs to look at raw layout strings again.
// ============================================================
void Room::parseSpawnPositions(const std::vector<std::string>& layout, float tileSize) {
    for (int r = 0; r < static_cast<int>(layout.size()); ++r) {
        for (int c = 0; c < static_cast<int>(layout[r].size()); ++c) {
            char ch = layout[r][c];
            // Centre of the tile in world space
            float worldX = c * tileSize + tileSize * 0.5f;
            float worldY = r * tileSize + tileSize * 0.5f;

            if (ch == 'P') {
                m_playerSpawnPosition = sf::Vector2f(worldX, worldY);
            } else if (ch == 'E') {
                m_enemySpawnPositions.emplace_back(worldX, worldY);
            }
        }
    }
}

void Room::render(sf::RenderTarget& target, const sf::Texture& tileset) const {
    m_tileMap.render(target, tileset);
    for (const auto& chest : m_chests) {
        chest.render(target);
    }
    // OOP CONCEPT: RUNTIME POLYMORPHISM (DYNAMIC DISPATCH)
    // Why: Renders all active interactables (Doors, Traps, KeyPickups) polymorphically
    // through the Interactable base pointer.
    for (const auto& interactable : m_interactables) {
        if (interactable && interactable->isActive()) {
            interactable->render(target);
        }
    }
}

void Room::addDoor(const DoorInfo& door) {
    m_doors.push_back(door);
}

void Room::addChest(const TreasureChest& chest) {
    m_chests.push_back(chest);
}

void Room::addInteractable(std::shared_ptr<Interactable> interactable) {
    if (interactable) {
        m_interactables.push_back(std::move(interactable));
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & ENCAPSULATION
// Why: updateInteractables() dispatches frame updates and collision
// queries polymorphically to each owned Interactable.
// ============================================================
void Room::updateInteractables(float deltaTime, Player& player, SoundManager* soundMgr) {
    for (auto& interactable : m_interactables) {
        if (interactable && interactable->isActive()) {
            interactable->update(deltaTime, player, soundMgr);
        }
    }
}

