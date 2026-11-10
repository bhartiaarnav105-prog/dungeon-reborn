#include "entities/Enemy.hpp"
#include <cmath>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (BASE DELEGATION VIA INITIALIZATION LIST)
// Why: Enemy delegates foundational state initialization (health, speed, scale)
// to Character and initializes enemy-specific AI ranges and states.
// ============================================================
Enemy::Enemy(int maxHealth, float speed, float scale, const sf::Texture& texture,
             sf::Vector2f startPosition, float detectionRadius, float attackRange)
    : Character(maxHealth, speed, scale, texture, startPosition)
    , m_detectionRadius(detectionRadius)
    , m_attackRange(attackRange)
    , m_isChasing(false)
    , m_aiTimer(0.0f)
    , m_aiState(AIState::Idle)
    , m_hasDroppedLoot(false) {
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING
// Why: Standard update ticks damage feedback and animations.
// ============================================================
void Enemy::update(float deltaTime) {
    if (!m_isAlive) {
        m_state = CharacterState::Dead;
        m_aiState = AIState::Dead;
        return;
    }

    m_position += m_velocity * m_speed * deltaTime;
    m_sprite.setPosition(m_position);

    updateDamageFeedback(deltaTime);

    if (m_state == CharacterState::Walking) {
        m_walkAnimation.update(deltaTime);
        m_sprite.setTextureRect(m_walkAnimation.getCurrentFrameRect());
    } else {
        m_idleAnimation.update(deltaTime);
        m_sprite.setTextureRect(m_idleAnimation.getCurrentFrameRect());
    }

    updateSpriteTransform();
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM (DYNAMIC BINDING)
// Why: Calls the pure virtual updateAI() which resolves dynamically to the derived
// enemy's AI implementation (Skeleton, Goblin, or Boss) at runtime.
// ============================================================
void Enemy::updateWithTarget(float deltaTime, const sf::Vector2f& playerPos) {
    if (!m_isAlive) {
        m_state = CharacterState::Dead;
        m_aiState = AIState::Dead;
        return;
    }

    // Call polymorphic AI behavior
    updateAI(deltaTime, playerPos);

    // Apply movement
    m_position += m_velocity * m_speed * deltaTime;
    m_sprite.setPosition(m_position);

    // Tick visual damage feedback (flash)
    updateDamageFeedback(deltaTime);

    // Update animations
    if (m_state == CharacterState::Walking) {
        m_walkAnimation.update(deltaTime);
        m_sprite.setTextureRect(m_walkAnimation.getCurrentFrameRect());
    } else {
        m_idleAnimation.update(deltaTime);
        m_sprite.setTextureRect(m_idleAnimation.getCurrentFrameRect());
    }

    updateSpriteTransform();
}

// Distance calculation utility
float Enemy::distanceTo(const sf::Vector2f& target) const {
    float dx = target.x - m_position.x;
    float dy = target.y - m_position.y;
    return std::sqrt(dx * dx + dy * dy);
}

// Direction vector calculation utility
sf::Vector2f Enemy::directionTo(const sf::Vector2f& target) const {
    float dx = target.x - m_position.x;
    float dy = target.y - m_position.y;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length > 0.0001f) {
        return sf::Vector2f(dx / length, dy / length);
    }
    return sf::Vector2f(0.0f, 0.0f);
}

float Enemy::getDetectionRadius() const {
    return m_detectionRadius;
}

float Enemy::getAttackRange() const {
    return m_attackRange;
}

bool Enemy::isChasing() const {
    return m_isChasing;
}

AIState Enemy::getAIState() const {
    return m_aiState;
}

void Enemy::setAIState(AIState state) {
    m_aiState = state;
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
// Why: Standard enemies (Skeleton, Goblin) use a 24x24 physical footprint
// allowing them to navigate corridors without getting hung up on walls.
// ============================================================
sf::FloatRect Enemy::getCollisionBounds() const {
    return sf::FloatRect(m_position.x - 12.0f, m_position.y - 6.0f, 24.0f, 24.0f);
}
