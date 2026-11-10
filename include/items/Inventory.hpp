#pragma once

#include "items/Item.hpp"
#include <vector>
#include <memory>
#include <optional>

class Player;

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: InventorySlot is a value struct encapsulating a single
// inventory cell (item reference and quantity).
// ============================================================
struct InventorySlot {
    std::shared_ptr<Item> item;
    int                   quantity;
};

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: Inventory strictly encapsulates its internal storage (m_slots).
// Client code cannot directly modify, insert, or delete raw elements.
// All modifications go through verified public operations (addItem,
// removeItem, useItem) ensuring capacity constraints and invariants.
// ============================================================
class Inventory {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Container storage and currency values are strictly private.
private:
    std::vector<InventorySlot> m_slots;
    size_t                     m_capacity;
    int                        m_gold;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Initializes inventory with a fixed maximum capacity (default 6 slots).
    // ============================================================
    explicit Inventory(size_t capacity = 6);
    ~Inventory() = default;

    // Non-copyable to prevent accidental object duplication
    Inventory(const Inventory&)            = delete;
    Inventory& operator=(const Inventory&) = delete;

    // Move operations
    Inventory(Inventory&&) noexcept            = default;
    Inventory& operator=(Inventory&&) noexcept = default;

    // ============================================================
    // OOP CONCEPT: ENCAPSULATION (SAFE OPERATIONS)
    // Why: addItem checks capacity and manages stacking for stackable items.
    // Returns true on success, false if inventory is full.
    // ============================================================
    bool addItem(std::shared_ptr<Item> item, int quantity = 1);

    // Removes an item or decrements its quantity.
    bool removeItem(size_t slotIndex, int quantity = 1);

    // Uses the item at the specified slot polymorphically.
    bool useItem(size_t slotIndex, Player& player);

    // Checks if inventory contains an item with the given ID
    bool hasItem(const std::string& itemId) const;

    // Removes an item matching the given ID
    bool removeItemById(const std::string& itemId, int quantity = 1);

    // Retrieves an item matching the given ID
    std::shared_ptr<Item> getItemById(const std::string& itemId) const;

    // Currency management
    void addGold(int amount);
    bool spendGold(int amount);
    int  getGold() const { return m_gold; }

    // ============================================================
    // OOP CONCEPT: CONST-CORRECTNESS & ENCAPSULATION
    // Why: Read-only inspection methods. Returns const pointers or values.
    // The internal container is never exposed as a mutable reference.
    // ============================================================
    const InventorySlot* getSlot(size_t index) const;
    size_t getCapacity() const { return m_capacity; }
    size_t getItemCount() const { return m_slots.size(); }
    bool   isFull() const { return m_slots.size() >= m_capacity; }
    void   clear();
};

