#pragma once

#include "dungeon/Room.hpp"
#include "dungeon/DungeonGenerator.hpp"
#include "dungeon/CameraController.hpp"
#include "dungeon/CollisionSystem.hpp"
#include "dungeon/Minimap.hpp"
#include "interactables/Interactable.hpp"
#include "interactables/Door.hpp"
#include "entities/Player.hpp"
#include "entities/Enemy.hpp"
#include "entities/Projectile.hpp"
#include "items/LootDrop.hpp"
#include "items/TreasureChest.hpp"
#include "core/SoundManager.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: Dungeon is the top-level container for all dungeon state.
// It coordinates rooms, the camera, collision, real-time combat,
// and world pickups (chests & dropped loot).
//
// OOP CONCEPT: COMPOSITION
// Why: Dungeon has-a vector of Rooms (m_rooms), CameraController (m_camera),
// Projectiles (m_projectiles), LootDrops (m_lootDrops), and TreasureChests (m_chests).
// All sub-objects exist within and are owned by Dungeon.
// ============================================================
class Dungeon {
private:
    std::vector<Room>          m_rooms;
    int                        m_currentRoomId;
    CameraController           m_camera;
    unsigned int               m_viewWidth;
    unsigned int               m_viewHeight;
    sf::RectangleShape         m_minimapBg;

    // ============================================================
    // OOP CONCEPT: COMPOSITION
    // Why: Dungeon HAS-A Minimap. Minimap's lifecycle is tied to Dungeon.
    // ============================================================
    Minimap                    m_minimap;

    // Active world entities
    std::vector<Projectile>    m_projectiles;
    std::vector<LootDrop>      m_lootDrops;
    std::vector<TreasureChest> m_chests;
    float                      m_trapCooldown;

    // Internal helpers
    void buildRooms(const sf::Texture* tileset = nullptr);
    void updateCombat(float deltaTime,
                      Player& player,
                      std::vector<std::unique_ptr<Enemy>>& enemies,
                      const sf::Texture& tileset,
                      SoundManager* soundMgr);
    void updateLootAndChests(float deltaTime,
                             Player& player,
                             const sf::Texture& tileset,
                             SoundManager* soundMgr);
public:
    // ============================================================
    // OOP CONCEPT: FACTORY PATTERN & POLYMORPHISM
    // Why: respawnEnemies populates polymorphic Enemy instances for a given room.
    // ============================================================
    void respawnEnemies(int roomId,
                        std::vector<std::unique_ptr<Enemy>>& enemies,
                        const sf::Texture& tileset);
    Dungeon(unsigned int viewWidth, unsigned int viewHeight, const sf::Texture* tileset = nullptr);
    ~Dungeon() = default;

    Dungeon(const Dungeon&)            = delete;
    Dungeon& operator=(const Dungeon&) = delete;

    // OOP CONCEPT: COMPOSITION & ENCAPSULATION
    Minimap&       getMinimap()       { return m_minimap; }
    const Minimap& getMinimap() const { return m_minimap; }
    void renderMinimap(sf::RenderTarget& target, sf::Vector2f playerPos = sf::Vector2f(0.0f, 0.0f)) const;

    void update(float                                deltaTime,
                Player&                              player,
                std::vector<std::unique_ptr<Enemy>>& enemies,
                const sf::Texture&                   tileset,
                SoundManager*                        soundMgr = nullptr);

    void render(sf::RenderTarget& target, const sf::Texture& tileset) const;

    void applyCamera(sf::RenderWindow& window) const;

    // Returns contextual interaction prompt from active room interactables near player
    std::string getActiveInteractionPrompt(const Player& player) const;

    // Const queries
    int          getCurrentRoomId() const { return m_currentRoomId; }
    Room&        getCurrentRoom();
    const Room&  getCurrentRoom() const;
    int          getRoomCount()   const { return static_cast<int>(m_rooms.size()); }
    const Room*  getRoomById(int id) const;

    // ============================================================
    // OOP CONCEPT: ENCAPSULATION & ABSTRACTION
    // Why: isWalkable() exposes a controlled query into the active room's
    // layout. Callers (e.g. Player or AI) query this without accessing
    // internal tile grids or room collections directly.
    // ============================================================
    bool isWalkable(sf::Vector2f worldPos) const;
    bool isWalkable(int col, int row) const;
    bool isAreaWalkable(const sf::FloatRect& bounds) const;
    const Tile& getTileAt(int col, int row) const;

    // Dungeon generation
    void generate(const sf::Texture* tileset = nullptr);

    const std::vector<Projectile>&    getProjectiles() const { return m_projectiles; }
    const std::vector<LootDrop>&      getLootDrops()   const { return m_lootDrops; }
    const std::vector<TreasureChest>& getChests()      const { return m_chests; }

    void addLootDrop(const LootDrop& drop) { m_lootDrops.push_back(drop); }
    void clearProjectiles() { m_projectiles.clear(); }
    void clearLoot() { m_lootDrops.clear(); }
};
