#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class TileMap;

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS
// Why: CollisionSystem is a stateless utility class that groups
// related collision functions under a common namespace-like
// scope. Making it a class (rather than free functions) allows
// it to be extended with state if needed without breaking callers.
//
// OOP CONCEPT: ABSTRACTION
// Why: Callers supply a desired movement vector and receive a safe,
// wall-corrected movement vector back — all SFML rect intersection
// math and axis-separation logic is hidden inside this class.
//
// OOP CONCEPT: ENCAPSULATION
// Why: All collision math is private to this class. The public
// interface is minimal: one main resolver and two utility queries.
// ============================================================
class CollisionSystem {
public:
    // ============================================================
    // OOP CONCEPT: ABSTRACTION
    // Why: resolveCollision is the primary public entry point.
    // It applies axis-separated AABB collision resolution:
    //   1. Move along X, resolve all X-axis penetrations.
    //   2. Move along Y, resolve all Y-axis penetrations.
    // This prevents corner-sticking artifacts common with simultaneous
    // axis collision resolution.
    //
    // Parameters:
    //   bounds      - Current AABB of the moving character.
    //   desiredMove - How far the character wants to move this frame.
    //   solidRects  - All solid tile rectangles for the current room.
    //
    // Returns the actual safe movement vector (may be shorter than desired).
    // ============================================================
    static sf::Vector2f resolveCollision(sf::FloatRect          bounds,
                                         sf::Vector2f           desiredMove,
                                         const std::vector<sf::FloatRect>& solidRects);

    // ============================================================
    // OOP CONCEPT: ABSTRACTION & EFFICIENCY
    // Why: resolveTileCollision performs direct grid coordinate lookups,
    // testing only tiles immediately surrounding the character bounding box.
    // ============================================================
    static sf::Vector2f resolveTileCollision(sf::FloatRect    bounds,
                                             sf::Vector2f     desiredMove,
                                             const TileMap&   tileMap);

    // ============================================================
    // OOP CONCEPT: ENCAPSULATION (UTILITY HELPERS)
    // Why: isOverlapping exposes the overlap test for other systems
    // (e.g., door trigger detection) without duplicating the math.
    // ============================================================
    static bool isOverlapping(const sf::FloatRect& a, const sf::FloatRect& b);

    // Returns all rects in 'candidates' that overlap 'bounds'.
    static std::vector<sf::FloatRect> getOverlapping(
        const sf::FloatRect&              bounds,
        const std::vector<sf::FloatRect>& candidates);

private:
    // ============================================================
    // OOP CONCEPT: ENCAPSULATION (PRIVATE HELPERS)
    // Why: resolveAxis is an internal helper that resolves penetration
    // along a single axis. It is not part of the public API.
    // ============================================================
    static float resolveAxis(float pos,       // position along axis
                             float size,      // extent along axis
                             float delta,     // desired movement along axis
                             const std::vector<sf::FloatRect>& solidRects,
                             bool  isXAxis);  // true = resolving X, false = Y
};

