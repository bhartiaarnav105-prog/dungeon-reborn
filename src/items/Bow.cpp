#include "items/Bow.hpp"
#include "entities/Player.hpp"
#include "entities/Projectile.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Initializes Weapon with bow sprite and icon rects.
// Sets arrow velocity.
// ============================================================
Bow::Bow(const std::string& id,
         const std::string& name,
         const std::string& description,
         sf::IntRect iconRect,
         int damage,
         float cooldown,
         float arrowSpeed,
         int value)
    : Weapon(id, name, description, iconRect, damage, cooldown, sf::IntRect(304, 160, 16, 16), value)
    , m_arrowSpeed(arrowSpeed) {
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: Overrides Weapon::executeAttack(). Rather than a melee slash,
// Bow fires a high-velocity arrow Projectile into the active world.
// ============================================================
void Bow::executeAttack(Player& player,
                       std::vector<Projectile>& projectiles,
                       const sf::Texture& tileset) {
    player.performAttack();

    // Determine shot direction from player facing
    sf::Vector2f dir = (player.getFacingDirection() == Direction::Left)
                     ? sf::Vector2f(-1.0f, 0.0f)
                     : sf::Vector2f(1.0f, 0.0f);

    sf::Vector2f spawnPos = player.getPosition() + dir * 22.0f;
    sf::IntRect arrowRect(304, 160, 16, 16);

    projectiles.emplace_back(
        spawnPos,
        dir,
        m_arrowSpeed,
        m_damage,
        3.0f, // 3.0s flight lifespan
        tileset,
        arrowRect
    );

    std::cout << "[Bow] Knight loosed an arrow from " << m_name << "! (" << m_damage << " damage, "
              << m_arrowSpeed << " px/s)\n";
}

std::unique_ptr<Item> Bow::clone() const {
    return std::make_unique<Bow>(*this);
}
