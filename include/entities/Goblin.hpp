#pragma once

#include "entities/Enemy.hpp"

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Goblin derives from Enemy, reusing common navigation and health attributes
// while introducing specialized high-speed hit-and-run skirmishing logic.
// ============================================================
class Goblin : public Enemy {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Goblin-specific retreat timers, tactic states, and cooldowns are encapsulated.
private:
    enum class CombatTactic {
        Lunge,
        Retreat,
        Circle,
        Flank
    };

    CombatTactic m_tactic;
    float m_tacticTimer;
    float m_circleAngle;
    float m_attackTimer;
    float m_flankAngle;

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Public interface for instantiation and polymorphic invocation.
public:
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Initializes Goblin with fast, fragile stats (30 HP, 150 speed, 3.0 scale, 8 damage).
    Goblin(const sf::Texture& texture, sf::Vector2f startPosition);
    virtual ~Goblin() override = default;

    // OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
    // Why: Overrides pure virtual functions with distinct goblin agility and skirmish combat.
    void updateAI(float deltaTime, const sf::Vector2f& playerPos) override;
    void updateCombat(float deltaTime,
                      Character& target,
                      std::vector<Projectile>& projectiles,
                      const sf::Texture& tileset) override;
    void dropLoot(std::vector<LootDrop>& outDrops, const sf::Texture& tileset) override;
    void performAttack() override;
    std::string getCharacterType() const override;
};
