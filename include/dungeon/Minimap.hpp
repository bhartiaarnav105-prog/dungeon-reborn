#pragma once

#include <SFML/Graphics.hpp>
#include <set>
#include <vector>

// Forward declarations
class Room;

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS & ENCAPSULATION
// Why: Minimap encapsulates all navigation HUD state: discovered rooms,
// fog-of-war tracking, mini-radar coordinate mapping, and player blip
// blinking animation. The Dungeon interacts with Minimap strictly through
// its well-defined public interface.
// ============================================================
class Minimap {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: All rendering coordinates, discovered sets, and timer counters
// are hidden to prevent external modification and maintain data integrity.
private:
    sf::Vector2f          m_position;
    sf::Vector2f          m_size;
    sf::RectangleShape    m_background;
    sf::RectangleShape    m_border;
    std::set<int>         m_discoveredRooms;
    float                 m_pulseTimer;
    mutable bool          m_showPlayerBlip;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTOR
    // Why: Initializes minimap HUD dimensions, background styling,
    // and sets room 0 as initially discovered.
    // ============================================================
    explicit Minimap(sf::Vector2f position = sf::Vector2f(20.0f, 20.0f),
                     sf::Vector2f size = sf::Vector2f(170.0f, 95.0f));

    ~Minimap() = default;

    // Discovered rooms / Fog-of-war management
    void discoverRoom(int roomId);
    bool isRoomDiscovered(int roomId) const;
    const std::set<int>& getDiscoveredRooms() const { return m_discoveredRooms; }

    // Navigation updates
    void update(float deltaTime, int currentRoomId);

    // ============================================================
    // OOP CONCEPT: CONST-CORRECTNESS
    // Why: Rendering does not mutate the state of the Minimap.
    // ============================================================
    void render(sf::RenderTarget& target,
                int currentRoomId,
                const std::vector<Room>& rooms,
                sf::Vector2f playerPos) const;

    // Accessors
    sf::Vector2f getPosition() const { return m_position; }
    sf::Vector2f getSize() const { return m_size; }
    void setPosition(sf::Vector2f pos);
};

