#pragma once

#include "items/Weapon.hpp"

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Sword IS-A Weapon (which IS-AN Item).
// Specializes weapon behavior for close-quarters melee combat.
//
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: Overrides executeAttack() and getWeaponCategory().
// When player swings, dynamic dispatch calls Sword's melee implementation.
// ============================================================
class Sword : public Weapon {
public:
    Sword(const std::string& id,
          const std::string& name,
          const std::string& description,
          sf::IntRect iconRect,
          int damage,
          float cooldown,
          sf::IntRect weaponSpriteRect,
          int value = 30);

    ~Sword() override = default;

    void executeAttack(Player& player,
                       std::vector<Projectile>& projectiles,
                       const sf::Texture& tileset) override;

    std::unique_ptr<Item> clone() const override;

    std::string getWeaponCategory() const override { return "Sword"; }
};

