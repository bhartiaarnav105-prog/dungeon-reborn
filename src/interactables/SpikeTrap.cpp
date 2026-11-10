#include "interactables/SpikeTrap.hpp"
#include "entities/Player.hpp"
#include "core/SoundManager.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (MULTI-LEVEL INITIALIZATION)
// Why: Invokes Trap("SpikeTrap", damage, cooldown).
// Configures retracted and extended spike textures and world bounds.
// ============================================================
SpikeTrap::SpikeTrap(sf::Vector2f position,
                     const sf::Texture& tileset,
                     int damage,
                     float cooldown)
    : Trap("SpikeTrap", damage, cooldown)
    , m_position(position)
    , m_bounds(position.x, position.y, 48.0f, 48.0f)
    , m_sprite(tileset, sf::IntRect(128, 96, 16, 16))
    , m_retractedRect(128, 96, 16, 16)
    , m_extendedRect(144, 96, 16, 16)
    , m_springDuration(0.6f)
    , m_springTimer(0.0f) {

    m_sprite.setPosition(position);
    m_sprite.setScale(3.0f, 3.0f); // 16x16 -> 48x48
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: When stepped on, trigger() applies puncture damage directly
// through the Player's encapsulated takeDamage() method.
// ============================================================
void SpikeTrap::trigger(Player& player) {
    if (m_isTriggered || m_cooldownTimer > 0.0f) return;

    m_isTriggered = true;
    m_cooldownTimer = m_cooldown;
    m_springTimer = m_springDuration;
    m_sprite.setTextureRect(m_extendedRect);

    player.takeDamage(m_damage);
    std::cout << "[SpikeTrap] *SHH-KLANG!* Steel spikes impaled player for "
              << m_damage << " damage! (Player HP: " << player.getHealth() << ")\n";
}

void SpikeTrap::update(float deltaTime, Player& player, SoundManager* soundMgr) {
    // Call base class Trap cooldown management
    Trap::update(deltaTime, player, soundMgr);

    // Manage visual extended spring timer
    if (m_springTimer > 0.0f) {
        m_springTimer -= deltaTime;
        if (m_springTimer <= 0.0f) {
            m_springTimer = 0.0f;
            m_sprite.setTextureRect(m_retractedRect);
        }
    }

    // Step-on hazard detection: player collision bounds overlap trap
    if (player.isAlive() && m_cooldownTimer <= 0.0f) {
        sf::FloatRect pBounds = player.getCollisionBounds();
        if (m_bounds.intersects(pBounds)) {
            trigger(player);
            if (soundMgr) {
                soundMgr->playSound("hurt");
            }
        }
    }
}

void SpikeTrap::render(sf::RenderTarget& target) const {
    target.draw(m_sprite);

    // Subtle red hazard glow when active spikes are out
    if (m_springTimer > 0.0f) {
        sf::RectangleShape bloodGlow(sf::Vector2f(m_bounds.width, m_bounds.height));
        bloodGlow.setPosition(m_bounds.left, m_bounds.top);
        bloodGlow.setFillColor(sf::Color(255, 30, 30, 60));
        target.draw(bloodGlow);
    }
}

