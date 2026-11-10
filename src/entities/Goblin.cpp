#include "entities/Goblin.hpp"
#include "items/LootDrop.hpp"
#include "items/Potion.hpp"
#include <cmath>
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (BASE CONSTRUCTOR CHAINING)
// Why: Initializes Goblin with 30 HP, 150 speed (fast!), 3.0 scale, 280 detection radius, 35 attack range.
// ============================================================
Goblin::Goblin(const sf::Texture& texture, sf::Vector2f startPosition)
    : Enemy(30, 150.0f, 3.0f, texture, startPosition, 280.0f, 35.0f)
    , m_tactic(CombatTactic::Lunge)
    , m_tacticTimer(0.0f)
    , m_circleAngle(0.0f)
    , m_attackTimer(0.0f) {

    m_attackDamage = 8;
    m_attackCooldown = 0.8f;

    // 0x72 Dungeon Tileset II Goblin Coordinates:
    // goblin_idle_anim: 4 frames of 16x16 starting at x=368, y=32
    for (int i = 0; i < 4; ++i) {
        m_idleAnimation.addFrame(sf::IntRect(368 + i * 16, 32, 16, 16));
    }

    // goblin_run_anim: 4 frames of 16x16 starting at x=432, y=32
    for (int i = 0; i < 4; ++i) {
        m_walkAnimation.addFrame(sf::IntRect(432 + i * 16, 32, 16, 16));
    }

    m_sprite.setOrigin(8.0f, 8.0f);
    m_sprite.setTextureRect(m_idleAnimation.getCurrentFrameRect());
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
// Why: Goblin implements agile, erratic hit-and-run tactics:
// 1. Low-health retreat: when HP <= 12, panic-flees away from the player.
// 2. Flanking approach: approaches player from a perpendicular/diagonal direction.
// 3. Lunging stabs and rapid retreat-and-circle cycles.
// ============================================================
void Goblin::updateAI(float deltaTime, const sf::Vector2f& playerPos) {
    if (!m_isAlive) {
        m_aiState = AIState::Dead;
        return;
    }

    float dist = distanceTo(playerPos);

    if (dist <= m_detectionRadius) {
        m_isChasing = true;
        m_tacticTimer += deltaTime;

        sf::Vector2f toPlayer = directionTo(playerPos);

        // Low-health panic retreat: if goblin is heavily wounded (HP <= 12), flee!
        // OOP CONCEPT: ENCAPSULATION
        // Why: Accesses health via public inspector getHealth() preserving base encapsulation.
        if (getHealth() <= 12) {
            m_velocity = sf::Vector2f(-toPlayer.x, -toPlayer.y);
            m_state = CharacterState::Walking;
            m_aiState = AIState::Chase;
            m_facingDirection = (m_velocity.x < 0.0f) ? Direction::Left : Direction::Right;
            return;
        }

        switch (m_tactic) {
            case CombatTactic::Flank: {
                // Approach player from an angled flank (45 degrees offset)
                sf::Vector2f flankDir(toPlayer.x - toPlayer.y * 0.7f, toPlayer.y + toPlayer.x * 0.7f);
                float flen = std::sqrt(flankDir.x * flankDir.x + flankDir.y * flankDir.y);
                if (flen > 0.001f) flankDir /= flen;

                m_velocity = flankDir;
                m_state = CharacterState::Walking;
                m_aiState = AIState::Chase;
                m_facingDirection = (flankDir.x < 0.0f) ? Direction::Left : Direction::Right;

                if (m_tacticTimer > 1.0f || dist <= m_attackRange * 1.5f) {
                    m_tactic = CombatTactic::Lunge;
                    m_tacticTimer = 0.0f;
                }
                break;
            }

            case CombatTactic::Lunge: {
                // Sprint straight toward the player
                m_velocity = toPlayer;
                m_state = CharacterState::Walking;
                m_aiState = AIState::Chase;
                m_facingDirection = (toPlayer.x < 0.0f) ? Direction::Left : Direction::Right;

                if (dist <= m_attackRange) {
                    m_state = CharacterState::Attacking;
                    m_aiState = AIState::Attack;
                }
                break;
            }

            case CombatTactic::Retreat: {
                // Dash away from the player for 0.8 seconds
                m_velocity = sf::Vector2f(-toPlayer.x, -toPlayer.y);
                m_state = CharacterState::Walking;
                m_aiState = AIState::Chase;
                m_facingDirection = (m_velocity.x < 0.0f) ? Direction::Left : Direction::Right;

                if (m_tacticTimer > 0.8f || dist > 180.0f) {
                    m_tactic = CombatTactic::Flank; // Switch to flanking approach
                    m_tacticTimer = 0.0f;
                }
                break;
            }

            case CombatTactic::Circle: {
                // Orbit the player tangentially
                m_circleAngle += deltaTime * 3.0f;
                sf::Vector2f tangent(-toPlayer.y, toPlayer.x);
                m_velocity = tangent;
                m_state = CharacterState::Walking;
                m_aiState = AIState::Chase;
                m_facingDirection = (m_velocity.x < 0.0f) ? Direction::Left : Direction::Right;

                if (m_tacticTimer > 1.2f) {
                    m_tactic = CombatTactic::Flank; // Re-engage from flank!
                    m_tacticTimer = 0.0f;
                }
                break;
            }
        }
    } else {
        // Unaware: subtle jitter/idle
        m_isChasing = false;
        m_aiState = AIState::Idle;
        m_velocity = sf::Vector2f(0.0f, 0.0f);
        m_state = CharacterState::Idle;
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM (COMBAT IMPLEMENTATION)
// Why: Overrides pure virtual updateCombat() from Enemy. Goblin executes
// swift dagger stabs and immediately initiates retreating tactics.
// ============================================================
void Goblin::updateCombat(float deltaTime,
                          Character& target,
                          std::vector<Projectile>& /*projectiles*/,
                          const sf::Texture& /*tileset*/) {
    if (!m_isAlive || !target.isAlive()) return;

    if (m_attackTimer > 0.0f) {
        m_attackTimer -= deltaTime;
    }

    float dist = distanceTo(target.getPosition());
    sf::FloatRect daggerHitbox(m_position.x - 24.0f, m_position.y - 24.0f, 48.0f, 48.0f);
    if ((dist <= 48.0f || daggerHitbox.intersects(target.getCollisionBounds()))
        && m_tactic != CombatTactic::Retreat
        && m_attackTimer <= 0.0f) {
        performAttack();
        target.takeDamage(m_attackDamage);
        m_attackTimer = m_attackCooldown;

        // Immediately trigger hit-and-run retreat tactic
        m_tactic = CombatTactic::Retreat;
        m_tacticTimer = 0.0f;
        std::cout << "[Goblin] Stabs with dagger! Dealt " << m_attackDamage << " damage to player!\n";
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM (LOOT DISPATCH)
// Why: Overrides pure virtual dropLoot() to generate goblin loot (Gold and Health Potion).
// ============================================================
void Goblin::dropLoot(std::vector<LootDrop>& outDrops, const sf::Texture& tileset) {
    if (m_hasDroppedLoot) return;
    m_hasDroppedLoot = true;

    outDrops.emplace_back(m_position, 25, tileset);
    outDrops.emplace_back(
        sf::Vector2f(m_position.x + 16.0f, m_position.y),
        Potion::createHealthPotion(),
        1,
        tileset
    );
    std::cout << "[Loot] Goblin dropped 25 Gold and a Health Potion!\n";
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING
// Why: Overrides Character::performAttack() with Goblin's dagger jab.
// ============================================================
void Goblin::performAttack() {
    std::cout << "[Goblin] Stabs with dagger for " << m_attackDamage << " damage!" << std::endl;
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: Identifies concrete type during polymorphic execution.
// ============================================================
std::string Goblin::getCharacterType() const {
    return "Goblin";
}
