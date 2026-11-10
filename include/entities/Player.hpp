#pragma once

#include "entities/Character.hpp"
#include "items/Inventory.hpp"
#include "items/Weapon.hpp"
#include <memory>

class Dungeon;

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Player inherits from the abstract Character base class, reusing shared
// properties (health, maxHealth, position, velocity, speed, sprite, and animations)
// while specializing behavior for user-driven keyboard input and knight combat.
// ============================================================
class Player : public Character {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Player-specific state like attack animation timers and combo states
// are private to Player and not exposed to other entities.
private:
    bool  m_isAttacking;
    float m_attackDuration;
    float m_attackElapsed;
    float m_attackCooldownTimer;
    bool  m_hasDealtDamageThisAttack;

    // OOP CONCEPT: COMPOSITION
    // Why: Player has-an Inventory (m_inventory) managing collected items,
    // and has-an equipped Weapon (m_equippedWeapon). Strong ownership.
    Inventory                     m_inventory;
    std::shared_ptr<Weapon>       m_equippedWeapon;
    sf::Sprite                    m_weaponSprite;

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Public API for input handling, specialized updates, and player actions.
public:
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Derived constructor passes initial knight stats (100 HP, 200 speed, 3.5 scale)
    // up to the Character base class constructor via initialization list.
    Player(const sf::Texture& texture, sf::Vector2f startPosition = sf::Vector2f(480.f, 320.f));
    virtual ~Player() override = default;

    // OOP CONCEPT: ENCAPSULATION
    // Why: Reads keyboard input (WASD / Arrows / Space / Hotkeys 1-6) and translates
    // it into velocity vectors, attack requests, and inventory item usage.
    void handleInput();

    // OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
    // Why: Overrides pure virtual update(), performAttack(), render(), and getAttackHitbox() from Character.
    // When invoked via Character* or Character&, Player's specific implementation is called.
    void update(float deltaTime) override;
    void performAttack() override;
    void render(sf::RenderTarget& target) const override;
    sf::FloatRect getAttackHitbox() const override;
    sf::FloatRect getCollisionBounds() const override;
    std::string getCharacterType() const override;

    // Controlled single-hit attack system
    bool isAttacking() const;
    bool canDeliverHit() const;
    void markAttackHitDelivered();
    float getAttackCooldownTimer() const;

    // OOP CONCEPT: RUNTIME POLYMORPHISM & DELEGATION
    // Why: Delegates attack execution polymorphically to m_equippedWeapon->executeAttack().
    void performAttackWithWeapon(std::vector<Projectile>& projectiles, const sf::Texture& tileset);

    // ============================================================
    // OOP CONCEPT: ENCAPSULATION & ABSTRACTION
    // Why: Player does not directly access Dungeon's internal map.
    // The player asks the Dungeon whether a proposed movement is valid,
    // protecting map invariants and decoupling Player from TileMap layout.
    // ============================================================
    bool isMovementValid(sf::Vector2f desiredMove, const Dungeon& dungeon) const;

    // ============================================================
    // OOP CONCEPT: COMPOSITION AND ENCAPSULATION (EQUIPMENT & INVENTORY)
    // Why: Exposes controlled access to inventory and weapon swapping.
    // ============================================================
    void equipWeapon(std::shared_ptr<Weapon> weapon);
    const Weapon* getEquippedWeapon() const;
    Inventory& getInventory() { return m_inventory; }
    const Inventory& getInventory() const { return m_inventory; }
};
