#include "dungeon/CollisionSystem.hpp"
#include "dungeon/TileMap.hpp"
#include <algorithm>
#include <cmath>

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: isOverlapping wraps SFML intersects() under a named,
// intent-revealing function so call sites read clearly.
// ============================================================
bool CollisionSystem::isOverlapping(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.intersects(b);
}

std::vector<sf::FloatRect> CollisionSystem::getOverlapping(
    const sf::FloatRect&              bounds,
    const std::vector<sf::FloatRect>& candidates)
{
    std::vector<sf::FloatRect> result;
    for (const auto& rect : candidates) {
        if (bounds.intersects(rect)) result.push_back(rect);
    }
    return result;
}

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: resolveCollision is the high-level entry point that hides
// the axis-separation algorithm from callers.
//
// Algorithm — axis-separated AABB (prevents corner sticking):
//   1. Apply desiredMove.x, check every solid rect for X overlap,
//      push the character back along X until clear.
//   2. Apply the resulting desiredMove.y, check every solid rect
//      for Y overlap, push back along Y until clear.
// ============================================================
sf::Vector2f CollisionSystem::resolveCollision(
    sf::FloatRect                     bounds,
    sf::Vector2f                      desiredMove,
    const std::vector<sf::FloatRect>& solidRects)
{
    sf::Vector2f safeMove(0.f, 0.f);

    // ----- Resolve X -----
    if (std::abs(desiredMove.x) > 0.f) {
        sf::FloatRect movedX = bounds;
        movedX.left += desiredMove.x;

        for (const auto& solid : solidRects) {
            if (!movedX.intersects(solid)) continue;

            // Calculate penetration depth along X
            if (desiredMove.x > 0.f) {
                // Moving right: right edge of movedX past solid's left edge
                float penetration = (movedX.left + movedX.width) - solid.left;
                movedX.left -= penetration;
            } else {
                // Moving left: left edge of movedX past solid's right edge
                float penetration = (solid.left + solid.width) - movedX.left;
                movedX.left += penetration;
            }
        }
        safeMove.x  = movedX.left - bounds.left;
        bounds.left = movedX.left; // commit X so Y test uses correct position
    }

    // ----- Resolve Y -----
    if (std::abs(desiredMove.y) > 0.f) {
        sf::FloatRect movedY = bounds;
        movedY.top += desiredMove.y;

        for (const auto& solid : solidRects) {
            if (!movedY.intersects(solid)) continue;

            if (desiredMove.y > 0.f) {
                // Moving down: bottom edge past solid's top edge
                float penetration = (movedY.top + movedY.height) - solid.top;
                movedY.top -= penetration;
            } else {
                // Moving up: top edge past solid's bottom edge
                float penetration = (solid.top + solid.height) - movedY.top;
                movedY.top += penetration;
            }
        }
        safeMove.y = movedY.top - bounds.top;
    }

    return safeMove;
}

// ============================================================
// OOP CONCEPT: ABSTRACTION & EFFICIENCY
// Why: resolveTileCollision performs direct grid coordinate lookups,
// testing only the specific tile coordinates that intersect the character's
// movement delta rather than iterating all walls in the room.
// ============================================================
sf::Vector2f CollisionSystem::resolveTileCollision(
    sf::FloatRect    bounds,
    sf::Vector2f     desiredMove,
    const TileMap&   tileMap)
{
    sf::Vector2f safeMove(0.f, 0.f);
    const float tileSize = tileMap.getTileSize();

    // ----- Resolve X -----
    if (std::abs(desiredMove.x) > 0.f) {
        sf::FloatRect movedX = bounds;
        movedX.left += desiredMove.x;

        int minCol = static_cast<int>(std::floor(movedX.left / tileSize));
        int maxCol = static_cast<int>(std::floor((movedX.left + movedX.width - 0.001f) / tileSize));
        int minRow = static_cast<int>(std::floor(movedX.top / tileSize));
        int maxRow = static_cast<int>(std::floor((movedX.top + movedX.height - 0.001f) / tileSize));

        for (int r = minRow; r <= maxRow; ++r) {
            for (int c = minCol; c <= maxCol; ++c) {
                if (tileMap.isSolid(c, r)) {
                    float tileLeft = c * tileSize;
                    float tileRight = tileLeft + tileSize;
                    if (desiredMove.x > 0.f) {
                        float penetration = (movedX.left + movedX.width) - tileLeft;
                        if (penetration > 0.f) movedX.left -= penetration;
                    } else if (desiredMove.x < 0.f) {
                        float penetration = tileRight - movedX.left;
                        if (penetration > 0.f) movedX.left += penetration;
                    }
                }
            }
        }
        safeMove.x = movedX.left - bounds.left;
        if (desiredMove.x > 0.f && safeMove.x < 0.f) safeMove.x = 0.f;
        if (desiredMove.x < 0.f && safeMove.x > 0.f) safeMove.x = 0.f;
        bounds.left = movedX.left;
    }

    // ----- Resolve Y -----
    if (std::abs(desiredMove.y) > 0.f) {
        sf::FloatRect movedY = bounds;
        movedY.top += desiredMove.y;

        int minCol = static_cast<int>(std::floor(bounds.left / tileSize));
        int maxCol = static_cast<int>(std::floor((bounds.left + bounds.width - 0.001f) / tileSize));
        int minRow = static_cast<int>(std::floor(movedY.top / tileSize));
        int maxRow = static_cast<int>(std::floor((movedY.top + movedY.height - 0.001f) / tileSize));

        for (int r = minRow; r <= maxRow; ++r) {
            for (int c = minCol; c <= maxCol; ++c) {
                if (tileMap.isSolid(c, r)) {
                    float tileTop = r * tileSize;
                    float tileBottom = tileTop + tileSize;
                    if (desiredMove.y > 0.f) {
                        float penetration = (movedY.top + movedY.height) - tileTop;
                        if (penetration > 0.f) movedY.top -= penetration;
                    } else if (desiredMove.y < 0.f) {
                        float penetration = tileBottom - movedY.top;
                        if (penetration > 0.f) movedY.top += penetration;
                    }
                }
            }
        }
        safeMove.y = movedY.top - bounds.top;
        if (desiredMove.y > 0.f && safeMove.y < 0.f) safeMove.y = 0.f;
        if (desiredMove.y < 0.f && safeMove.y > 0.f) safeMove.y = 0.f;
    }

    return safeMove;
}
