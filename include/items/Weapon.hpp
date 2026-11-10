#pragma once

#include "items/Item.hpp"
#include <vector>

// Forward declarations
class Projectile;

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Weapon inherits from Item, reusing base identity and atlas
// metadata while specializing attributes for offensive combat
// (attack damage, cooldown, reach, and weapon sprite rect).
//
// OOP CONCEPT: ABSTRACTION & RUNTIME POLYMORPHISM
// Why: Weapon defines the contract executeAttack() and getWeaponCategory()
// for all weapon archetypes (Sword, Bow, MagicWeapon). Derived classes
// implement unique attack mechanics (melee slashes, physical arrows, magic bolts).
// ============================================================
class Weapon : public Item {
// OOP CONCEPT: ACCESS SPECIFIERS (PROTECTED)
// Why: Subclasses (Sword, Bow, MagicWeapon) can access core weapon stats
// while keeping them protected from arbitrary external mutation.
protected:
    int         m_damage;
    float       m_cooldown;
    sf::IntRect m_weaponSpriteRect; // Sub-rectangle for in-hand swinging sprite

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Chained base constructor initializes Item properties (non-stackable).
    // ============================================================
    Weapon(const std::string& id,
           const std::string& name,
           const std::string& description,
           sf::IntRect iconRect,
           int damage,
           float cooldown,
           sf::IntRect weaponSpriteRect,
           int value = 25);

    virtual ~Weapon() override = default;

    // ============================================================
    // OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
    // Why: Overrides pure virtual use() from Item. Equipping a Weapon
    // binds it to the Player's active combat slots and updates player stats.
    // ============================================================
    bool use(Player& player) override;

    // Polymorphic cloning
    std::unique_ptr<Item> clone() const override;

    std::string getItemType() const override;

    // ============================================================
    // OOP CONCEPT: VIRTUAL FUNCTIONS & RUNTIME POLYMORPHISM
    // Why: executeAttack defines what happens when the player strikes:
    // - Sword initiates directional melee hitboxes.
    // - Bow fires arrow Projectiles forward.
    // - MagicWeapon casts burning fireball or shockwave Projectiles.
    // ============================================================
    virtual void executeAttack(Player& player,
                               std::vector<Projectile>& projectiles,
                               const sf::Texture& tileset);

    virtual std::string getWeaponCategory() const;

    // ============================================================
    // OOP CONCEPT: CONST-CORRECTNESS & ENCAPSULATION
    // ============================================================
    int         getDamage()           const { return m_damage; }
    float       getCooldown()         const { return m_cooldown; }
    sf::IntRect getWeaponSpriteRect() const { return m_weaponSpriteRect; }

    // Weapon factory presets
    static std::shared_ptr<Weapon> createRustyDagger();
    static std::shared_ptr<Weapon> createIronSword();
    static std::shared_ptr<Weapon> createGoldenBroadsword();
    static std::shared_ptr<Weapon> createHunterBow();
    static std::shared_ptr<Weapon> createFireStaff();
};
