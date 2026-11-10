#include "entities/Skeleton.hpp"
#include "items/LootDrop.hpp"
#include "items/Weapon.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (BASE CONSTRUCTOR CHAINING)
// Why: Chains through Enemy constructor up to Character with Skeleton-specific stats:
// 45 HP, 80 speed, 3.0 scale, 220 detection radius, 40 attack range.
// ============================================================
Skeleton::Skeleton(const sf::Texture& texture, sf::Vector2f startPosition)
    : Enemy(45, 80.0f, 3.0f, texture, startPosition, 220.0f, 40.0f)
    , m_patrolTimer(0.0f)
    , m_patrolDirection(1)
    , m_attackTimer(0.0f) {

    m_attackDamage = 12;
    m_attackCooldown = 1.2f;

    // 0x72 Dungeon Tileset II Skeleton Coordinates:
    // skelet_idle_anim: 4 frames of 16x16 starting at x=368, y=80
    for (int i = 0; i < 4; ++i) {
        m_idleAnimation.addFrame(sf::IntRect(368 + i * 16, 80, 16, 16));
    }

    // skelet_run_anim: 4 frames of 16x16 starting at x=432, y=80
    for (int i = 0; i < 4; ++i) {
        m_walkAnimation.addFrame(sf::IntRect(432 + i * 16, 80, 16, 16));
    }

    m_sprite.setOrigin(8.0f, 8.0f);
    m_sprite.setTextureRect(m_idleAnimation.getCurrentFrameRect());
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
// Why: Skeleton implements simple relentless shambling toward the player when detected,
// or a slow back-and-forth patrol when idle.
// ============================================================
void Skeleton::updateAI(float deltaTime, const sf::Vector2f& playerPos) {
    if (!m_isAlive) {
        m_aiState = AIState::Dead;
        return;
    }

    float dist = distanceTo(playerPos);

    if (dist <= m_detectionRadius) {
        // Player detected -> relentless pursuit
        m_isChasing = true;
        if (dist > m_attackRange) {
            sf::Vector2f dir = directionTo(playerPos);
            m_velocity = dir;
            m_state = CharacterState::Walking;
            m_aiState = AIState::Chase;
            m_facingDirection = (dir.x < 0.0f) ? Direction::Left : Direction::Right;
        } else {
            // In melee strike range: continue pressing forward while slashing
            sf::Vector2f dir = directionTo(playerPos);
            m_velocity = dir * 0.35f;
            m_state = CharacterState::Attacking;
            m_aiState = AIState::Attack;
            m_facingDirection = (dir.x < 0.0f) ? Direction::Left : Direction::Right;
        }
    } else {
        // Idle patrol state
        m_isChasing = false;
        m_aiState = AIState::Idle;
        m_patrolTimer += deltaTime;
        if (m_patrolTimer > 2.0f) {
            m_patrolTimer = 0.0f;
            m_patrolDirection *= -1; // Flip patrol direction
        }

        m_velocity = sf::Vector2f(m_patrolDirection * 0.3f, 0.0f);
        m_state = CharacterState::Walking;
        m_facingDirection = (m_patrolDirection < 0) ? Direction::Left : Direction::Right;
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM (COMBAT IMPLEMENTATION)
// Why: Overrides pure virtual updateCombat() from Enemy. Skeleton executes
// close-quarters claw attacks with an attack cooldown timer.
// ============================================================
void Skeleton::updateCombat(float deltaTime,
                            Character& target,
                            std::vector<Projectile>& /*projectiles*/,
                            const sf::Texture& /*tileset*/) {
    if (!m_isAlive || !target.isAlive()) return;

    if (m_attackTimer > 0.0f) {
        m_attackTimer -= deltaTime;
    }

    // Check if within strike range or overlapping player footprint
    float dist = distanceTo(target.getPosition());
    sf::FloatRect clawHitbox(m_position.x - 26.0f, m_position.y - 26.0f, 52.0f, 52.0f);
    if ((dist <= 50.0f || clawHitbox.intersects(target.getCollisionBounds())) && m_attackTimer <= 0.0f) {
        performAttack();
        target.takeDamage(m_attackDamage);
        m_attackTimer = m_attackCooldown;
        std::cout << "[Skeleton] Swings skeletal claws! Dealt " << m_attackDamage << " damage to player!\n";
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM (LOOT DISPATCH)
// Why: Overrides pure virtual dropLoot() to generate skeleton-themed loot (Gold and Rusty Dagger).
// ============================================================
void Skeleton::dropLoot(std::vector<LootDrop>& outDrops, const sf::Texture& tileset) {
    if (m_hasDroppedLoot) return;
    m_hasDroppedLoot = true;

    // Drop 15 gold coins
    outDrops.emplace_back(m_position, 15, tileset);
    // Drop a Rusty Dagger
    outDrops.emplace_back(
        sf::Vector2f(m_position.x + 16.0f, m_position.y),
        Weapon::createRustyDagger(),
        1,
        tileset
    );
    std::cout << "[Loot] Skeleton dropped 15 Gold and a Rusty Dagger!\n";
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING
// Why: Overrides Character::performAttack() with Skeleton's bone-rattle melee strike.
// ============================================================
void Skeleton::performAttack() {
    std::cout << "[Skeleton] Swings skeletal claws for " << m_attackDamage << " damage!" << std::endl;
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: Returns concrete class name for dynamic type verification.
// ============================================================
std::string Skeleton::getCharacterType() const {
    return "Skeleton";
}
