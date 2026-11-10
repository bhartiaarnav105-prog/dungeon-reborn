#include "interactables/KeyPickup.hpp"
#include "entities/Player.hpp"
#include "core/SoundManager.hpp"
#include <cmath>
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Initializes Interactable base class with tag "KeyPickup",
// and configures sprite scale, origin, and starting world position.
// ============================================================
KeyPickup::KeyPickup(sf::Vector2f position,
                     std::shared_ptr<Key> key,
                     const sf::Texture& tileset)
    : Interactable("KeyPickup")
    , m_position(position)
    , m_basePosition(position)
    , m_key(std::move(key))
    , m_sprite(tileset, m_key ? m_key->getIconRect() : sf::IntRect(288, 32, 16, 16))
    , m_bobTimer(0.0f)
    , m_isCollected(false) {

    m_sprite.setOrigin(8.0f, 8.0f);
    m_sprite.setScale(2.5f, 2.5f);
    m_sprite.setPosition(m_position);
    m_sprite.setColor(sf::Color(255, 230, 100)); // Golden sheen
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: Overrides Interactable::interact(player). Transfers the owned
// Key into the Player's Inventory composition container.
// ============================================================
void KeyPickup::interact(Player& player) {
    if (m_isCollected || !m_key) return;

    bool added = player.getInventory().addItem(m_key, 1);
    if (added) {
        m_isCollected = true;
        m_active = false;
        std::cout << "[KeyPickup] Player collected key: " << m_key->getName()
                  << " (Unlocks: " << m_key->getTargetDoorId() << ")!\n";
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: update() adds gentle sinusoidal bobbing animation and checks
// proximity trigger with the player.
// ============================================================
void KeyPickup::update(float deltaTime, Player& player, SoundManager* soundMgr) {
    if (m_isCollected) return;

    m_bobTimer += deltaTime * 4.0f;
    float offsetY = std::sin(m_bobTimer) * 5.0f;
    m_sprite.setPosition(m_basePosition.x, m_basePosition.y + offsetY);

    // Auto-pickup when player touches key
    if (player.isAlive() && getBounds().intersects(player.getGlobalBounds())) {
        interact(player);
        if (m_isCollected && soundMgr) {
            soundMgr->playSound("coin");
        }
    }
}

void KeyPickup::render(sf::RenderTarget& target) const {
    if (!m_isCollected) {
        target.draw(m_sprite);
    }
}

sf::FloatRect KeyPickup::getBounds() const {
    return m_sprite.getGlobalBounds();
}

std::string KeyPickup::getPrompt() const {
    if (m_isCollected || !m_key) return "";
    return "[E] Collect " + m_key->getName();
}

