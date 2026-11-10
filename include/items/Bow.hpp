#pragma once

#include "items/Weapon.hpp"

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Bow IS-A Weapon (which IS-AN Item).
// Specializes weapon behavior for ranged projectile combat.
//
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: Overrides executeAttack() to spawn an arrow Projectile
// traveling in the Player's facing direction.
// ============================================================
class Bow : public Weapon {
private:
    float m_arrowSpeed;

public:
    Bow(const std::string& id,
        const std::string& name,
        const std::string& description,
        sf::IntRect iconRect,
        int damage = 18,
        float cooldown = 0.5f,
        float arrowSpeed = 340.0f,
        int value = 40);

    ~Bow() override = default;

    void executeAttack(Player& player,
                       std::vector<Projectile>& projectiles,
                       const sf::Texture& tileset) override;

    std::unique_ptr<Item> clone() const override;

    std::string getWeaponCategory() const override { return "Bow"; }

    float getArrowSpeed() const { return m_arrowSpeed; }
};

