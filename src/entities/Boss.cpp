#include "entities/Boss.hpp"
#include "items/LootDrop.hpp"
#include "items/Weapon.hpp"
#include "items/Potion.hpp"
#include <iostream>
#include <cmath>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (BASE CONSTRUCTOR CHAINING)
// Why: Initializes Boss with 250 HP, 75 base speed, 3.5 scale, 350 detection radius, 65 attack range.
// Initializes attack pattern timers and telegraph duration.
// ============================================================
Boss::Boss(const sf::Texture& texture, sf::Vector2f startPosition)
    : Enemy(250, 75.0f, 3.5f, texture, startPosition, 350.0f, 65.0f)
    , m_bossState(BossState::Idle)
    , m_isEnraged(false)
    , m_slamTimer(2.5f)
    , m_slamCooldown(2.5f)
    , m_isSlamming(false)
    , m_isWindingUp(false)
    , m_windupTimer(0.0f)
    , m_windupDuration(0.55f)
    , m_retreatTimer(0.0f)
    , m_retreatDuration(1.0f)
    , m_rangedTimer(3.0f)
    , m_rangedCooldown(3.0f)
    , m_rangedDamage(15) {

    m_attackDamage = 25;

    // 0x72 Dungeon Tileset II Big Demon Coordinates:
    // big_demon_idle_anim: 4 frames of 32x36 starting at x=16, y=364
    for (int i = 0; i < 4; ++i) {
        m_idleAnimation.addFrame(sf::IntRect(16 + i * 32, 364, 32, 36));
    }

    // big_demon_run_anim: 4 frames of 32x36 starting at x=144, y=364
    for (int i = 0; i < 4; ++i) {
        m_walkAnimation.addFrame(sf::IntRect(144 + i * 32, 364, 32, 36));
    }

    m_sprite.setOrigin(16.0f, 18.0f);
    m_sprite.setTextureRect(m_idleAnimation.getCurrentFrameRect());
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
// Why: Boss AI behavior operates on an explicit state machine:
// - Idle: Waiting for challenger.
// - Chase: Moving aggressively toward the player.
// - Attack: Telegraphed windup and shockwave ground slam.
// - Retreat: Strategic spacing after a slam to force player repositioning.
// - Enraged: High-speed frenzy state below 50% HP.
// ============================================================
void Boss::updateAI(float deltaTime, const sf::Vector2f& playerPos) {
    if (!m_isAlive) {
        m_aiState = AIState::Dead;
        return;
    }

    // Check Enraged state
    if (m_isEnraged && !m_isWindingUp && m_retreatTimer <= 0.0f) {
        m_bossState = BossState::Enraged;
    }

    // 1. Attack Windup State
    if (m_isWindingUp) {
        m_bossState = BossState::Attack;
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_state = CharacterState::Attacking;
        m_aiState = AIState::Windup;
        return;
    }

    float dist = distanceTo(playerPos);
    sf::Vector2f toPlayer = directionTo(playerPos);

    // 2. Strategic Retreat State (post-slam spacing)
    if (m_retreatTimer > 0.0f) {
        m_retreatTimer -= deltaTime;
        m_bossState = BossState::Retreat;
        m_velocity = sf::Vector2f(-toPlayer.x * 0.6f, -toPlayer.y * 0.6f);
        m_state = CharacterState::Walking;
        m_aiState = AIState::Chase;
        m_facingDirection = (toPlayer.x < 0.0f) ? Direction::Left : Direction::Right;
        return;
    }

    // 3. Chase / Attack / Idle States
    if (dist <= m_detectionRadius) {
        m_isChasing = true;
        m_facingDirection = (toPlayer.x < 0.0f) ? Direction::Left : Direction::Right;

        if (dist > m_attackRange) {
            m_velocity = toPlayer;
            m_state = CharacterState::Walking;
            m_aiState = AIState::Chase;
            m_bossState = m_isEnraged ? BossState::Enraged : BossState::Chase;
        } else {
            // In heavy melee slam range
            m_velocity = sf::Vector2f(0.0f, 0.0f);
            m_state = CharacterState::Attacking;
            m_aiState = AIState::Attack;
            m_bossState = BossState::Attack;
        }
    } else {
        m_isChasing = false;
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_state = CharacterState::Idle;
        m_aiState = AIState::Idle;
        m_bossState = BossState::Idle;
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM (MULTI-PATTERN COMBAT)
// Why: Overrides pure virtual updateCombat() from Enemy.
// Executes Pattern 1 (Telegraphed Ground Slam) and Pattern 2 (Ranged Shockwave Projectile).
// ============================================================
void Boss::updateCombat(float deltaTime,
                        Character& target,
                        std::vector<Projectile>& projectiles,
                        const sf::Texture& tileset) {
    if (!m_isAlive || !target.isAlive()) {
        m_isWindingUp = false;
        return;
    }

    m_slamTimer += deltaTime;
    m_rangedTimer += deltaTime;

    float dist = distanceTo(target.getPosition());

    // -------------------------------------------------------------
    // Attack Pattern 1: Telegraphed Melee Ground Slam
    // -------------------------------------------------------------
    if (m_isWindingUp) {
        m_windupTimer += deltaTime;
        if (m_windupTimer >= m_windupDuration) {
            // Telegraph finished: SLAM!
            m_isWindingUp = false;
            m_windupTimer = 0.0f;
            performAttack();

            // Deal heavy damage if target is still within slam radius
            if (distanceTo(target.getPosition()) <= m_attackRange + 35.0f || getGlobalBounds().intersects(target.getCollisionBounds())) {
                target.takeDamage(m_attackDamage);
                std::cout << "[Boss] SLAMS the ground! Dealt " << m_attackDamage << " damage to player!\n";
            }
            m_slamTimer = 0.0f;
            if (!m_isEnraged) {
                m_retreatTimer = m_retreatDuration;
            }
        }
    } else if (dist <= m_attackRange && m_slamTimer >= m_slamCooldown) {
        // Initiate windup telegraph
        m_isWindingUp = true;
        m_windupTimer = 0.0f;
    }

    // -------------------------------------------------------------
    // Attack Pattern 2: Ranged Fiery Shockwave Projectile
    // -------------------------------------------------------------
    if (!m_isWindingUp && dist > m_attackRange && dist <= 320.0f && m_rangedTimer >= m_rangedCooldown) {
        sf::Vector2f dir = directionTo(target.getPosition());
        sf::Vector2f spawnPos = m_position + dir * 32.0f;
        // Spawn fiery shockwave projectile from boss center
        // In 0x72 tileset: flame/shockwave frame at (64, 16, 16, 16)
        projectiles.emplace_back(
            spawnPos,
            dir,
            230.0f,         // speed
            m_rangedDamage, // damage
            2.0f,           // lifetime
            tileset,
            sf::IntRect(64, 16, 16, 16)
        );
        std::cout << "[Boss] Fires Fiery Shockwave! Dealing " << m_rangedDamage << " potential damage!" << std::endl;
        m_rangedTimer = 0.0f;
    }
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING
// Why: Overrides Character::render() to draw the visual telegraph
// wind-up cue (an expanding glowing circle on the dungeon floor) before slamming.
// ============================================================
void Boss::render(sf::RenderTarget& target) const {
    // If winding up, draw visual telegraph danger zone
    if (m_isWindingUp && m_isAlive) {
        float progress = m_windupTimer / m_windupDuration;
        float radius = (m_attackRange + 25.0f) * progress;

        sf::CircleShape telegraph(radius);
        telegraph.setOrigin(radius, radius);
        telegraph.setPosition(m_position);
        telegraph.setFillColor(sf::Color(255, 180, 50, static_cast<sf::Uint8>(60 + progress * 100)));
        telegraph.setOutlineColor(sf::Color(255, 80, 0, 220));
        telegraph.setOutlineThickness(2.0f);
        target.draw(telegraph);
    }

    // Call base class render to draw body sprite & floating health bar
    Character::render(target);
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM (LOOT DISPATCH)
// Why: Overrides pure virtual dropLoot() to generate epic boss loot:
// 100 Gold coins, the Golden Broadsword, and an Elixir of Vitality!
// ============================================================
void Boss::dropLoot(std::vector<LootDrop>& outDrops, const sf::Texture& tileset) {
    if (m_hasDroppedLoot) return;
    m_hasDroppedLoot = true;

    outDrops.emplace_back(m_position, 100, tileset);
    outDrops.emplace_back(
        sf::Vector2f(m_position.x - 20.0f, m_position.y + 15.0f),
        Weapon::createGoldenBroadsword(),
        1,
        tileset
    );
    outDrops.emplace_back(
        sf::Vector2f(m_position.x + 20.0f, m_position.y + 15.0f),
        Potion::createVitalityElixir(),
        1,
        tileset
    );
    std::cout << "[Loot] Boss dropped 100 Gold, the Golden Broadsword, and an Elixir of Vitality!\n";
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING
// Why: Overrides Character::performAttack() with Demon Boss's seismic ground slam.
// ============================================================
void Boss::performAttack() {
    std::cout << "[Boss] SLAMS the ground! Seismic shockwave deals " << m_attackDamage << " damage!" << std::endl;
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING
// Why: Extends Character::takeDamage() with a custom event hook: when health drops
// below 50%, Boss enters an ENRAGED phase (increased speed, red tint, lower attack cooldown).
// ============================================================
void Boss::takeDamage(int amount) {
    Character::takeDamage(amount); // Call base class damage logic first

    if (m_isAlive && !m_isEnraged && getHealth() <= getMaxHealth() / 2) {
        m_isEnraged = true;
        m_bossState = BossState::Enraged;
        m_retreatTimer = 0.0f; // Break out of retreat immediately on enrage
        m_speed = 125.0f; // Speed boost!
        m_slamCooldown = 1.3f; // Much faster melee slams
        m_windupDuration = 0.35f; // Faster telegraph
        m_rangedCooldown = 1.6f; // Rapid shockwave volleys
        m_attackDamage = 35; // Boosted melee damage
        m_rangedDamage = 22; // Boosted ranged damage
        m_baseColor = sf::Color(255, 100, 100); // Enraged red aura
        m_sprite.setColor(m_baseColor);

        std::cout << "\n>>> [WARNING] DEMON BOSS HAS ENTERED ENRAGED PHASE! <<<\n"
                  << ">>> Speed boosted to 125 px/s! Attack damage boosted to 35! <<<\n" << std::endl;
    }
}

std::string Boss::getCharacterType() const {
    return "DemonBoss";
}

bool Boss::isEnraged() const {
    return m_isEnraged;
}

bool Boss::isWindingUp() const {
    return m_isWindingUp;
}

int Boss::getRangedDamage() const {
    return m_rangedDamage;
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
// Why: Boss overrides getCollisionBounds() with a 48x48 footprint,
// scaled appropriately for the giant demon boss while preventing
// sticking on throne room pillars and corridors.
// ============================================================
sf::FloatRect Boss::getCollisionBounds() const {
    return sf::FloatRect(m_position.x - 24.0f, m_position.y - 10.0f, 48.0f, 48.0f);
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: Returns a human-readable representation of internal state.
// ============================================================
std::string Boss::getBossStateString() const {
    switch (m_bossState) {
        case BossState::Idle:    return "Idle";
        case BossState::Chase:   return "Chase";
        case BossState::Attack:  return "Attack";
        case BossState::Retreat: return "Retreat";
        case BossState::Enraged: return "Enraged";
        default:                 return "Unknown";
    }
}
