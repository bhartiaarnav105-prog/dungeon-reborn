#pragma once

#include "items/Item.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class Player;

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: LootDrop is a dedicated class representing an item or gold
// coin physically present on the dungeon floor.
//
// OOP CONCEPT: ENCAPSULATION
// Why: World position, bobbing animation timers, and collection
// states are private. Interaction occurs through collect().
// ============================================================
class LootDrop {
private:
    sf::Vector2f          m_position;
    sf::Vector2f          m_basePosition;
    std::shared_ptr<Item> m_item;
    int                   m_quantity;
    int                   m_goldValue;
    bool                  m_isGold;
    bool                  m_isCollected;
    float                 m_bobTimer;
    sf::Sprite            m_sprite;

public:
    // Constructor for Item drops (Weapons, Potions)
    LootDrop(sf::Vector2f position,
             std::shared_ptr<Item> item,
             int quantity,
             const sf::Texture& tileset);

    // Constructor for Gold drops
    LootDrop(sf::Vector2f position,
             int goldAmount,
             const sf::Texture& tileset);

    ~LootDrop() = default;

    // Movement & bobbing animation update
    void update(float deltaTime);
    void render(sf::RenderTarget& target) const;

    // Collision & collection
    sf::FloatRect getGlobalBounds() const;
    bool isCollected() const { return m_isCollected; }
    bool collect(Player& player);

    // Queries
    bool isGold() const { return m_isGold; }
    int  getGoldValue() const { return m_goldValue; }
    const Item* getItem() const { return m_item.get(); }
    int  getQuantity() const { return m_quantity; }
};

