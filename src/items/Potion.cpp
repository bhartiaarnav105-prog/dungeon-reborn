#include "items/Potion.hpp"
#include "entities/Player.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (BASE CONSTRUCTOR INITIALIZER)
// Why: Initializes Item base class with stackable = true.
// ============================================================
Potion::Potion(const std::string& id,
               const std::string& name,
               const std::string& description,
               sf::IntRect iconRect,
               int healAmount,
               PotionType type,
               int value)
    : Item(id, name, description, iconRect, true, value)
    , m_healAmount(healAmount)
    , m_potionType(type) {
}

// ============================================================
// OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
// Why: When use() is invoked on a Potion, it safely heals the Player.
// If player is already at full health, returns false to avoid wasting the item.
// ============================================================
bool Potion::use(Player& player) {
    if (player.getHealth() >= player.getMaxHealth()) {
        std::cout << "[Inventory] Health is already full! Potion was not consumed.\n";
        return false;
    }

    int oldHealth = player.getHealth();
    player.heal(m_healAmount);
    int healed = player.getHealth() - oldHealth;

    std::cout << "[Inventory] Drank " << m_name << "! Restored "
              << healed << " HP. (Current HP: " << player.getHealth() << "/" << player.getMaxHealth() << ")\n";
    return true;
}

std::unique_ptr<Item> Potion::clone() const {
    return std::make_unique<Potion>(*this);
}

std::string Potion::getItemType() const {
    return "Potion";
}

// 1. Health Potion: Red flask (+40 HP)
std::shared_ptr<Potion> Potion::createHealthPotion() {
    return std::make_shared<Potion>(
        "potion_health",
        "Health Potion",
        "Restores up to 40 health points.",
        sf::IntRect(288, 224, 16, 16),
        40,
        PotionType::Health,
        20
    );
}

// 2. Vitality Elixir: Blue flask (+100 HP, full heal)
std::shared_ptr<Potion> Potion::createVitalityElixir() {
    return std::make_shared<Potion>(
        "potion_vitality",
        "Elixir of Vitality",
        "Rare mystical elixir that fully restores health.",
        sf::IntRect(304, 224, 16, 16),
        100,
        PotionType::Vitality,
        50
    );
}

