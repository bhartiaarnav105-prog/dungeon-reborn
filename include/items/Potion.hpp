#pragma once

#include "items/Item.hpp"

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS (SCOPED ENUM)
// ============================================================
enum class PotionType {
    Health,
    Vitality
};

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Potion inherits from Item, reusing identity, icon coordinates,
// and inventory stackability while specializing for consumable healing.
// ============================================================
class Potion : public Item {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
private:
    int        m_healAmount;
    PotionType m_potionType;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Initializes consumable Item with stackable = true.
    // ============================================================
    Potion(const std::string& id,
           const std::string& name,
           const std::string& description,
           sf::IntRect iconRect,
           int healAmount,
           PotionType type = PotionType::Health,
           int value = 20);

    virtual ~Potion() override = default;

    // ============================================================
    // OOP CONCEPT: FUNCTION OVERRIDING AND RUNTIME POLYMORPHISM
    // Why: Overrides pure virtual use() from Item. Consuming a potion
    // heals the Player, clamped to max health. Returns false if already full.
    // ============================================================
    bool use(Player& player) override;

    std::unique_ptr<Item> clone() const override;

    std::string getItemType() const override;

    int        getHealAmount() const { return m_healAmount; }
    PotionType getPotionType() const { return m_potionType; }

    // Potion factory presets
    static std::shared_ptr<Potion> createHealthPotion();
    static std::shared_ptr<Potion> createVitalityElixir();
};

