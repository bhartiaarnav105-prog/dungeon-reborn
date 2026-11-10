#pragma once

#include "items/Weapon.hpp"

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: MagicWeapon IS-A Weapon (which IS-AN Item).
// Specializes weapon behavior for arcane spellcasting combat.
//
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: Overrides executeAttack() to cast an arcane energy bolt
// with distinct speed, lifetime, and visual characteristics.
// ============================================================
class MagicWeapon : public Weapon {
private:
    float m_spellSpeed;
    float m_manaEfficiency;

public:
    MagicWeapon(const std::string& id,
                const std::string& name,
                const std::string& description,
                sf::IntRect iconRect,
                int damage = 24,
                float cooldown = 0.65f,
                float spellSpeed = 260.0f,
                int value = 60);

    ~MagicWeapon() override = default;

    void executeAttack(Player& player,
                       std::vector<Projectile>& projectiles,
                       const sf::Texture& tileset) override;

    std::unique_ptr<Item> clone() const override;

    std::string getWeaponCategory() const override { return "MagicWeapon"; }
};

