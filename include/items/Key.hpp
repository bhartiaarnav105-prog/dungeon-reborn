#pragma once

#include "items/Item.hpp"

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Key IS-AN Item. It inherits all fundamental item metadata
// (id, name, description, iconRect, value) from the abstract Item base class.
//
// OOP CONCEPT: ENCAPSULATION
// Why: m_targetDoorId is strictly encapsulated. External systems inspect
// which door this key opens via getTargetDoorId() without modifying it.
// ============================================================
class Key : public Item {
private:
    std::string m_targetDoorId;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS (BASE CONSTRUCTOR CALL)
    // Why: Invokes Item(id, name, description, iconRect, stackable=false, value).
    // Initializes the door target identifier.
    // ============================================================
    Key(const std::string& id,
        const std::string& name,
        const std::string& description,
        sf::IntRect iconRect,
        const std::string& targetDoorId,
        int value = 0);

    ~Key() override = default;

    // ============================================================
    // OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
    // Why: Overrides Item::use(player). Keys are not consumed like potions;
    // using a key from the hotbar inspects the key and reminds the player
    // which door lock it fits.
    // ============================================================
    bool use(Player& player) override;

    // Prototype pattern clone
    std::unique_ptr<Item> clone() const override;

    std::string getItemType() const override;

    const std::string& getTargetDoorId() const { return m_targetDoorId; }

    // Static factory methods for consistent key creation
    static std::shared_ptr<Key> createBossKey();
    static std::shared_ptr<Key> createIronKey();
};

