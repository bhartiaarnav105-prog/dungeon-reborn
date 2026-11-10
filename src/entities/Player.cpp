#include "entities/Player.hpp"
#include "dungeon/Dungeon.hpp"
#include "items/Potion.hpp"
#include "entities/Projectile.hpp"
#include <cmath>
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (MEMBER INITIALIZATION LIST WITH BASE CONSTRUCTOR)
// Why: Player constructor explicitly invokes Character base class constructor
// passing knight base stats (100 HP, 200 speed, 1.8 scale).
// Scale 1.8 separates visual size from collision size, sizing the knight's torso
// to 20-30% of corridor width (14px torso in 48px corridor) for smooth navigation.
// ============================================================
Player::Player(const sf::Texture& texture, sf::Vector2f startPosition)
    : Character(100, 200.0f, 1.8f, texture, startPosition)
    , m_isAttacking(false)
    , m_attackDuration(0.25f)
    , m_attackElapsed(0.0f)
    , m_attackCooldownTimer(0.0f)
    , m_hasDealtDamageThisAttack(false)
    , m_inventory(6)
    , m_equippedWeapon(nullptr)
    , m_weaponSprite(texture, sf::IntRect(320, 160, 16, 32)) {

    // OOP CONCEPT: COMPOSITION
    // Why: Player starts with an equipped Iron Sword and initial inventory items.
    equipWeapon(Weapon::createIronSword());
    m_inventory.addItem(m_equippedWeapon, 1);
    m_inventory.addItem(Potion::createHealthPotion(), 2);

    // The hilt origin (8, 26) allows natural rotational slashing arcs
    m_weaponSprite.setOrigin(8.0f, 26.0f);
    m_weaponSprite.setScale(m_scale, m_scale);

    // Player sprite uses bright readable silver-white tone
    m_sprite.setColor(sf::Color(255, 255, 255));

    // 0x72 Dungeon Tileset II Knight Coordinates:
    // Knight idle animation: 4 frames of 16x28 starting at x=128, y=100
    for (int i = 0; i < 4; ++i) {
        m_idleAnimation.addFrame(sf::IntRect(128 + i * 16, 100, 16, 28));
    }

    // Knight run animation: 4 frames of 16x28 starting at x=192, y=100
    for (int i = 0; i < 4; ++i) {
        m_walkAnimation.addFrame(sf::IntRect(192 + i * 16, 100, 16, 28));
    }

    // Pivot around center-bottom
    m_sprite.setOrigin(8.0f, 14.0f);
    m_sprite.setTextureRect(m_idleAnimation.getCurrentFrameRect());
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: Handles WASD, arrow keys, attack key, and hotkeys 1-6 for inventory slots.
// ============================================================
void Player::handleInput() {
    if (!m_isAlive) return;

    m_velocity = sf::Vector2f(0.0f, 0.0f);

    // Support W, A, S, D and Arrow Keys
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        m_velocity.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        m_velocity.y += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        m_velocity.x -= 1.0f;
        m_facingDirection = Direction::Left;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        m_velocity.x += 1.0f;
        m_facingDirection = Direction::Right;
    }

    // Space key initiates sword attack if cooldown allows
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        if (!m_isAttacking && m_attackCooldownTimer <= 0.0f) {
            performAttack();
        }
    }

    // Number keys [1] through [6] trigger hotbar inventory usage / weapon equip
    static bool numPressedLast[6] = {false, false, false, false, false, false};
    sf::Keyboard::Key numKeys[6] = {
        sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3,
        sf::Keyboard::Num4, sf::Keyboard::Num5, sf::Keyboard::Num6
    };

    for (size_t i = 0; i < 6; ++i) {
        bool isDown = sf::Keyboard::isKeyPressed(numKeys[i]);
        if (isDown && !numPressedLast[i]) {
            m_inventory.useItem(i, *this);
        }
        numPressedLast[i] = isDown;
    }

    // Normalize diagonal movement vector so diagonal travel isn't faster
    float length = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
    if (length > 0.0f) {
        m_velocity.x /= length;
        m_velocity.y /= length;
        if (!m_isAttacking) {
            m_state = CharacterState::Walking;
        }
    } else if (!m_isAttacking) {
        m_state = CharacterState::Idle;
    }
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
// Why: Implements Character::update() specifically for the player, integrating delta-time
// movement, attack cooldowns, sword swinging animation, and damage feedback.
// ============================================================
void Player::update(float deltaTime) {
    if (!m_isAlive) {
        m_state = CharacterState::Dead;
        return;
    }

    // Time-based movement integration
    m_position += m_velocity * m_speed * deltaTime;
    m_sprite.setPosition(m_position);

    // Tick hurt damage feedback
    updateDamageFeedback(deltaTime);

    // Tick attack cooldown timer
    if (m_attackCooldownTimer > 0.0f) {
        m_attackCooldownTimer -= deltaTime;
    }

    // Attack animation handling
    if (m_isAttacking) {
        m_attackElapsed += deltaTime;
        float progress = m_attackElapsed / m_attackDuration;

        // Visual slash pose on knight
        m_sprite.setTextureRect(sf::IntRect(320, 100, 16, 28));

        // Sword swing arc rotation and placement
        if (m_facingDirection == Direction::Right) {
            float angle = -45.0f + progress * 110.0f; // -45 deg to +65 deg
            m_weaponSprite.setPosition(m_position.x + 10.0f, m_position.y - 2.0f);
            m_weaponSprite.setScale(m_scale, m_scale);
            m_weaponSprite.setRotation(angle);
        } else {
            float angle = 45.0f - progress * 110.0f; // +45 deg to -65 deg
            m_weaponSprite.setPosition(m_position.x - 10.0f, m_position.y - 2.0f);
            m_weaponSprite.setScale(-m_scale, m_scale);
            m_weaponSprite.setRotation(angle);
        }

        if (m_attackElapsed >= m_attackDuration) {
            m_isAttacking = false;
            m_attackElapsed = 0.0f;
            m_state = (m_velocity.x != 0.0f || m_velocity.y != 0.0f) ? CharacterState::Walking : CharacterState::Idle;
        }
    } else {
        // Animation frame progression when not attacking
        if (m_state == CharacterState::Walking) {
            m_walkAnimation.update(deltaTime);
            m_sprite.setTextureRect(m_walkAnimation.getCurrentFrameRect());
        } else {
            m_idleAnimation.update(deltaTime);
            m_sprite.setTextureRect(m_idleAnimation.getCurrentFrameRect());
        }
    }

    updateSpriteTransform();
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING
// Why: Overrides Character::render() to draw the active weapon swing overlay.
// ============================================================
void Player::render(sf::RenderTarget& target) const {
    Character::render(target); // Draw knight body and health bar

    // Draw swinging sword during active attack
    if (m_isAttacking && m_isAlive) {
        target.draw(m_weaponSprite);
    }
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING
// Why: Overrides Character::performAttack() with knight's weapon swing.
// Sets single-hit flag to false so only one strike registers per swing.
// ============================================================
void Player::performAttack() {
    if (!m_isAttacking && m_isAlive && m_attackCooldownTimer <= 0.0f) {
        m_isAttacking = true;
        m_attackElapsed = 0.0f;
        m_hasDealtDamageThisAttack = false;
        m_attackCooldownTimer = m_attackCooldown;
        m_state = CharacterState::Attacking;
        std::cout << "[Player] Knight slashes with "
                  << (m_equippedWeapon ? m_equippedWeapon->getName() : "fists")
                  << "! Dealing " << m_attackDamage << " potential damage." << std::endl;
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & DELEGATION
// Why: Delegates attack execution polymorphically to m_equippedWeapon->executeAttack().
// If the player has a Sword equipped, it executes a melee slash;
// if a Bow is equipped, it fires an arrow projectile;
// if a MagicWeapon is equipped, it launches an arcane orb projectile.
// Demonstrates OOP Composition (Player HAS-A Weapon) and Polymorphic Delegation.
// ============================================================
void Player::performAttackWithWeapon(std::vector<Projectile>& projectiles, const sf::Texture& tileset) {
    if (m_equippedWeapon) {
        m_equippedWeapon->executeAttack(*this, projectiles, tileset);
    } else {
        performAttack();
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & ABSTRACTION
// Why: Returns a directional hitbox extending in front of the knight.
// Sized cleanly to match the 1.8 visual scale.
// ============================================================
sf::FloatRect Player::getAttackHitbox() const {
    const float reach = 44.0f;
    const float boxH  = 34.0f;
    if (m_facingDirection == Direction::Right) {
        return sf::FloatRect(m_position.x, m_position.y - boxH * 0.5f, reach, boxH);
    } else {
        return sf::FloatRect(m_position.x - reach, m_position.y - boxH * 0.5f, reach, boxH);
    }
}

bool Player::canDeliverHit() const {
    return m_isAttacking && !m_hasDealtDamageThisAttack && m_attackElapsed >= 0.03f;
}

void Player::markAttackHitDelivered() {
    m_hasDealtDamageThisAttack = true;
}

float Player::getAttackCooldownTimer() const {
    return m_attackCooldownTimer;
}

std::string Player::getCharacterType() const {
    return "Player";
}

bool Player::isAttacking() const {
    return m_isAttacking;
}

// ============================================================
// OOP CONCEPT: COMPOSITION AND ENCAPSULATION
// Why: Equipping a weapon updates internal combat parameters
// (damage, cooldown, sprite rect) without exposing private fields.
// ============================================================
void Player::equipWeapon(std::shared_ptr<Weapon> weapon) {
    if (!weapon) return;

    m_equippedWeapon = weapon;
    m_attackDamage = weapon->getDamage();
    m_attackCooldown = weapon->getCooldown();
    m_weaponSprite.setTextureRect(weapon->getWeaponSpriteRect());
}

const Weapon* Player::getEquippedWeapon() const {
    return m_equippedWeapon.get();
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
// Why: Overrides Character::getCollisionBounds() with the knight's physical
// foot footprint (14x12 px). Sized to 20-30% of corridor width (48px),
// separating visual representation from collision physics and allowing
// fluid navigation through narrow corridors, intersections, and around enemies.
// ============================================================
sf::FloatRect Player::getCollisionBounds() const {
    return sf::FloatRect(m_position.x - 7.0f, m_position.y + 2.0f, 14.0f, 12.0f);
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION & ABSTRACTION
// Why: Player does not directly access Dungeon's internal map.
// Instead, the Player queries the Dungeon high-level interface
// to verify whether a proposed displacement remains entirely walkable.
// ============================================================
bool Player::isMovementValid(sf::Vector2f desiredMove, const Dungeon& dungeon) const {
    sf::FloatRect movedBounds = getCollisionBounds();
    movedBounds.left += desiredMove.x;
    movedBounds.top  += desiredMove.y;
    return dungeon.isAreaWalkable(movedBounds);
}
