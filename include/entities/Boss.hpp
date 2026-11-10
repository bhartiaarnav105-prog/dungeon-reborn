#pragma once

#include "entities/Enemy.hpp"

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS (SCOPED ENUM)
// Why: BossState formalizes the complex multi-state lifecycle of the Demon Boss:
// - Idle: Watching from the throne dais.
// - Chase: Relentless heavy stride toward the player.
// - Attack: Telegraphed ground slam or hellfire shockwave.
// - Retreat: Strategic repositioning/backstep after a heavy strike.
// - Enraged: High-speed, high-damage frenzy phase below 50% HP.
// ============================================================
enum class BossState {
    Idle,
    Chase,
    Attack,
    Retreat,
    Enraged
};

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Boss derives from Enemy, specializing behavior into a multi-phase boss encounter
// with massive health, high damage, ground slams, ranged shockwaves, and dynamic enrage.
// ============================================================
class Boss : public Enemy {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Boss phase states, slam timers, ranged attack timers, and enrage thresholds are private.
private:
    BossState m_bossState;
    bool  m_isEnraged;
    float m_slamTimer;
    float m_slamCooldown;
    bool  m_isSlamming;

    // Windup visual cue & telegraphing
    bool  m_isWindingUp;
    float m_windupTimer;
    float m_windupDuration;

    // Strategic retreat timer after heavy attacks
    float m_retreatTimer;
    float m_retreatDuration;

    // Ranged shockwave pattern
    float m_rangedTimer;
    float m_rangedCooldown;
    int   m_rangedDamage;

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Public API for boss state checking and polymorphic execution.
public:
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Initializes Boss with juggernaut stats (250 HP, 75 speed, 3.5 scale, 25 damage).
    Boss(const sf::Texture& texture, sf::Vector2f startPosition);
    virtual ~Boss() override = default;

    // OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
    // Why: Overrides pure virtual functions with phase-based boss mechanics and multiple attack patterns.
    void updateAI(float deltaTime, const sf::Vector2f& playerPos) override;
    void updateCombat(float deltaTime,
                      Character& target,
                      std::vector<Projectile>& projectiles,
                      const sf::Texture& tileset) override;
    void dropLoot(std::vector<LootDrop>& outDrops, const sf::Texture& tileset) override;
    void performAttack() override;
    void render(sf::RenderTarget& target) const override;
    sf::FloatRect getCollisionBounds() const override;
    std::string getCharacterType() const override;

    // OOP CONCEPT: FUNCTION OVERRIDING
    // Why: Overrides takeDamage to detect the 50% HP threshold and trigger the Enrage phase.
    void takeDamage(int amount) override;

    bool isEnraged() const;
    bool isWindingUp() const;
    int  getRangedDamage() const;
    BossState getBossState() const { return m_bossState; }
    std::string getBossStateString() const;
};
