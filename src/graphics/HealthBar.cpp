#include "graphics/HealthBar.hpp"
#include <algorithm>
#include <cmath>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (MEMBER INITIALIZATION LIST)
// Why: Initializes all sub-shapes, geometry, and styling cleanly.
// ============================================================
HealthBar::HealthBar(sf::Vector2f position,
                     sf::Vector2f size,
                     const std::string& label,
                     bool showText)
    : m_position(position)
    , m_size(size)
    , m_healthRatio(1.0f)
    , m_fillColor(80, 225, 100)
    , m_label(label)
    , m_showText(showText) {

    // Background plate (dark slate)
    m_background.setPosition(m_position);
    m_background.setSize(m_size);
    m_background.setFillColor(sf::Color(25, 20, 35, 220));

    // Outer border (stone/gold outline)
    m_border.setPosition(m_position);
    m_border.setSize(m_size);
    m_border.setFillColor(sf::Color::Transparent);
    m_border.setOutlineThickness(2.0f);
    m_border.setOutlineColor(sf::Color(160, 150, 130, 240));

    // Fill bar
    m_fill.setPosition(m_position);
    m_fill.setSize(m_size);
    m_fill.setFillColor(m_fillColor);
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION & DATA INVARIANTS
// Why: Enforces 0.0 <= ratio <= 1.0 and applies dynamic color coding:
// - Above 50%: Healthy emerald green
// - 25% to 50%: Warning amber/yellow
// - Below 25%: Critical bright red
// ============================================================
void HealthBar::update(int currentHealth, int maxHealth) {
    if (maxHealth <= 0) maxHealth = 1;
    float rawRatio = static_cast<float>(currentHealth) / static_cast<float>(maxHealth);
    m_healthRatio = std::max(0.0f, std::min(1.0f, rawRatio));

    m_fill.setSize(sf::Vector2f(m_size.x * m_healthRatio, m_size.y));

    // Dynamic color gradient based on health threshold
    if (m_healthRatio > 0.50f) {
        m_fillColor = sf::Color(70, 220, 100);   // Emerald green
    } else if (m_healthRatio > 0.25f) {
        m_fillColor = sf::Color(240, 180, 40);   // Amber yellow
    } else {
        m_fillColor = sf::Color(240, 50, 60);    // Critical crimson red
    }
    m_fill.setFillColor(m_fillColor);
}

void HealthBar::setPosition(sf::Vector2f pos) {
    m_position = pos;
    m_background.setPosition(m_position);
    m_border.setPosition(m_position);
    m_fill.setPosition(m_position);
}

// ============================================================
// OOP CONCEPT: CONST-CORRECTNESS
// Why: render() purely draws graphics to the target surface without
// modifying any internal state.
// ============================================================
void HealthBar::render(sf::RenderTarget& target, const sf::Font* font) const {
    // 1. Draw background and filled portion
    target.draw(m_background);
    if (m_healthRatio > 0.0f) {
        target.draw(m_fill);
    }
    target.draw(m_border);

    // 2. Render text label and percentage overlay if font is provided
    if (m_showText && font) {
        sf::Text text;
        text.setFont(*font);
        int percentage = static_cast<int>(std::round(m_healthRatio * 100.0f));
        text.setString(m_label + " " + std::to_string(percentage) + "%");
        text.setCharacterSize(static_cast<unsigned int>(m_size.y * 0.75f));
        text.setFillColor(sf::Color::White);
        text.setOutlineColor(sf::Color(10, 10, 15, 230));
        text.setOutlineThickness(1.0f);

        // Center text within the health bar
        sf::FloatRect bounds = text.getLocalBounds();
        text.setPosition(m_position.x + (m_size.x - bounds.width) * 0.5f - bounds.left,
                          m_position.y + (m_size.y - bounds.height) * 0.5f - bounds.top - 1.0f);
        target.draw(text);
    }
}

