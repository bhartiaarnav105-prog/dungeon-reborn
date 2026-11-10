#include "items/Sword.hpp"
#include "entities/Player.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (BASE CONSTRUCTOR INITIALIZER)
// Why: Initializes Weapon base class with sword-specific stats.
// ============================================================
Sword::Sword(const std::string& id,
             const std::string& name,
             const std::string& description,
             sf::IntRect iconRect,
             int damage,
             float cooldown,
             sf::IntRect weaponSpriteRect,
             int value)
    : Weapon(id, name, description, iconRect, damage, cooldown, weaponSpriteRect, value) {
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: Overrides Weapon::executeAttack(). Sword performs a directional
// slashing arc and primes the Player to deliver melee impact.
// ============================================================
void Sword::executeAttack(Player& player,
                         std::vector<Projectile>& /*projectiles*/,
                         const sf::Texture& /*tileset*/) {
    player.performAttack();
    std::cout << "[Sword] Knight swings " << m_name << "! (" << m_damage << " potential damage)\n";
}

std::unique_ptr<Item> Sword::clone() const {
    return std::make_unique<Sword>(*this);
}

