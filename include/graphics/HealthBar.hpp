#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS & ENCAPSULATION
// Why: HealthBar encapsulates the visual presentation, color gradient
// thresholds (healthy emerald, warning amber, critical crimson),
// border geometry, and text formatting of actor vitality.
// Subsystems (Player, Boss, Game HUD) interact via update() and render()
// without directly manipulating underlying SFML shapes or geometry.
// ============================================================
class HealthBar {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Internal shapes, colors, and calculated ratios are hidden
// to prevent arbitrary external tampering and ensure consistent rendering.
private:
    sf::Vector2f       m_position;
    sf::Vector2f       m_size;
    float              m_healthRatio;
    sf::RectangleShape m_background;
    sf::RectangleShape m_border;
    sf::RectangleShape m_fill;
    sf::Color          m_fillColor;
    std::string        m_label;
    bool               m_showText;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTOR
    // Why: Parameterized constructor initializes bar dimensions,
    // position, and default styling with member initializer list.
    // ============================================================
    HealthBar(sf::Vector2f position,
              sf::Vector2f size,
              const std::string& label = "HP",
              bool showText = true);

    ~HealthBar() = default;

    // ============================================================
    // OOP CONCEPT: ENCAPSULATION
    // Why: update() enforces health invariants (0.0 to 1.0 clamping)
    // and calculates dynamic color transitions without exposing shapes.
    // ============================================================
    void update(int currentHealth, int maxHealth);

    // ============================================================
    // OOP CONCEPT: CONST-CORRECTNESS
    // Why: render() draws the health bar without mutating its internal state.
    // ============================================================
    void render(sf::RenderTarget& target, const sf::Font* font = nullptr) const;

    // Setters & Getters
    void setPosition(sf::Vector2f pos);
    sf::Vector2f getPosition() const { return m_position; }
    sf::Vector2f getSize() const { return m_size; }
    float getHealthRatio() const { return m_healthRatio; }
};

