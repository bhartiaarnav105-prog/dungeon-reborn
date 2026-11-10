#pragma once

#include <SFML/Graphics.hpp>

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: Projectile is a dedicated class encapsulating the state and
// lifecycle of moving projectiles (such as boss ranged shockwaves).
// It has a Single Responsibility: moving through world space,
// checking lifespan, and rendering its visual sprite.
//
// OOP CONCEPT: ENCAPSULATION
// Why: Position, velocity, damage, and lifespan are private.
// External systems cannot illegally tamper with active projectiles.
// ============================================================
class Projectile {
private:
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    float        m_speed;
    int          m_damage;
    float        m_lifetime;
    float        m_maxLifetime;
    bool         m_isAlive;
    sf::Sprite   m_sprite;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Initializes all projectile parameters directly via an initialization list.
    // ============================================================
    Projectile(sf::Vector2f startPos,
               sf::Vector2f direction,
               float speed,
               int damage,
               float lifetime,
               const sf::Texture& texture,
               sf::IntRect textureRect);

    // ============================================================
    // OOP CONCEPT: ABSTRACTION
    // Why: update() handles delta-time physics and life duration
    // expiration without exposing internal timer calculations.
    // ============================================================
    void update(float deltaTime);
    void render(sf::RenderTarget& target) const;

    // Boundary & collision queries
    sf::FloatRect getGlobalBounds() const;
    bool isAlive() const;
    void destroy();

    // ============================================================
    // OOP CONCEPT: CONST-CORRECTNESS
    // Why: Pure state inspection methods are marked const.
    // ============================================================
    int getDamage() const;
    sf::Vector2f getPosition() const;
};