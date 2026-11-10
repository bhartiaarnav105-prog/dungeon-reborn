#pragma once

#include "entities/Enemy.hpp"

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Archer IS-AN Enemy (which IS-A Character).
// Archer specializes Enemy with ranged kiting tactics and projectile combat,
// demonstrating behavioral differentiation without modifying base classes.
//
// OOP CONCEPT: ENCAPSULATION
// Why: Archer's tactical distance thresholds (m_minDistance, m_preferredDistance)
// and arrow reload timers (m_shootTimer) are private to the Archer class.
// ============================================================
class Archer : public Enemy {
private:
    float m_preferredDistance; // Ideal standoff distance from target
    float m_minDistance;       // If player gets closer than this, Archer kites backward!
    float m_shootCooldown;     // Time between ranged arrow volleys
    float m_shootTimer;        // Active reload countdown timer
    int   m_arrowDamage;       // Damage inflicted by arrow projectiles
    float m_arrowSpeed;        // Arrow travel velocity
    float m_strafeTimer;       // Lateral strafe movement timer
    int   m_strafeDirection;   // 1 or -1 for perpendicular strafing

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Initializes Archer with medium health (35 HP), agile speed (110 px/s),
    // and long-range perception (320 detection radius).
    // ============================================================
    Archer(const sf::Texture& texture, sf::Vector2f startPosition);
    ~Archer() override = default;

    // ============================================================
    // OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
    // Why: Overrides pure virtual methods from Enemy:
    // - updateAI(): Kiting / standoff navigation (avoids touching player!).
    // - updateCombat(): Launches arrow Projectiles toward the player.
    // - dropLoot(): Dispenses gold and health potions upon defeat.
    // ============================================================
    void updateAI(float deltaTime, const sf::Vector2f& playerPos) override;

    void updateCombat(float deltaTime,
                      Character& target,
                      std::vector<Projectile>& projectiles,
                      const sf::Texture& tileset) override;

    void dropLoot(std::vector<LootDrop>& outDrops, const sf::Texture& tileset) override;

    void performAttack() override;

    std::string getCharacterType() const override;

    float getPreferredDistance() const { return m_preferredDistance; }
    float getMinDistance() const { return m_minDistance; }
    int   getArrowDamage() const { return m_arrowDamage; }
};

