#include "items/Weapon.hpp"
#include "items/Sword.hpp"
#include "items/Bow.hpp"
#include "items/MagicWeapon.hpp"
#include "entities/Player.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (BASE CONSTRUCTOR INITIALIZER)
// Why: Invokes the Item base constructor with stackable = false.
// ============================================================
Weapon::Weapon(const std::string& id,
               const std::string& name,
               const std::string& description,
               sf::IntRect iconRect,
               int damage,
               float cooldown,
               sf::IntRect weaponSpriteRect,
               int value)
    : Item(id, name, description, iconRect, false, value)
    , m_damage(damage)
    , m_cooldown(cooldown)
    , m_weaponSpriteRect(weaponSpriteRect) {
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
// Why: When use() is invoked on a Weapon through an Item* pointer,
// dynamic binding executes this method to equip the weapon on Player.
// ============================================================
bool Weapon::use(Player& player) {
    // Clone as shared_ptr to equip safely without pointer aliasing
    auto copy = std::shared_ptr<Weapon>(static_cast<Weapon*>(this->clone().release()));
    player.equipWeapon(copy);
    std::cout << "[Inventory] Equipped weapon: " << m_name
              << " (" << m_damage << " Dmg, " << m_cooldown << "s CD, Category: " << getWeaponCategory() << ")\n";
    return true;
}

std::unique_ptr<Item> Weapon::clone() const {
    return std::make_unique<Weapon>(*this);
}

std::string Weapon::getItemType() const {
    return "Weapon";
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & VIRTUAL METHODS
// Why: Default melee attack implementation if not overridden.
// Derived classes (Sword, Bow, MagicWeapon) customize attack mechanics.
// ============================================================
void Weapon::executeAttack(Player& player,
                          std::vector<Projectile>& /*projectiles*/,
                          const sf::Texture& /*tileset*/) {
    player.performAttack();
    std::cout << "[Weapon] Player attacks with " << m_name << "! (" << m_damage << " damage)\n";
}

std::string Weapon::getWeaponCategory() const {
    return "Melee";
}

// ============================================================
// OOP CONCEPT: FACTORY PATTERN & ABSTRACTION
// Why: Constructs balanced concrete weapons (Sword, Bow, MagicWeapon)
// returning std::shared_ptr<Weapon> for clean polymorphic use.
// ============================================================

// 1. Rusty Dagger (Sword subclass)
std::shared_ptr<Weapon> Weapon::createRustyDagger() {
    return std::make_shared<Sword>(
        "rusty_dagger",
        "Rusty Dagger",
        "Fast but rusted blade found in the catacombs.",
        sf::IntRect(288, 160, 16, 32),
        15,                           // damage
        0.22f,                        // cooldown
        sf::IntRect(288, 160, 16, 32),// sprite
        15                            // gold value
    );
}

// 2. Iron Sword (Sword subclass)
std::shared_ptr<Weapon> Weapon::createIronSword() {
    return std::make_shared<Sword>(
        "iron_sword",
        "Iron Sword",
        "A reliable steel blade forged for dungeon combat.",
        sf::IntRect(320, 160, 16, 32),
        25,                           // damage
        0.35f,                        // cooldown
        sf::IntRect(320, 160, 16, 32),// sprite
        35                            // gold value
    );
}

// 3. Golden Broadsword (Sword subclass)
std::shared_ptr<Weapon> Weapon::createGoldenBroadsword() {
    return std::make_shared<Sword>(
        "golden_broadsword",
        "Golden Broadsword",
        "Legendary royal sword imbued with fierce demonic power.",
        sf::IntRect(304, 160, 16, 32),
        45,                           // damage
        0.45f,                        // cooldown
        sf::IntRect(304, 160, 16, 32),// sprite
        100                           // gold value
    );
}

// 4. Hunter's Bow (Bow subclass)
std::shared_ptr<Weapon> Weapon::createHunterBow() {
    return std::make_shared<Bow>(
        "hunter_bow",
        "Hunter's Shortbow",
        "A swift yew bow capable of launching piercing arrows.",
        sf::IntRect(304, 160, 16, 16),
        20,                           // damage
        0.45f,                        // cooldown
        350.0f,                       // arrow speed
        50                            // value
    );
}

// 5. Staff of Embers (MagicWeapon subclass)
std::shared_ptr<Weapon> Weapon::createFireStaff() {
    return std::make_shared<MagicWeapon>(
        "fire_staff",
        "Staff of Embers",
        "Ancient staff humming with primordial pyromancy.",
        sf::IntRect(336, 160, 16, 16),
        30,                           // damage
        0.55f,                        // cooldown
        280.0f,                       // spell speed
        75                            // value
    );
}
