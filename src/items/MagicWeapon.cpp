#include "items/MagicWeapon.hpp"
#include "entities/Player.hpp"
#include "entities/Projectile.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Initializes Weapon with staff icon and visual rects.
// ============================================================
MagicWeapon::MagicWeapon(const std::string& id,
                         const std::string& name,
                         const std::string& description,
                         sf::IntRect iconRect,
                         int damage,
                         float cooldown,
                         float spellSpeed,
                         int value)
    : Weapon(id, name, description, iconRect, damage, cooldown, sf::IntRect(336, 160, 16, 16), value)
    , m_spellSpeed(spellSpeed)
    , m_manaEfficiency(1.0f) {
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: Overrides Weapon::executeAttack(). Casts an arcane fireball/shockwave
// projectile traveling forward.
// ============================================================
void MagicWeapon::executeAttack(Player& player,
                               std::vector<Projectile>& projectiles,
                               const sf::Texture& tileset) {
    player.performAttack();

    // Determine shot direction from player facing
    sf::Vector2f dir = (player.getFacingDirection() == Direction::Left)
                     ? sf::Vector2f(-1.0f, 0.0f)
                     : sf::Vector2f(1.0f, 0.0f);

    sf::Vector2f spawnPos = player.getPosition() + dir * 24.0f;
    // Magic orb / shockwave frame in 0x72 tileset
    sf::IntRect spellRect(64, 16, 16, 16);

    projectiles.emplace_back(
        spawnPos,
        dir,
        m_spellSpeed,
        m_damage,
        2.5f,
        tileset,
        spellRect
    );

    std::cout << "[MagicWeapon] Knight invoked " << m_name << "! Cast arcane bolt (" << m_damage << " magic damage)\n";
}

std::unique_ptr<Item> MagicWeapon::clone() const {
    return std::make_unique<MagicWeapon>(*this);
}
