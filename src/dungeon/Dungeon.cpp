#include "dungeon/Dungeon.hpp"
#include "entities/Skeleton.hpp"
#include "entities/Goblin.hpp"
#include "entities/Archer.hpp"
#include "entities/Boss.hpp"
#include "entities/EnemyFactory.hpp"
#include "items/Weapon.hpp"
#include "items/Potion.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

// ============================================================
// Room ASCII layout legend:
//   '@' WallTop  '#' WallMid  '|' WallSide
//   '+' CornerTL '=' CornerTR '[' CornerBL ']' CornerBR
//   '.' Floor    'f' FloorAlt
//   'D' DoorOpen (passable, marks door column/row)
//   'T' Torch    'B' Banner
//   'E' Enemy spawn (rendered as floor)
//   'P' Player spawn (rendered as floor)
//   ' ' Empty
//
// All rooms are 20 cols × 13 rows → 960×624 px (fits 960×640 window)
// ============================================================

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALISER LIST)
// Why: m_camera is initialised with the view dimensions in the
// initialiser list so it is valid before buildRooms() is called.
// ============================================================
Dungeon::Dungeon(unsigned int viewWidth, unsigned int viewHeight, const sf::Texture* tileset)
    : m_currentRoomId(0)
    , m_camera(viewWidth, viewHeight, 5.0f)
    , m_viewWidth(viewWidth)
    , m_viewHeight(viewHeight)
    , m_minimap(sf::Vector2f(static_cast<float>(viewWidth) - 188.0f, 15.0f), sf::Vector2f(176.0f, 106.0f))
    , m_trapCooldown(0.0f)
{
    buildRooms(tileset);

    // Camera starts at room 0 centre
    if (!m_rooms.empty()) {
        const Room& r = getCurrentRoom();
        float cx = r.getTileMap().getPixelWidth()  * 0.5f;
        float cy = r.getTileMap().getPixelHeight() * 0.5f;
        m_camera.setBounds(sf::FloatRect(0.f, 0.f,
                                         r.getTileMap().getPixelWidth(),
                                         r.getTileMap().getPixelHeight()));
        m_camera.resetTo(sf::Vector2f(cx, cy));
    }

    // Minimap background (top-right HUD overlay, drawn in UI coords)
    m_minimapBg.setSize(sf::Vector2f(90.f, 60.f));
    m_minimapBg.setFillColor(sf::Color(0, 0, 0, 160));
    m_minimapBg.setOutlineColor(sf::Color(200, 180, 100));
    m_minimapBg.setOutlineThickness(1.f);

    std::cout << "[Dungeon] Initialised with " << m_rooms.size() << " rooms.\n";
}

// ============================================================
// OOP CONCEPT: COMPOSITION
// Why: buildRooms() constructs Room objects (which contain TileMaps)
// and stores them in m_rooms. The Dungeon thus has-a collection of
// Rooms, each of which has-a TileMap — a composition tree.
//
// OOP CONCEPT: ENCAPSULATION
// Why: The room layouts, door connections, and spawn positions are
// all defined here as private data. The rest of the game never
// needs to know what character '#' means in the grid.
// ============================================================
// ============================================================
// OOP CONCEPT: COMPOSITION & DELEGATION
// Why: buildRooms() delegates map creation to DungeonGenerator.
// Dungeon owns and manages the resulting Room composition tree.
// ============================================================
void Dungeon::buildRooms(const sf::Texture* tileset) {
    generate(tileset);
}

// ============================================================
// OOP CONCEPT: SINGLE RESPONSIBILITY PRINCIPLE (SRP)
// Why: generate() delegates room carving and linking to DungeonGenerator,
// keeping Dungeon focused on state coordination and entity life cycles.
// ============================================================
void Dungeon::generate(const sf::Texture* tileset) {
    const float T = 48.0f; // tile display size
    m_rooms = DungeonGenerator::generateRooms(T, tileset);
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION & ABSTRACTION
// Why: isWalkable() queries the active room's TileMap without
// callers (such as Player or Enemy AI) needing to know which
// room is currently loaded or how internal tile grids are indexed.
// ============================================================
bool Dungeon::isWalkable(sf::Vector2f worldPos) const {
    return getCurrentRoom().isWalkable(worldPos);
}

bool Dungeon::isWalkable(int col, int row) const {
    return getCurrentRoom().isWalkable(col, row);
}

bool Dungeon::isAreaWalkable(const sf::FloatRect& bounds) const {
    return getCurrentRoom().getTileMap().isAreaWalkable(bounds);
}

const Tile& Dungeon::getTileAt(int col, int row) const {
    return getCurrentRoom().getTileMap().getTileAt(col, row);
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & ENCAPSULATION
// Why: update() coordinates tile-based collision for player and
// enemies, trap hazards, room transitions, and combat.
// ============================================================
void Dungeon::update(float                                deltaTime,
                     Player&                              player,
                     std::vector<std::unique_ptr<Enemy>>& enemies,
                     const sf::Texture&                   tileset,
                     SoundManager*                        soundMgr)
{
    Room&  room       = getCurrentRoom();
    const TileMap& map = room.getTileMap();

    // OOP CONCEPT: COMPOSITION
    // Why: Dungeon delegates fog-of-war discovery and minimap tracking to its owned Minimap.
    m_minimap.update(deltaTime, m_currentRoomId);

    // --- Apply tile-based collision for player ---
    // handleInput() already set m_velocity as a normalised direction unit vector.
    // Multiply by speed and deltaTime to get the frame displacement.
    sf::Vector2f playerMove = player.getVelocity() * player.getSpeed() * deltaTime;
    sf::FloatRect playerBounds = player.getCollisionBounds();
    sf::Vector2f safeMove = CollisionSystem::resolveTileCollision(playerBounds, playerMove, map);

    // OOP CONCEPT: RUNTIME POLYMORPHISM & ENCAPSULATION
    // Why: Query blocking interactables (e.g. locked doors) and resolve collision
    // against them so the player cannot physically walk through closed or locked doors.
    static std::vector<sf::FloatRect> s_blockingRects;
    s_blockingRects.clear();
    for (const auto& interactable : room.getInteractables()) {
        if (interactable && interactable->isActive() && interactable->isBlocking()) {
            s_blockingRects.push_back(interactable->getBounds());
        }
    }
    if (!s_blockingRects.empty()) {
        safeMove = CollisionSystem::resolveCollision(playerBounds, safeMove, s_blockingRects);
    }

    // Set position to collision-resolved destination.
    // Zero velocity so Player::update() does not double-apply movement.
    player.setPosition(player.getPosition() + safeMove);
    player.setVelocity(sf::Vector2f(0.f, 0.f));
    player.update(deltaTime);

    // --- Update room interactables polymorphically ---
    // OOP CONCEPT: RUNTIME POLYMORPHISM
    // Why: Dispatches update() to Doors, SpikeTraps, FireTraps, KeyPickups, and Chests.
    room.updateInteractables(deltaTime, player, soundMgr);

    // --- Trap hazard detection ---
    sf::Vector2i pTile = map.worldToTile(player.getPosition());
    if (map.getTileAt(pTile.x, pTile.y).type == TileType::Trap) {
        if (m_trapCooldown <= 0.0f) {
            player.takeDamage(6);
            m_trapCooldown = 1.0f; // 1s hazard cooldown
            if (soundMgr) soundMgr->playSound("hurt");
            std::cout << "[Dungeon] Player stepped on a spike trap! -6 HP\n";
        }
    }
    if (m_trapCooldown > 0.0f) {
        m_trapCooldown -= deltaTime;
    }

    // --- Polymorphic enemy updates ---
    // OOP CONCEPT: RUNTIME POLYMORPHISM
    // Why: Each unique_ptr<Enemy> calls the correct overridden updateWithTarget() and
    // updateAI() through virtual dispatch — Skeleton, Goblin, Boss each behave differently.
    // updateWithTarget() internally applies movement (velocity * speed * dt).
    // We then apply collision correction on the resulting position.
    for (auto& enemy : enemies) {
        if (!enemy->isAlive()) continue;

        // Record position before AI moves the enemy
        sf::Vector2f beforePos = enemy->getPosition();

        // Polymorphic AI: sets velocity, applies movement, updates animation
        enemy->updateWithTarget(deltaTime, player.getPosition());

        // Compute actual displacement the enemy applied
        sf::Vector2f afterPos   = enemy->getPosition();
        sf::Vector2f displacement = afterPos - beforePos;

        // Revert to pre-move position, then apply collision-resolved displacement
        enemy->setPosition(beforePos);
        sf::FloatRect eBounds = enemy->getCollisionBounds();
        sf::Vector2f safeDelta = CollisionSystem::resolveTileCollision(eBounds, displacement, map);
        // OOP CONCEPT: ENCAPSULATION
        // Why: Enemies must respect dungeon boundaries and cannot pass through locked doors.
        if (!s_blockingRects.empty()) {
            safeDelta = CollisionSystem::resolveCollision(eBounds, safeDelta, s_blockingRects);
        }
        enemy->setPosition(beforePos + safeDelta);
    }

    // --- Real-time combat processing ---
    updateCombat(deltaTime, player, enemies, tileset, soundMgr);

    // --- Floor loot drops and chest interactions ---
    updateLootAndChests(deltaTime, player, tileset, soundMgr);

    // --- Check door transitions ---
    sf::FloatRect pBounds = player.getCollisionBounds();
    for (const auto& door : room.getDoors()) {
        if (CollisionSystem::isOverlapping(pBounds, door.triggerRect)) {
            // OOP CONCEPT: RUNTIME POLYMORPHISM & DYNAMIC CAST (RTTI)
            // Why: Check if the room has an associated Door interactable for this doorway.
            // If the door is locked or closed, passage is barred until unlocked.
            bool blockedByDoor = false;
            for (const auto& interactable : room.getInteractables()) {
                if (auto d = std::dynamic_pointer_cast<Door>(interactable)) {
                    if (d->getTargetRoomId() == door.targetRoomId && d->isBlocking()) {
                        blockedByDoor = true;
                        break;
                    }
                }
            }
            if (blockedByDoor) continue;

            // Transition to target room
            int targetId = door.targetRoomId;
            bool found = false;
            for (auto& r : m_rooms) {
                if (r.getId() == targetId) { found = true; break; }
            }
            if (!found) continue;

            m_currentRoomId = targetId;
            player.setPosition(door.spawnOffset);
            m_projectiles.clear(); // Clear lingering projectiles on room swap
            m_lootDrops.clear();   // Clear lingering floor loot on room swap

            // Update camera bounds to new room
            const Room& newRoom = getCurrentRoom();
            m_camera.setBounds(sf::FloatRect(0.f, 0.f,
                                              newRoom.getTileMap().getPixelWidth(),
                                              newRoom.getTileMap().getPixelHeight()));
            m_camera.resetTo(door.spawnOffset);

            // Respawn enemies for the new room
            respawnEnemies(targetId, enemies, tileset);

            std::cout << "[Dungeon] Transitioned to room " << targetId
                      << " \"" << newRoom.getName() << "\"\n";
            break;
        }
    }

    // --- Update camera to follow player ---
    m_camera.update(deltaTime, player.getPosition());
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & ABSTRACTION
// Why: updateCombat() coordinates player strikes, enemy polymorphic attacks,
// and projectile lifetime/collision without having concrete enemy type dependencies.
// ============================================================
void Dungeon::updateCombat(float deltaTime,
                           Player& player,
                           std::vector<std::unique_ptr<Enemy>>& enemies,
                           const sf::Texture& tileset,
                           SoundManager* soundMgr) {
    // 1. Player sword attack vs enemies (single controlled hit per swing)
    if (player.isAlive() && player.canDeliverHit()) {
        sf::FloatRect hitbox = player.getAttackHitbox();
        bool hitAny = false;
        for (auto& enemy : enemies) {
            if (enemy->isAlive() && hitbox.intersects(enemy->getGlobalBounds())) {
                enemy->takeDamage(player.getAttackDamage());
                hitAny = true;
            }
        }
        if (hitAny && soundMgr) {
            soundMgr->playSound("hit");
        }
        player.markAttackHitDelivered();
    }

    // 2. OOP CONCEPT: RUNTIME POLYMORPHISM (DYNAMIC DISPATCH)
    // Why: Each enemy subclass executes its specific combat mechanics:
    // Skeleton -> claw strike; Goblin -> hit-and-run dagger stab; Boss -> ground slam & shockwaves.
    int playerHealthBefore = player.getHealth();
    for (auto& enemy : enemies) {
        if (enemy->isAlive()) {
            enemy->updateCombat(deltaTime, player, m_projectiles, tileset);
        } else if (!enemy->hasDroppedLoot()) {
            // OOP CONCEPT: RUNTIME POLYMORPHISM (LOOT DISPATCH)
            // Why: Invokes each enemy's overridden dropLoot() polymorphically upon defeat.
            enemy->dropLoot(m_lootDrops, tileset);
        }
    }
    if (player.getHealth() < playerHealthBefore && soundMgr) {
        soundMgr->playSound("hurt");
    }

    // 3. Projectiles update & collision resolution (O(1) direct tile checks without vector allocations)
    if (!m_projectiles.empty()) {
        const TileMap& tileMap = getCurrentRoom().getTileMap();
        const float tileSize = tileMap.getTileSize();

        for (auto& proj : m_projectiles) {
            if (!proj.isAlive()) continue;
            proj.update(deltaTime);

            // Player collision
            if (player.isAlive() && (proj.getGlobalBounds().intersects(player.getCollisionBounds()) || proj.getGlobalBounds().intersects(player.getGlobalBounds()))) {
                player.takeDamage(proj.getDamage());
                proj.destroy();
                if (soundMgr) soundMgr->playSound("hurt");
                std::cout << "[Combat] Projectile struck player! - " << proj.getDamage() << " HP (Player HP: " << player.getHealth() << ")\n";
                continue;
            }

            // Solid wall collision: direct O(1) tile lookup
            sf::FloatRect pb = proj.getGlobalBounds();
            sf::FloatRect pInset(pb.left + 4.f, pb.top + 4.f, std::max(2.f, pb.width - 8.f), std::max(2.f, pb.height - 8.f));

            int minCol = static_cast<int>(std::floor(pInset.left / tileSize));
            int maxCol = static_cast<int>(std::floor((pInset.left + pInset.width) / tileSize));
            int minRow = static_cast<int>(std::floor(pInset.top / tileSize));
            int maxRow = static_cast<int>(std::floor((pInset.top + pInset.height) / tileSize));

            bool hitWall = false;
            for (int r = minRow; r <= maxRow && !hitWall; ++r) {
                for (int c = minCol; c <= maxCol && !hitWall; ++c) {
                    if (tileMap.isSolid(c, r)) {
                        hitWall = true;
                    }
                }
            }
            if (hitWall) {
                proj.destroy();
            }
        }

        // Clean up inactive projectiles
        m_projectiles.erase(
            std::remove_if(m_projectiles.begin(), m_projectiles.end(),
                           [](const Projectile& p) { return !p.isAlive(); }),
            m_projectiles.end()
        );
    }
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: updateLootAndChests manages physical pickups and chest interactions
// through clean public interfaces (collect, open) without exposing raw containers.
// ============================================================
void Dungeon::updateLootAndChests(float deltaTime,
                                  Player& player,
                                  const sf::Texture& tileset,
                                  SoundManager* soundMgr) {
    // 1. Update and collect active floor loot drops
    for (auto& drop : m_lootDrops) {
        if (drop.isCollected()) continue;
        drop.update(deltaTime);

        if (player.isAlive() && drop.getGlobalBounds().intersects(player.getGlobalBounds())) {
            bool collected = drop.collect(player);
            if (collected && soundMgr) {
                soundMgr->playSound(drop.isGold() ? "coin" : "potion");
            }
        }
    }

    // Erase collected loot drops
    m_lootDrops.erase(
        std::remove_if(m_lootDrops.begin(), m_lootDrops.end(),
                       [](const LootDrop& d) { return d.isCollected(); }),
        m_lootDrops.end()
    );

    // 2. Chest interaction: player within 65px and presses [E] or walks into it (35px)
    bool ePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
    for (auto& chest : getCurrentRoom().getChests()) {
        if (!chest.isOpen()) {
            float dist = std::hypot(chest.getPosition().x - player.getPosition().x,
                                    chest.getPosition().y - player.getPosition().y);
            if (dist <= 65.0f && (ePressed || dist <= 38.0f)) {
                chest.open(m_lootDrops, tileset, soundMgr);
            }
        }
    }
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION (PRIVATE METHOD)
// Why: Enemy spawning logic is hidden inside Dungeon. Game does
// not need to know which enemy types live in which rooms.
//
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: Concrete enemies (Skeleton, Goblin, Boss) are constructed
// and stored as unique_ptr<Enemy>. Game iterates them via the
// Enemy base interface without knowing their concrete type.
// ============================================================
// ============================================================
// OOP CONCEPT: FACTORY PATTERN & RUNTIME POLYMORPHISM
// Why: Enemy spawning delegates instantiation to EnemyFactory::createEnemy().
// Derived instances (Skeleton, Goblin, Archer, Boss) are stored polymorphically
// as std::unique_ptr<Enemy> in the heterogeneous enemy vector.
// ============================================================
void Dungeon::respawnEnemies(int                                  roomId,
                              std::vector<std::unique_ptr<Enemy>>& enemies,
                              const sf::Texture&                   tileset)
{
    enemies.clear();

    const Room* room = getRoomById(roomId);
    if (!room) return;

    const float T = 48.0f; // Tile dimension

    // ============================================================
    // OOP CONCEPT: FACTORY PATTERN & RUNTIME POLYMORPHISM
    // Why: EnemyFactory::createEnemy() returns an abstract std::unique_ptr<Enemy>.
    // Concrete instances (Skeleton, Goblin, Archer, Boss) are stored in the
    // polymorphic container and updated/rendered via virtual dispatch.
    // ============================================================
    if (roomId == 0) {
        // Room 0 (The Ancient Crossroads, 24x14):
        // 1. Shambling Skeleton patrolling the crossroads
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Skeleton, tileset, sf::Vector2f(9.0f * T, 6.0f * T)));
        // 2. Tactical Archer guarding the upper corridor
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Archer, tileset, sf::Vector2f(16.0f * T, 3.0f * T)));

    } else if (roomId == 1) {
        // Room 1 (The Demon's Sanctum, 26x16):
        // 1. Demon Boss in center of the grand throne dais
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Boss, tileset, sf::Vector2f(12.5f * T, 7.0f * T)));
        // 2. Hooded Archer providing tactical flank fire
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Archer, tileset, sf::Vector2f(5.0f * T, 4.0f * T)));
        // 3. Skeleton Royal Guard
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Skeleton, tileset, sf::Vector2f(20.0f * T, 4.0f * T)));

    } else if (roomId == 2) {
        // Room 2 (The Grand Labyrinth & Key Altar, 28x16):
        // 1. Skeleton patrolling north maze loop
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Skeleton, tileset, sf::Vector2f(6.0f * T, 3.0f * T)));
        // 2. Skeleton patrolling northeast branch
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Skeleton, tileset, sf::Vector2f(20.0f * T, 3.0f * T)));
        // 3. Goblin skirmisher in west T-junction
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Goblin, tileset, sf::Vector2f(7.0f * T, 7.0f * T)));
        // 4. Goblin skirmisher in east corridor
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Goblin, tileset, sf::Vector2f(19.0f * T, 7.0f * T)));
        // 5. Goblin Guard directly defending the Boss Key Altar
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Goblin, tileset, sf::Vector2f(14.0f * T, 13.0f * T)));
        // 6. Tactical Archer guarding the trap shortcut
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Archer, tileset, sf::Vector2f(14.0f * T, 9.0f * T)));

    } else if (roomId == 3) {
        // Room 3 (The Forgotten Armory, 22x14):
        // 1. Skeleton sentry at armory door
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Skeleton, tileset, sf::Vector2f(6.0f * T, 5.0f * T)));
        // 2. Skeleton guarding weapon racks
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Skeleton, tileset, sf::Vector2f(15.0f * T, 5.0f * T)));
        // 3. Goblin looter raiding armory chest
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Goblin, tileset, sf::Vector2f(10.5f * T, 5.0f * T)));

    } else if (roomId == 4) {
        // Room 4 (The Crypt of Shadows, 22x14):
        // 1. Skeleton Crypt Guardian left
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Skeleton, tileset, sf::Vector2f(6.0f * T, 4.0f * T)));
        // 2. Skeleton Crypt Guardian right
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Skeleton, tileset, sf::Vector2f(15.0f * T, 4.0f * T)));
        // 3. Fast Goblin assassin
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Goblin, tileset, sf::Vector2f(10.5f * T, 8.0f * T)));
        // 4. Hooded Archer sniper perched behind crypt columns
        enemies.push_back(EnemyFactory::createEnemy(EnemyType::Archer, tileset, sf::Vector2f(10.5f * T, 3.0f * T)));
    }

    std::cout << "[Dungeon] Spawned " << enemies.size() << " enemies for room " << roomId << " via EnemyFactory\n";
}

void Dungeon::render(sf::RenderTarget& target, const sf::Texture& tileset) const {
    getCurrentRoom().render(target, tileset);

    // Render floor loot drops
    for (const auto& drop : m_lootDrops) {
        drop.render(target);
    }

    // Render active combat projectiles
    for (const auto& p : m_projectiles) {
        p.render(target);
    }
}

// ============================================================
// OOP CONCEPT: COMPOSITION & DELEGATION
// Why: Dungeon delegates minimap rendering to its composed Minimap object,
// passing room references and the player's position.
// ============================================================
void Dungeon::renderMinimap(sf::RenderTarget& target, sf::Vector2f playerPos) const {
    m_minimap.render(target, m_currentRoomId, m_rooms, playerPos);
}

void Dungeon::applyCamera(sf::RenderWindow& window) const {
    m_camera.applyTo(window);
}

Room& Dungeon::getCurrentRoom() {
    for (auto& r : m_rooms) {
        if (r.getId() == m_currentRoomId) return r;
    }
    return m_rooms[0]; // fallback
}

const Room& Dungeon::getCurrentRoom() const {
    for (const auto& r : m_rooms) {
        if (r.getId() == m_currentRoomId) return r;
    }
    return m_rooms[0];
}

const Room* Dungeon::getRoomById(int id) const {
    for (const auto& r : m_rooms) {
        if (r.getId() == id) return &r;
    }
    return nullptr;
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & ENCAPSULATION
// Why: getActiveInteractionPrompt iterates the current room's
// interactable collection, checks proximity, and retrieves prompts
// polymorphically without needing to branch on concrete types.
// ============================================================
std::string Dungeon::getActiveInteractionPrompt(const Player& player) const {
    const Room& room = getCurrentRoom();
    sf::FloatRect pBounds = player.getCollisionBounds();
    sf::FloatRect searchArea(pBounds.left - 25.0f, pBounds.top - 25.0f,
                             pBounds.width + 50.0f, pBounds.height + 50.0f);

    for (const auto& interactable : room.getInteractables()) {
        if (interactable && interactable->isActive()) {
            if (interactable->getBounds().intersects(searchArea)) {
                std::string prompt = interactable->getPrompt();
                if (!prompt.empty()) return prompt;
            }
        }
    }
    return "";
}
