#pragma once

#include "interactables/Trap.hpp"
#include <SFML/Graphics.hpp>

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: SpikeTrap IS-A Trap (which IS-AN Interactable).
// This demonstrates multi-level inheritance:
// Interactable -> Trap -> SpikeTrap
//
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: SpikeTrap overrides trigger(), update(), render(), and getBounds().
// Calls to interact() or update() on an Interactable* dispatch dynamically
// to SpikeTrap's concrete implementation.
// ============================================================
class SpikeTrap : public Trap {
private:
    sf::Vector2f  m_position;
    sf::FloatRect m_bounds;
    sf::Sprite    m_sprite;
    sf::IntRect   m_retractedRect;
    sf::IntRect   m_extendedRect;
    float         m_springDuration;
    float         m_springTimer;

public:
    SpikeTrap(sf::Vector2f position,
              const sf::Texture& tileset,
              int damage = 10,
              float cooldown = 1.2f);

    ~SpikeTrap() override = default;

    // ============================================================
    // OOP CONCEPT: METHOD OVERRIDING & RUNTIME POLYMORPHISM
    // Why: Implements Trap::trigger(Player& player). Inflicts puncture
    // damage, springs the spikes visually, and enters cooldown.
    // ============================================================
    void trigger(Player& player) override;

    void update(float deltaTime, Player& player, SoundManager* soundMgr = nullptr) override;

    void render(sf::RenderTarget& target) const override;

    sf::FloatRect getBounds() const override { return m_bounds; }

    std::string getPrompt() const override { return "[Hazard] Watch out for spikes!"; }
};

