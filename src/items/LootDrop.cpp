#include "items/LootDrop.hpp"
#include "entities/Player.hpp"
#include <cmath>
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Initializes Item loot drop with sprite from tileset.
// ============================================================
LootDrop::LootDrop(sf::Vector2f position,
                   std::shared_ptr<Item> item,
                   int quantity,
                   const sf::Texture& tileset)
    : m_position(position)
    , m_basePosition(position)
    , m_item(item)
    , m_quantity(quantity)
    , m_goldValue(0)
    , m_isGold(false)
    , m_isCollected(false)
    , m_bobTimer(0.0f)
    , m_sprite(tileset, item ? item->getIconRect() : sf::IntRect(0, 0, 16, 16)) {

    m_sprite.setOrigin(8.0f, 8.0f);
    m_sprite.setScale(2.5f, 2.5f);
    m_sprite.setPosition(m_position);
}

// Gold drop constructor: Coin sprite at (288, 272, 16, 16)
LootDrop::LootDrop(sf::Vector2f position,
                   int goldAmount,
                   const sf::Texture& tileset)
    : m_position(position)
    , m_basePosition(position)
    , m_item(nullptr)
    , m_quantity(1)
    , m_goldValue(goldAmount)
    , m_isGold(true)
    , m_isCollected(false)
    , m_bobTimer(0.0f)
    , m_sprite(tileset, sf::IntRect(288, 272, 16, 16)) {

    m_sprite.setOrigin(8.0f, 8.0f);
    m_sprite.setScale(2.5f, 2.5f);
    m_sprite.setPosition(m_position);
}

// Bobbing vertical animation
void LootDrop::update(float deltaTime) {
    if (m_isCollected) return;

    m_bobTimer += deltaTime * 4.0f;
    float offsetY = std::sin(m_bobTimer) * 4.0f;
    m_position.y = m_basePosition.y + offsetY;
    m_sprite.setPosition(m_position);
}

void LootDrop::render(sf::RenderTarget& target) const {
    if (!m_isCollected) {
        target.draw(m_sprite);
    }
}

sf::FloatRect LootDrop::getGlobalBounds() const {
    return m_sprite.getGlobalBounds();
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: collect() handles adding gold or items to the Player's
// inventory, managing success and inventory capacity.
// ============================================================
bool LootDrop::collect(Player& player) {
    if (m_isCollected) return false;

    if (m_isGold) {
        player.getInventory().addGold(m_goldValue);
        m_isCollected = true;
        return true;
    } else if (m_item) {
        bool added = player.getInventory().addItem(m_item, m_quantity);
        if (added) {
            m_isCollected = true;
            return true;
        }
    }
    return false;
}

