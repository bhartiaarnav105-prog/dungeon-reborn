#include "entities/Archer.hpp"
#include "items/LootDrop.hpp"
#include "items/Potion.hpp"
#include <cmath>
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (BASE CONSTRUCTOR CHAINING)
// Why: Initializes Archer via Enemy constructor with:
// 35 HP, 110.0f speed, 3.0f scale, 340.0f detection radius, 250.0f attack range.
// Sets preferred kiting distance and reload timers.
// ============================================================
Archer::Archer(const sf::Texture& texture, sf::Vector2f startPosition)
    : Enemy(35, 110.0f, 3.0f, texture, startPosition, 340.0f, 250.0f)
    , m_preferredDistance(210.0f)
    , m_minDistance(120.0f)
    , m_shootCooldown(1.8f)
    , m_shootTimer(0.5f) // Short initial delay before first shot
    , m_arrowDamage(10)
    , m_arrowSpeed(280.0f)
    , m_strafeTimer(0.0f)
    , m_strafeDirection(1) {

    m_attackDamage = m_arrowDamage;
    m_attackCooldown = m_shootCooldown;

    // 0x72 Dungeon Tileset II Masked Archer Coordinates:
    // Idle anim: 4 frames of 16x16 starting at x=368, y=160
    for (int i = 0; i < 4; ++i) {
        m_idleAnimation.addFrame(sf::IntRect(368 + i * 16, 160, 16, 16));
    }

    // Walk / Run anim: 4 frames of 16x16 starting at x=432, y=160
    for (int i = 0; i < 4; ++i) {
        m_walkAnimation.addFrame(sf::IntRect(432 + i * 16, 160, 16, 16));
    }

    m_sprite.setOrigin(8.0f, 8.0f);
    m_sprite.setTextureRect(m_idleAnimation.getCurrentFrameRect());
    // Visual theme: distinctive hooded purple/violet sniper color
    m_sprite.setColor(sf::Color(225, 140, 255));
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: Archer implements specialized KITING / STANDOFF movement:
// 1. If player gets too close (dist < m_minDistance): Archer RETREATS directly
//    away from the player to avoid melee contact.
// 2. If player is at ideal range: Archer strafes laterally to line up bowshots.
// 3. If player is far away: Archer advances carefully into firing range.
// This is completely distinct from Skeleton's direct chase or Goblin's lunge.
// ============================================================
void Archer::updateAI(float deltaTime, const sf::Vector2f& playerPos) {
    if (!m_isAlive) {
        m_aiState = AIState::Dead;
        return;
    }

    float dist = distanceTo(playerPos);
    sf::Vector2f toPlayer = directionTo(playerPos);

    if (dist <= m_detectionRadius) {
        m_isChasing = true;
        m_facingDirection = (toPlayer.x < 0.0f) ? Direction::Left : Direction::Right;

        if (dist < m_minDistance) {
            // TACTIC 1: KITE / RETREAT AWAY FROM MELEE THREAT
            // Back up directly away from player
            m_velocity = sf::Vector2f(-toPlayer.x, -toPlayer.y);
            m_state = CharacterState::Walking;
            m_aiState = AIState::Chase;
        } else if (dist <= m_preferredDistance) {
            // TACTIC 2: HOLD GROUND & STRAFE LATERALLY
            m_strafeTimer += deltaTime;
            if (m_strafeTimer >= 1.5f) {
                m_strafeTimer = 0.0f;
                m_strafeDirection *= -1; // Flip strafe direction
            }

            // Perpendicular vector for lateral strafing
            sf::Vector2f strafeDir(-toPlayer.y * m_strafeDirection, toPlayer.x * m_strafeDirection);
            m_velocity = strafeDir * 0.5f; // Slower tactical repositioning
            m_state = CharacterState::Walking;
            m_aiState = AIState::Attack;
        } else {
            // TACTIC 3: ADVANCE TO REACH MAXIMUM ARROW RANGE
            m_velocity = toPlayer * 0.7f;
            m_state = CharacterState::Walking;
            m_aiState = AIState::Chase;
        }
    } else {
        // Idle state: hold watch
        m_isChasing = false;
        m_aiState = AIState::Idle;
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_state = CharacterState::Idle;
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM (PROJECTILE COMBAT)
// Why: Overrides Enemy::updateCombat(). Instead of dealing direct melee
// damage, Archer fires arrow Projectile entities into the projectile container,
// which travel through space and hit the player.
// ============================================================
void Archer::updateCombat(float deltaTime,
                          Character& target,
                          std::vector<Projectile>& projectiles,
                          const sf::Texture& tileset) {
    if (!m_isAlive || !target.isAlive()) return;

    if (m_shootTimer > 0.0f) {
        m_shootTimer -= deltaTime;
    }

    float dist = distanceTo(target.getPosition());

    // Fire arrow when target is detected and shoot timer expires
    if (dist <= m_attackRange && m_shootTimer <= 0.0f) {
        performAttack();

        sf::Vector2f dir = directionTo(target.getPosition());
        sf::Vector2f spawnPos = m_position + dir * 26.0f;

        // Arrow projectile rect in 0x72 tileset (304, 160, 16, 16)
        sf::IntRect arrowRect(304, 160, 16, 16);

        projectiles.emplace_back(
            spawnPos,
            dir,
            m_arrowSpeed,
            m_arrowDamage,
            2.5f, // 2.5s maximum flight lifetime
            tileset,
            arrowRect
        );

        m_shootTimer = m_shootCooldown;
        std::cout << "[Archer] Loose! Fired arrow at player! (Dmg: " << m_arrowDamage << ")\n";
    }
}

void Archer::performAttack() {
    m_state = CharacterState::Attacking;
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & ENCAPSULATION
// Why: Archer dispenses thematic loot (gold and health potions).
// ============================================================
void Archer::dropLoot(std::vector<LootDrop>& outDrops, const sf::Texture& tileset) {
    if (m_hasDroppedLoot) return;
    m_hasDroppedLoot = true;

    // 25 Gold drop
    outDrops.emplace_back(m_position, 25, tileset);

    // 50% chance to drop a Health Potion
    outDrops.emplace_back(
        sf::Vector2f(m_position.x + 16.0f, m_position.y),
        Potion::createHealthPotion(),
        1,
        tileset
    );

    std::cout << "[Archer] Slain! Dropped 25 gold and supplies.\n";
}

std::string Archer::getCharacterType() const {
    return "Archer";
}

