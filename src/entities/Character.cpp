#include "entities/Character.hpp"
#include <algorithm>
#include <iostream>

// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Directly initializes base attributes before any derived constructor body runs.
Character::Character(int maxHealth, float speed, float scale, const sf::Texture& texture, sf::Vector2f startPosition)
    : m_health(maxHealth)
    , m_maxHealth(maxHealth)
    , m_position(startPosition)
    , m_velocity(0.0f, 0.0f)
    , m_speed(speed)
    , m_scale(scale)
    , m_facingDirection(Direction::Right)
    , m_state(CharacterState::Idle)
    , m_isAlive(true)
    , m_attackDamage(10)
    , m_attackCooldown(1.0f)
    , m_attackTimer(0.0f)
    , m_hurtTimer(0.0f)
    , m_baseColor(sf::Color::White)
    , m_sprite(texture)
    , m_idleAnimation(0.18f, true)
    , m_walkAnimation(0.12f, true) {

    m_sprite.setPosition(m_position);
    m_sprite.setScale(m_scale, m_scale);
}

// OOP CONCEPT: ENCAPSULATION
// Why: Flips the sprite horizontally around its center based on facing direction.
void Character::updateSpriteTransform() {
    if (m_facingDirection == Direction::Left) {
        m_sprite.setScale(-m_scale, m_scale);
    } else {
        m_sprite.setScale(m_scale, m_scale);
    }
}

// OOP CONCEPT: FUNCTION OVERRIDING AND VIRTUAL FUNCTIONS
// Why: Base rendering implementation draws the sprite and floating health bar.
// Derived classes can override or extend this behavior.
void Character::render(sf::RenderTarget& target) const {
    if (!m_isAlive && m_state == CharacterState::Dead) {
        // Can render faded corpse or skip
        return;
    }
    target.draw(m_sprite);
    renderHealthBar(target);
}

// OOP CONCEPT: ENCAPSULATION
// Why: Health modifications enforce invariants: health never drops below 0 or exceeds maxHealth.
// When health reaches 0, the character is transitioned into the Dead state automatically.
// Visual damage feedback (red tint) is triggered upon taking damage.
void Character::takeDamage(int amount) {
    if (!m_isAlive || amount <= 0) {
        return;
    }

    m_health -= amount;
    if (m_health <= 0) {
        m_health = 0;
        m_isAlive = false;
        m_state = CharacterState::Dead;
    } else {
        // Visual hurt feedback
        m_hurtTimer = 0.15f;
        m_sprite.setColor(sf::Color(255, 60, 60)); // Flash red
    }

    std::cout << "[" << getCharacterType() << "] Took " << amount << " damage. Health: "
              << m_health << "/" << m_maxHealth << std::endl;

    if (!m_isAlive) {
        std::cout << "[" << getCharacterType() << "] Has perished!" << std::endl;
    }
}

// Damage feedback update: resets hurt flash when timer expires
void Character::updateDamageFeedback(float deltaTime) {
    if (m_hurtTimer > 0.0f) {
        m_hurtTimer -= deltaTime;
        if (m_hurtTimer <= 0.0f) {
            m_hurtTimer = 0.0f;
            m_sprite.setColor(m_baseColor);
        }
    }
}

// OOP CONCEPT: ABSTRACTION
// Why: Default attack hitbox extends directly in front of the character according to facing direction.
sf::FloatRect Character::getAttackHitbox() const {
    sf::FloatRect bounds = m_sprite.getGlobalBounds();
    const float reach = 40.0f;
    if (m_facingDirection == Direction::Right) {
        return sf::FloatRect(bounds.left + bounds.width, bounds.top, reach, bounds.height);
    } else {
        return sf::FloatRect(bounds.left - reach, bounds.top, reach, bounds.height);
    }
}

void Character::heal(int amount) {
    if (!m_isAlive || amount <= 0) {
        return;
    }
    m_health = std::min(m_maxHealth, m_health + amount);
}

void Character::kill() {
    takeDamage(m_health);
}

void Character::clampToBounds(float minX, float minY, float maxX, float maxY) {
    if (m_position.x < minX) m_position.x = minX;
    if (m_position.x > maxX) m_position.x = maxX;
    if (m_position.y < minY) m_position.y = minY;
    if (m_position.y > maxY) m_position.y = maxY;
    m_sprite.setPosition(m_position);
}

// Render floating health bar above the character
void Character::renderHealthBar(sf::RenderTarget& target) const {
    if (!m_isAlive) return;

    sf::FloatRect bounds = m_sprite.getGlobalBounds();
    const float barWidth = std::max(36.0f, bounds.width * 0.8f);
    const float barHeight = 5.0f;
    const float barX = m_position.x - barWidth / 2.0f;
    const float barY = bounds.top - 10.0f;

    // Background (dark red/gray)
    sf::RectangleShape bgRect(sf::Vector2f(barWidth, barHeight));
    bgRect.setPosition(barX, barY);
    bgRect.setFillColor(sf::Color(40, 10, 10, 220));
    bgRect.setOutlineColor(sf::Color(10, 10, 10, 255));
    bgRect.setOutlineThickness(1.0f);
    target.draw(bgRect);

    // Foreground health fill (green -> red)
    float healthRatio = static_cast<float>(m_health) / static_cast<float>(m_maxHealth);
    sf::RectangleShape fgRect(sf::Vector2f(barWidth * healthRatio, barHeight));
    fgRect.setPosition(barX, barY);
    
    if (healthRatio > 0.5f) {
        fgRect.setFillColor(sf::Color(60, 220, 80)); // Green
    } else if (healthRatio > 0.25f) {
        fgRect.setFillColor(sf::Color(240, 180, 40)); // Yellow/Orange
    } else {
        fgRect.setFillColor(sf::Color(230, 40, 40)); // Red
    }
    target.draw(fgRect);
}

// OOP CONCEPT: CONST-CORRECTNESS (GETTERS AND SETTERS)
int Character::getHealth() const {
    return m_health;
}

int Character::getMaxHealth() const {
    return m_maxHealth;
}

bool Character::isAlive() const {
    return m_isAlive;
}

sf::Vector2f Character::getPosition() const {
    return m_position;
}

void Character::setPosition(const sf::Vector2f& position) {
    m_position = position;
    m_sprite.setPosition(m_position);
}

sf::Vector2f Character::getVelocity() const {
    return m_velocity;
}

void Character::setVelocity(const sf::Vector2f& velocity) {
    m_velocity = velocity;
}

float Character::getSpeed() const {
    return m_speed;
}

void Character::setSpeed(float speed) {
    if (speed >= 0.0f) {
        m_speed = speed;
    }
}

Direction Character::getFacingDirection() const {
    return m_facingDirection;
}

void Character::setFacingDirection(Direction direction) {
    m_facingDirection = direction;
    updateSpriteTransform();
}

CharacterState Character::getState() const {
    return m_state;
}

bool Character::isMoving() const {
    return m_state == CharacterState::Walking;
}

bool Character::isHurt() const {
    return m_hurtTimer > 0.0f;
}

int Character::getAttackDamage() const {
    return m_attackDamage;
}

float Character::getAttackCooldown() const {
    return m_attackCooldown;
}

sf::FloatRect Character::getGlobalBounds() const {
    return m_sprite.getGlobalBounds();
}

// ============================================================
// OOP CONCEPT: VIRTUAL FUNCTIONS AND POLYMORPHISM
// Why: Character::getCollisionBounds() provides a default 26x24 physical
// collision footprint centered at the character's ground position.
// ============================================================
sf::FloatRect Character::getCollisionBounds() const {
    return sf::FloatRect(m_position.x - 13.0f, m_position.y - 12.0f, 26.0f, 24.0f);
}
