#pragma once

#include "dungeon/TileMap.hpp"
#include "items/TreasureChest.hpp"
#include "interactables/Interactable.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class Player;
class SoundManager;

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS (SCOPED ENUM)
// Why: RoomType formalizes the role and gameplay archetype of each
// dungeon chamber (Entrance, Combat, Treasure, Trap, Boss)
// avoiding arbitrary integers or loose strings.
// ============================================================
enum class RoomType {
    Entrance,
    Combat,
    Treasure,
    Trap,
    Boss
};

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: DoorInfo is a plain data type that bundles all information
// about a single doorway: which side of the room it is on, which
// room it connects to, and the world-space trigger rectangle.
// ============================================================
struct DoorInfo {
    enum class Side { North, South, East, West };

    Side         side;
    int          targetRoomId;
    sf::FloatRect triggerRect; // world-space AABB player must overlap to transition
    sf::Vector2f  spawnOffset; // where the player appears in the target room
};

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: Room encapsulates everything about a single dungeon chamber:
// its tile layout, doors, chests, entity spawn positions, and archetype.
//
// OOP CONCEPT: COMPOSITION
// Why: Room has-a TileMap (m_tileMap) and has-a list of TreasureChests (m_chests).
// When the Room is destroyed, all owned sub-objects are cleanly reclaimed.
//
// OOP CONCEPT: ENCAPSULATION
// Why: All room data is strictly private. The public interface exposes
// only well-named getters, predicates, and safe mutation methods.
// ============================================================
class Room {
private:
    int                        m_id;
    std::string                m_name;
    RoomType                   m_type;
    TileMap                    m_tileMap;         // COMPOSITION: Room has-a TileMap
    std::vector<DoorInfo>      m_doors;
    std::vector<sf::Vector2f>  m_enemySpawnPositions;
    sf::Vector2f               m_playerSpawnPosition;
    std::vector<TreasureChest> m_chests;          // COMPOSITION: Room has-a list of chests
    // ============================================================
    // OOP CONCEPT: COMPOSITION & RUNTIME POLYMORPHISM
    // Why: Room has-a collection of polymorphic Interactables
    // (Doors, Traps, Chests, Keys). Room manages their lifetimes.
    // ============================================================
    std::vector<std::shared_ptr<Interactable>> m_interactables;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Parameterized constructors guarantee invariant initialization
    // with an explicit RoomType and ASCII grid layout.
    // ============================================================
    Room(int id,
         const std::string& name,
         RoomType type,
         const std::vector<std::string>& layout,
         float tileSize);

    Room(int id,
         const std::string& name,
         const std::vector<std::string>& layout,
         float tileSize);

    ~Room() = default;

    // Render the tile map and chests using the shared tileset texture.
    void render(sf::RenderTarget& target, const sf::Texture& tileset) const;

    // Add connections and interactive chests
    void addDoor(const DoorInfo& door);
    void addChest(const TreasureChest& chest);

    // ============================================================
    // OOP CONCEPT: RUNTIME POLYMORPHISM & COMPOSITION
    // Why: Adds any derived Interactable (Door, SpikeTrap, FireTrap, KeyPickup)
    // to the room's managed collection.
    // ============================================================
    void addInteractable(std::shared_ptr<Interactable> interactable);
    void updateInteractables(float deltaTime, Player& player, SoundManager* soundMgr = nullptr);
    const std::vector<std::shared_ptr<Interactable>>& getInteractables() const { return m_interactables; }
    std::vector<std::shared_ptr<Interactable>>&       getInteractables()       { return m_interactables; }

    // ============================================================
    // OOP CONCEPT: ENCAPSULATION & CONST-CORRECTNESS
    // Why: Read-only queries expose room metadata without allowing
    // arbitrary mutation of internal spatial data structures.
    // ============================================================
    int                              getId()                    const { return m_id; }
    const std::string&               getName()                  const { return m_name; }
    RoomType                         getType()                  const { return m_type; }
    void                             setType(RoomType type)           { m_type = type; }
    const TileMap&                   getTileMap()               const { return m_tileMap; }
    const std::vector<DoorInfo>&     getDoors()                 const { return m_doors; }
    sf::Vector2f                     getPlayerSpawnPosition()   const { return m_playerSpawnPosition; }
    const std::vector<sf::Vector2f>& getEnemySpawnPositions()   const { return m_enemySpawnPositions; }
    std::vector<TreasureChest>&      getChests()                      { return m_chests; }
    const std::vector<TreasureChest>& getChests()               const { return m_chests; }

    // Physical bounds and walkability queries
    sf::FloatRect getBounds() const;
    bool isWalkable(sf::Vector2f worldPos) const;
    bool isWalkable(int col, int row) const;

private:
    // ============================================================
    // OOP CONCEPT: ENCAPSULATION
    // Why: Parsing the ASCII grid for spawn-point characters is an
    // internal detail. It is done once in the constructor and stored,
    // so callers never need to touch the raw layout strings.
    // ============================================================
    void parseSpawnPositions(const std::vector<std::string>& layout, float tileSize);
};

