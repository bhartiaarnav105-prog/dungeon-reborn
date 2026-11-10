#pragma once

#include "interactables/Trap.hpp"
#include <SFML/Graphics.hpp>

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: FireTrap IS-A Trap (which IS-AN Interactable).
// Specializes hazard behavior with periodic time-based flame eruptions.
//
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: Overrides trigger(), update(), render(), getBounds().
// ============================================================
class FireTrap : public Trap {
private:
    sf::Vector2f  m_position;
    sf::FloatRect m_bounds;
    sf::Sprite    m_sprite;
    float         m_cycleTimer;
    float         m_dormantTime;
    float         m_eruptTime;
    bool          m_isErupting;
    float         m_tickDamageTimer;

public:
    FireTrap(sf::Vector2f position,
             const sf::Texture& tileset,
             int damage = 12,
             float dormantTime = 2.0f,
             float eruptTime = 1.5f);

    ~FireTrap() override = default;

    // ============================================================
    // OOP CONCEPT: METHOD OVERRIDING & RUNTIME POLYMORPHISM
    // Why: Overrides Trap::trigger(Player& player) to apply fiery burn damage.
    // ============================================================
    void trigger(Player& player) override;

    void update(float deltaTime, Player& player, SoundManager* soundMgr = nullptr) override;

    void render(sf::RenderTarget& target) const override;

    sf::FloatRect getBounds() const override { return m_bounds; }

    bool isErupting() const { return m_isErupting; }

    std::string getPrompt() const override {
        return m_isErupting ? "[Danger] Blazing Inferno!" : "[Hazard] Fire Grate";
    }
};

