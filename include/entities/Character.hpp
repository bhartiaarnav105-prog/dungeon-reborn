#pragma once

#include "graphics/Animation.hpp"
#include <SFML/Graphics.hpp>
#include <string>

// Enum representing horizontal facing direction
enum class Direction {
    Left,
    Right
};

// Enum representing common character states
enum class CharacterState {
    Idle,
    Walking,
    Attacking,
    Hurt,
    Dead
};

// OOP CONCEPT: ABSTRACT CLASSES AND PURE VIRTUAL FUNCTIONS
// Why: Character is an abstract base class establishing a contract for all game actors
// (Player, Skeleton, Goblin, Boss). It cannot be instantiated directly because it contains
// pure virtual functions (= 0), guaranteeing derived classes implement specific behaviors.
class Character {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Health and max health are strictly private to prevent external or derived classes
// from directly mutating them without enforcing invariants (e.g., 0 <= health <= maxHealth).
private:
    int m_health;
    int m_maxHealth;

// OOP CONCEPT: ACCESS SPECIFIERS (PROTECTED)
// Why: Protected members are accessible to derived classes (Player, Enemy, Skeleton, etc.)
// so they can read and manipulate physical state and graphics while remaining hidden from outside code.
protected:
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    float m_speed;
    float m_scale;
    Direction m_facingDirection;
    CharacterState m_state;
    bool m_isAlive;

    // Combat attributes
    int m_attackDamage;
    float m_attackCooldown;
    float m_attackTimer;

    // Damage feedback attributes
    float m_hurtTimer;
    sf::Color m_baseColor;

    // OOP CONCEPT: COMPOSITION
    // Why: Every Character has-a sf::Sprite and Animation components for idle and walking.
    sf::Sprite m_sprite;
    Animation m_idleAnimation;
    Animation m_walkAnimation;

    // Protected helper to flip sprite horizontally based on facing direction
    void updateSpriteTransform();

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Public interface exposing lifecycle, damage handling, positioning, and rendering.
public:
    // OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
    // Why: Parameterized constructor initializes shared stats (health, speed, scale).
    // The virtual destructor is essential: deleting a derived object through a Character*
    // base pointer must invoke the derived destructor to avoid undefined behavior and leaks.
    Character(int maxHealth, float speed, float scale, const sf::Texture& texture, sf::Vector2f startPosition = {0.f, 0.f});
    virtual ~Character() = default;

    // OOP CONCEPT: ABSTRACTION AND RUNTIME POLYMORPHISM
    // Why: Pure virtual functions define interface requirements that must be overridden
    // by derived classes. The appropriate implementation is resolved dynamically at runtime.
    virtual void update(float deltaTime) = 0;
    virtual void performAttack() = 0;
    virtual std::string getCharacterType() const = 0;

    // OOP CONCEPT: FUNCTION OVERRIDING AND VIRTUAL FUNCTIONS
    // Why: render() provides a default implementation that draws the sprite and health bar,
    // but can be overridden by specialized derived classes if needed.
    virtual void render(sf::RenderTarget& target) const;

    // OOP CONCEPT: RUNTIME POLYMORPHISM & ABSTRACTION
    // Why: getAttackHitbox() returns the directional damage area of an active attack.
    // Derived classes define their specific reach and orientation.
    virtual sf::FloatRect getAttackHitbox() const;

    // Damage feedback update helper
    void updateDamageFeedback(float deltaTime);

    // OOP CONCEPT: ENCAPSULATION
    // Why: Damage and healing logic is centralized in the base class. State changes
    // strictly enforce boundary conditions (clamped to 0 and maxHealth).
    virtual void takeDamage(int amount);
    void heal(int amount);
    void kill();

    // Screen and dungeon boundary clamping
    void clampToBounds(float minX, float minY, float maxX, float maxY);

    // OOP CONCEPT: CONST-CORRECTNESS AND GETTERS / SETTERS
    // Why: Member functions inspecting state are marked const, guaranteeing no mutations.
    int getHealth() const;
    int getMaxHealth() const;
    bool isAlive() const;

    sf::Vector2f getPosition() const;
    void setPosition(const sf::Vector2f& position);

    sf::Vector2f getVelocity() const;
    void setVelocity(const sf::Vector2f& velocity);

    float getSpeed() const;
    void setSpeed(float speed);

    Direction getFacingDirection() const;
    void setFacingDirection(Direction direction);

    CharacterState getState() const;
    bool isMoving() const;
    bool isHurt() const;
    int getAttackDamage() const;
    float getAttackCooldown() const;

    sf::FloatRect getGlobalBounds() const;

    // ============================================================
    // OOP CONCEPT: VIRTUAL FUNCTIONS AND POLYMORPHISM
    // Why: getCollisionBounds() returns the physical collision footprint of the character.
    // Derived character types (Player, Skeleton, Goblin, Boss) override this with their specific
    // foot/body bounds to navigate narrow corridors without getting stuck on walls.
    // ============================================================
    virtual sf::FloatRect getCollisionBounds() const;

    // Helper to render a mini health bar above the character
    void renderHealthBar(sf::RenderTarget& target) const;
};
