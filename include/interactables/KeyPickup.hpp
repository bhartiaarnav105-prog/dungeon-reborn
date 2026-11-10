#pragma once

#include "interactables/Interactable.hpp"
#include "items/Key.hpp"
#include <memory>

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: KeyPickup IS-AN Interactable. It implements the Interactable
// abstract interface (interact, render, getBounds).
//
// OOP CONCEPT: COMPOSITION
// Why: KeyPickup HAS-A std::shared_ptr<Key>. It holds and guards the
// concrete Key item until the player claims it.
//
// OOP CONCEPT: ENCAPSULATION
// Why: Visual animation parameters (bobbing timer, original position)
// and collected state are strictly private.
// ============================================================
class KeyPickup : public Interactable {
private:
    sf::Vector2f          m_position;
    sf::Vector2f          m_basePosition;
    std::shared_ptr<Key>  m_key;
    sf::Sprite            m_sprite;
    float                 m_bobTimer;
    bool                  m_isCollected;

public:
    KeyPickup(sf::Vector2f position,
              std::shared_ptr<Key> key,
              const sf::Texture& tileset);

    ~KeyPickup() override = default;

    // ============================================================
    // OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
    // Why: When the player interacts or collides with the key pickup,
    // interact() deposits the Key into the player's inventory and
    // deactivates the world pickup entity.
    // ============================================================
    void interact(Player& player) override;

    void update(float deltaTime, Player& player, SoundManager* soundMgr = nullptr) override;

    void render(sf::RenderTarget& target) const override;

    sf::FloatRect getBounds() const override;

    bool isActive() const override { return !m_isCollected; }

    std::string getPrompt() const override;

    const Key* getKey() const { return m_key.get(); }
};

