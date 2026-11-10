#include "items/Item.hpp"

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Directly initializes base item attributes before derived
// class constructor bodies execute.
// ============================================================
Item::Item(const std::string& id,
           const std::string& name,
           const std::string& description,
           sf::IntRect iconRect,
           bool stackable,
           int value)
    : m_id(id)
    , m_name(name)
    , m_description(description)
    , m_iconRect(iconRect)
    , m_stackable(stackable)
    , m_value(value) {
}

