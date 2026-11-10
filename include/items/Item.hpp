#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <memory>

// Forward declaration to prevent circular include dependencies
class Player;

// ============================================================
// OOP CONCEPT: ABSTRACT CLASSES AND PURE VIRTUAL FUNCTIONS
// Why: Item is an abstract base class defining the foundational
// contract for all collectable, inventoriable objects in the game
// (Weapons, Potions, Quest items). It cannot be directly instantiated
// because use() and clone() are pure virtual (= 0).
// ============================================================
class Item {
// OOP CONCEPT: ACCESS SPECIFIERS (PROTECTED)
// Why: Shared attributes (name, description, atlas texture rect)
// are accessible to derived subclasses (Weapon, Potion) while remaining
// hidden from arbitrary external modification.
protected:
    std::string m_id;
    std::string m_name;
    std::string m_description;
    sf::IntRect m_iconRect; // Coordinates in 0x72 dungeon tileset
    bool        m_stackable;
    int         m_value;    // Gold sale value

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
    // Why: Parameterized constructor initializes base item metadata.
    // The virtual destructor is essential: deleting an Item through
    // an Item* or std::shared_ptr<Item> must properly invoke derived destructors.
    // ============================================================
    Item(const std::string& id,
         const std::string& name,
         const std::string& description,
         sf::IntRect iconRect,
         bool stackable = false,
         int value = 10);

    virtual ~Item() = default;

    // ============================================================
    // OOP CONCEPT: ABSTRACTION AND RUNTIME POLYMORPHISM
    // Why: use() defines what happens when an item is consumed or equipped.
    // The specific logic is implemented exclusively by the derived class.
    // Returns true if the item was successfully used/equipped.
    // ============================================================
    virtual bool use(Player& player) = 0;

    // Prototype pattern: deep-copies concrete derived items polymorphically
    virtual std::unique_ptr<Item> clone() const = 0;

    virtual std::string getItemType() const = 0;

    // ============================================================
    // OOP CONCEPT: CONST-CORRECTNESS & ENCAPSULATION
    // Why: Pure state queries are marked const and return copies/const refs.
    // ============================================================
    const std::string& getId()          const { return m_id; }
    const std::string& getName()        const { return m_name; }
    const std::string& getDescription() const { return m_description; }
    sf::IntRect        getIconRect()    const { return m_iconRect; }
    bool               isStackable()    const { return m_stackable; }
    int                getValue()       const { return m_value; }
};

