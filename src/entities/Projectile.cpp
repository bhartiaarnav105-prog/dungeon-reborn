#include "entities/Projectile.hpp"
#include <cmath>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Initializes all sub-objects and properties cleanly before body execution.
// ============================================================
Projectile::Projectile(sf::Vector2f startPos,
                       sf::Vector2f direction,
                       float speed,
                       int damage,
                       float lifetime,
                       const sf::Texture& texture,
                       sf::IntRect textureRect)
    : m_position(startPos)
    , m_velocity(0.f, 0.f)
    , m_speed(speed)
    , m_damage(damage)
    , m_lifetime(0.f)
    , m_maxLifetime(lifetime)
    , m_isAlive(true)
    , m_sprite(texture, textureRect) {

    // Normalize direction vector
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.0001f) {
        m_velocity = sf::Vector2f(direction.x / length, direction.y / length);
    } else {
        m_velocity = sf::Vector2f(1.f, 0.f);
    }

    m_sprite.setOrigin(textureRect.width / 2.0f, textureRect.height / 2.0f);
    m_sprite.setScale(3.0f, 3.0f);
    m_sprite.setPosition(m_position);
    m_sprite.setColor(sf::Color(255, 120, 50)); // Fiery glow
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: Lifetime expiration and motion integration are encapsulated inside update().
// ============================================================
void Projectile::update(float deltaTime) {
    if (!m_isAlive) return;

    m_lifetime += deltaTime;
    if (m_lifetime >= m_maxLifetime) {
        m_isAlive = false;
        return;
    }

    m_position += m_velocity * m_speed * deltaTime;
    m_sprite.setPosition(m_position);
    m_sprite.rotate(180.0f * deltaTime); // Spinning animation
}

void Projectile::render(sf::RenderTarget& target) const {
    if (m_isAlive) {
        target.draw(m_sprite);
    }
}

sf::FloatRect Projectile::getGlobalBounds() const {
    return m_sprite.getGlobalBounds();
}

bool Projectile::isAlive() const {
    return m_isAlive;
}

void Projectile::destroy() {
    m_isAlive = false;
}

int Projectile::getDamage() const {
    return m_damage;
}

sf::Vector2f Projectile::getPosition() const {
    return m_position;
}

