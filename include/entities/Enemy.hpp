#pragma once

#include "entities/Character.hpp"
#include "entities/Projectile.hpp"
#include <vector>

class LootDrop;

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS (SCOPED ENUM)
// Why: AIState formalizes state-based enemy decision-making
// into distinct phases: Idle, Chase, Windup, Attack, Hurt, and Dead.
// ============================================================
enum class AIState {
    Idle,
    Chase,
    Windup,
    Attack,
    Hurt,
    Dead
};

// ============================================================
// OOP CONCEPT: INHERITANCE AND ABSTRACT CLASSES
// Why: Enemy is an abstract base class deriving from Character, extending the
// character contract with AI perception, detection radii, state tracking, and
// polymorphic combat routines. It cannot be instantiated directly.
// ============================================================
class Enemy : public Character {
// OOP CONCEPT: ACCESS SPECIFIERS (PROTECTED)
// Why: Detection attributes and AI state are protected so derived enemy species
// (Skeleton, Goblin, Boss) can customize patrol ranges, speeds, and behavior.
protected:
    float m_detectionRadius;
    float m_attackRange;
    bool m_isChasing;
    float m_aiTimer;
    AIState m_aiState;
    bool m_hasDroppedLoot;

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Clean public interface for AI target updating and query methods.
public:
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Passes maxHealth, speed, scale, and texture up to Character.
    Enemy(int maxHealth, float speed, float scale, const sf::Texture& texture,
          sf::Vector2f startPosition, float detectionRadius = 250.0f, float attackRange = 40.0f);
    virtual ~Enemy() override = default;

    // OOP CONCEPT: ABSTRACTION AND RUNTIME POLYMORPHISM
    // Why: Pure virtual methods that every concrete enemy type must implement
    // with its own specialized strategy (Skeleton, Goblin, Boss).
    virtual void updateAI(float deltaTime, const sf::Vector2f& playerPos) = 0;

    // OOP CONCEPT: RUNTIME POLYMORPHISM (COMBAT CONTRACT)
    // Why: updateCombat() handles attacking the player (melee strikes, lunges, slams, or projectiles)
    // through dynamic dispatch without the caller needing to know the concrete enemy type.
    virtual void updateCombat(float deltaTime,
                              Character& target,
                              std::vector<Projectile>& projectiles,
                              const sf::Texture& tileset) = 0;

    // OOP CONCEPT: RUNTIME POLYMORPHISM (LOOT DROP CONTRACT)
    // Why: Each derived enemy species dispenses appropriate thematic loot upon defeat.
    virtual void dropLoot(std::vector<LootDrop>& outDrops, const sf::Texture& tileset) = 0;
    bool hasDroppedLoot() const { return m_hasDroppedLoot; }
    void markLootDropped() { m_hasDroppedLoot = true; }

    // OOP CONCEPT: FUNCTION OVERRIDING
    // Why: Default Character::update(dt) implementation for enemies.
    void update(float deltaTime) override;
    sf::FloatRect getCollisionBounds() const override;

    // Update with player's position target
    void updateWithTarget(float deltaTime, const sf::Vector2f& playerPos);

    // AI Math utility helpers
    float distanceTo(const sf::Vector2f& target) const;
    sf::Vector2f directionTo(const sf::Vector2f& target) const;

    float getDetectionRadius() const;
    float getAttackRange() const;
    bool isChasing() const;
    AIState getAIState() const;
    void setAIState(AIState state);
};
