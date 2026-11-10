#pragma once

#include <SFML/Graphics.hpp>

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: CameraController encapsulates all SFML view logic —
// smooth follow, boundary clamping, and view application.
// Without this class, all of that logic would be scattered
// through Game::update() and Game::render().
//
// OOP CONCEPT: ENCAPSULATION
// Why: The sf::View object (m_view) and lerp speed are private.
// External code only calls update() and applyTo(); it cannot
// accidentally resize or misalign the view.
// ============================================================
class CameraController {
private:
    sf::View      m_view;
    float         m_lerpSpeed;  // higher = snappier follow (units: 1/s)
    sf::FloatRect m_bounds;     // world-space clamping limits (room pixel rect)

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Initialises the view to the window size and sets a
    // sensible default lerp speed. No manual reset is required
    // before first use.
    // ============================================================
    CameraController(unsigned int viewWidth,
                     unsigned int viewHeight,
                     float        lerpSpeed = 5.0f);

    // ============================================================
    // OOP CONCEPT: ABSTRACTION
    // Why: update() hides the lerp formula and clamp math behind
    // a simple "target the player" interface.
    // ============================================================
    void update(float deltaTime, const sf::Vector2f& targetPosition);

    // Set new clamping bounds when transitioning to a new room.
    void setBounds(const sf::FloatRect& roomBounds);

    // Apply the computed view to the render window.
    void applyTo(sf::RenderWindow& window) const;

    // Snap the camera instantly to a position (used on room transitions).
    void resetTo(const sf::Vector2f& position);

    // ============================================================
    // OOP CONCEPT: CONST-CORRECTNESS
    // Why: getView() does not modify state; it returns a const ref.
    // ============================================================
    const sf::View& getView() const;
};

