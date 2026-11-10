#pragma once

#include "entities/Enemy.hpp"

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Skeleton inherits from Enemy, which inherits from Character.
// This 3-level hierarchy demonstrates multilevel inheritance and reuse of
// health, movement, and AI foundation while providing custom skeleton behavior.
// ============================================================
class Skeleton : public Enemy {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Patrol timers and attack timers are strictly private to Skeleton.
private:
    float m_patrolTimer;
    int   m_patrolDirection;
    float m_attackTimer;

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Clean public interface for instantiation and polymorphic execution.
public:
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Initializes Skeleton with sensible low-tier stats (45 HP, 80 speed, 3.0 scale, 12 damage).
    Skeleton(const sf::Texture& texture, sf::Vector2f startPosition);
    virtual ~Skeleton() override = default;

    // OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
    // Why: Overrides pure virtual AI and combat functions from Enemy/Character.
    void updateAI(float deltaTime, const sf::Vector2f& playerPos) override;
    void updateCombat(float deltaTime,
                      Character& target,
                      std::vector<Projectile>& projectiles,
                      const sf::Texture& tileset) override;
    void dropLoot(std::vector<LootDrop>& outDrops, const sf::Texture& tileset) override;
    void performAttack() override;
    std::string getCharacterType() const override;
};
