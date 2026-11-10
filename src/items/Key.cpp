#include "items/Key.hpp"
#include "entities/Player.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Invokes the Item base constructor with stackable = false
// and sets m_targetDoorId before derived constructor body executes.
// ============================================================
Key::Key(const std::string& id,
         const std::string& name,
         const std::string& description,
         sf::IntRect iconRect,
         const std::string& targetDoorId,
         int value)
    : Item(id, name, description, iconRect, false, value)
    , m_targetDoorId(targetDoorId) {
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: When the player activates a Key from their inventory hotbar,
// use() is called polymorphically via Item*. It inspects the key
// and prints the door it unlocks rather than consuming the key.
// ============================================================
bool Key::use(Player& player) {
    (void)player;
    std::cout << "[Inventory] Inspecting " << m_name << ": " << m_description
              << " Fits lock: [" << m_targetDoorId << "]\n";
    return false; // Return false so the key is not consumed from the hotbar!
}

std::unique_ptr<Item> Key::clone() const {
    return std::make_unique<Key>(*this);
}

std::string Key::getItemType() const {
    return "Key";
}

// ============================================================
// OOP CONCEPT: FACTORY PATTERN
// Why: Encapsulates creation of standard dungeon keys with
// exact texture rects and target door IDs.
// ============================================================
std::shared_ptr<Key> Key::createBossKey() {
    return std::make_shared<Key>(
        "boss_key",
        "Boss Chamber Key",
        "An ornate skull-crested iron key. Unlocks the Boss Sanctum door.",
        sf::IntRect(288, 32, 16, 16),
        "boss_door",
        50
    );
}

std::shared_ptr<Key> Key::createIronKey() {
    return std::make_shared<Key>(
        "iron_key",
        "Rusty Iron Key",
        "A heavy iron key found in the goblin warrens.",
        sf::IntRect(304, 32, 16, 16),
        "iron_gate",
        20
    );
}

