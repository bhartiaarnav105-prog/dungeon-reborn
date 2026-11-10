#include "items/Inventory.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Sets capacity and zero starting gold cleanly.
// ============================================================
Inventory::Inventory(size_t capacity)
    : m_capacity(capacity)
    , m_gold(0) {
    m_slots.reserve(capacity);
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: addItem enforces capacity and handles item stacking
// invariants without exposing the internal std::vector.
// ============================================================
bool Inventory::addItem(std::shared_ptr<Item> item, int quantity) {
    if (!item || quantity <= 0) return false;

    // 1. If item is stackable, check if already present
    if (item->isStackable()) {
        for (auto& slot : m_slots) {
            if (slot.item && slot.item->getId() == item->getId()) {
                slot.quantity += quantity;
                std::cout << "[Inventory] Stacked " << quantity << "x " << item->getName()
                          << " (Total: " << slot.quantity << ")\n";
                return true;
            }
        }
    }

    // 2. Check capacity before adding new slot
    if (m_slots.size() >= m_capacity) {
        std::cout << "[Inventory WARNING] Cannot add " << item->getName()
                  << ": Inventory is FULL (" << m_slots.size() << "/" << m_capacity << ")!\n";
        return false;
    }

    // 3. Insert new slot
    m_slots.push_back({item, quantity});
    std::cout << "[Inventory] Added " << item->getName()
              << " (" << m_slots.size() << "/" << m_capacity << " slots used)\n";
    return true;
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: removeItem safely decrements quantity or removes the slot when 0.
// ============================================================
bool Inventory::removeItem(size_t slotIndex, int quantity) {
    if (slotIndex >= m_slots.size() || quantity <= 0) return false;

    auto& slot = m_slots[slotIndex];
    if (slot.quantity <= quantity) {
        m_slots.erase(m_slots.begin() + slotIndex);
    } else {
        slot.quantity -= quantity;
    }
    return true;
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & DYNAMIC DISPATCH
// Why: Invokes item->use(player) via base pointer Item*.
// If the item is a Weapon, it equips it. If a Potion, it heals.
// If consumed (and stackable), quantity is decremented automatically.
// ============================================================
bool Inventory::useItem(size_t slotIndex, Player& player) {
    if (slotIndex >= m_slots.size()) return false;

    auto& slot = m_slots[slotIndex];
    if (!slot.item) return false;

    // Polymorphic invocation
    bool used = slot.item->use(player);
    if (!used) return false;

    // Consumable items (potions) are consumed upon use
    if (slot.item->isStackable()) {
        slot.quantity--;
        if (slot.quantity <= 0) {
            m_slots.erase(m_slots.begin() + slotIndex);
        }
    }
    return true;
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: Querying and removing items by ID searches the internal
// m_slots container without exposing the raw container to callers.
// ============================================================
bool Inventory::hasItem(const std::string& itemId) const {
    for (const auto& slot : m_slots) {
        if (slot.item && slot.item->getId() == itemId) {
            return true;
        }
    }
    return false;
}

bool Inventory::removeItemById(const std::string& itemId, int quantity) {
    for (size_t i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].item && m_slots[i].item->getId() == itemId) {
            return removeItem(i, quantity);
        }
    }
    return false;
}

std::shared_ptr<Item> Inventory::getItemById(const std::string& itemId) const {
    for (const auto& slot : m_slots) {
        if (slot.item && slot.item->getId() == itemId) {
            return slot.item;
        }
    }
    return nullptr;
}

void Inventory::addGold(int amount) {
    if (amount > 0) {
        m_gold += amount;
        std::cout << "[Inventory] Collected " << amount << " Gold. (Total: " << m_gold << " Gold)\n";
    }
}

bool Inventory::spendGold(int amount) {
    if (amount > 0 && m_gold >= amount) {
        m_gold -= amount;
        return true;
    }
    return false;
}

const InventorySlot* Inventory::getSlot(size_t index) const {
    if (index < m_slots.size()) {
        return &m_slots[index];
    }
    return nullptr;
}

void Inventory::clear() {
    m_slots.clear();
    m_gold = 0;
}

