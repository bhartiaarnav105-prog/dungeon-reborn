#include "dungeon/CameraController.hpp"
#include <algorithm>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALISER LIST)
// Why: All members are set directly in the initialiser list so
// the object is fully valid before the constructor body runs.
// m_bounds starts as the entire float plane and is tightened
// by setBounds() when a room is loaded.
// ============================================================
CameraController::CameraController(unsigned int viewWidth,
                                   unsigned int viewHeight,
                                   float        lerpSpeed)
    : m_view(sf::FloatRect(0.f, 0.f,
                           static_cast<float>(viewWidth),
                           static_cast<float>(viewHeight)))
    , m_lerpSpeed(lerpSpeed)
    , m_bounds(0.f, 0.f,
               static_cast<float>(viewWidth),
               static_cast<float>(viewHeight))
{}

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: Smooth camera follow using linear interpolation (lerp).
// The maths is hidden from callers; they just call update().
//
// Algorithm:
//   new_centre = current_centre + (target - current_centre) * lerpSpeed * dt
//   Then clamp so the view edges never exceed the room bounds.
//   If a room dimension is smaller than or equal to the view size,
//   the camera is centered on that dimension to prevent clipping.
// ============================================================
void CameraController::update(float deltaTime, const sf::Vector2f& targetPosition) {
    sf::Vector2f current = m_view.getCenter();
    sf::Vector2f size    = m_view.getSize();

    // Lerp towards target
    float alpha = m_lerpSpeed * deltaTime;
    if (alpha > 1.f) alpha = 1.f;

    sf::Vector2f newCenter;
    newCenter.x = current.x + (targetPosition.x - current.x) * alpha;
    newCenter.y = current.y + (targetPosition.y - current.y) * alpha;

    // Clamp so view stays within room bounds
    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;

    if (m_bounds.width <= size.x) {
        newCenter.x = m_bounds.left + m_bounds.width * 0.5f;
    } else {
        newCenter.x = std::max(m_bounds.left + halfW,
                      std::min(newCenter.x, m_bounds.left + m_bounds.width  - halfW));
    }

    if (m_bounds.height <= size.y) {
        newCenter.y = m_bounds.top + m_bounds.height * 0.5f;
    } else {
        newCenter.y = std::max(m_bounds.top  + halfH,
                      std::min(newCenter.y, m_bounds.top  + m_bounds.height - halfH));
    }

    m_view.setCenter(newCenter);
}

void CameraController::setBounds(const sf::FloatRect& roomBounds) {
    m_bounds = roomBounds;
}

void CameraController::applyTo(sf::RenderWindow& window) const {
    window.setView(m_view);
}

void CameraController::resetTo(const sf::Vector2f& position) {
    sf::Vector2f size = m_view.getSize();
    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;
    sf::Vector2f c = position;

    if (m_bounds.width <= size.x) {
        c.x = m_bounds.left + m_bounds.width * 0.5f;
    } else {
        c.x = std::max(m_bounds.left + halfW, std::min(c.x, m_bounds.left + m_bounds.width - halfW));
    }

    if (m_bounds.height <= size.y) {
        c.y = m_bounds.top + m_bounds.height * 0.5f;
    } else {
        c.y = std::max(m_bounds.top + halfH, std::min(c.y, m_bounds.top + m_bounds.height - halfH));
    }

    m_view.setCenter(c);
}

const sf::View& CameraController::getView() const {
    return m_view;
}

