#include "dungeon/Minimap.hpp"
#include "dungeon/Room.hpp"
#include <cmath>

// ============================================================
// OOP CONCEPT: CONSTRUCTOR & MEMBER INITIALIZATION LIST
// Why: Initializes minimap bounds, background plate styling, and sets Room 0
// as immediately discovered upon entering the dungeon.
// ============================================================
Minimap::Minimap(sf::Vector2f position, sf::Vector2f size)
    : m_position(position)
    , m_size(size)
    , m_pulseTimer(0.0f)
    , m_showPlayerBlip(true) {

    m_background.setSize(m_size);
    m_background.setPosition(m_position);
    m_background.setFillColor(sf::Color(18, 20, 30, 215));

    m_border.setSize(m_size);
    m_border.setPosition(m_position);
    m_border.setFillColor(sf::Color::Transparent);
    m_border.setOutlineThickness(2.0f);
    m_border.setOutlineColor(sf::Color(190, 160, 90, 230));

    // Room 0 (Entrance Crypt) is revealed by default
    m_discoveredRooms.insert(0);
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: Room discovery state is private. The Dungeon signals discovery
// when the player enters a room or uses a map scroll.
// ============================================================
void Minimap::discoverRoom(int roomId) {
    m_discoveredRooms.insert(roomId);
}

bool Minimap::isRoomDiscovered(int roomId) const {
    return m_discoveredRooms.find(roomId) != m_discoveredRooms.end();
}

void Minimap::setPosition(sf::Vector2f pos) {
    m_position = pos;
    m_background.setPosition(m_position);
    m_border.setPosition(m_position);
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: Internal timer and blip oscillations are encapsulated within Minimap.
// ============================================================
void Minimap::update(float deltaTime, int currentRoomId) {
    discoverRoom(currentRoomId);

    m_pulseTimer += deltaTime;
    if (m_pulseTimer >= 0.35f) {
        m_pulseTimer = 0.0f;
        m_showPlayerBlip = !m_showPlayerBlip;
    }
}

// ============================================================
// OOP CONCEPT: CONST-CORRECTNESS & ENCAPSULATION
// Why: Minimap rendering queries game state without mutating it.
// Dynamically renders the 5-room 2D topological graph:
//   Room 3 (Forgotten Armory) [North]
//   Room 0 (Ancient Crossroads) [Center] <-> Room 2 (Goblin Warren) <-> Room 4 (Crypt)
//   Room 1 (Demon's Sanctum) [South]
// Demonstrates fog-of-war silhouettes, chamber type colors, and the player blip.
// ============================================================
void Minimap::render(sf::RenderTarget& target,
                     int currentRoomId,
                     const std::vector<Room>& rooms,
                     sf::Vector2f playerPos) const {
    // 1. Draw frame and background
    target.draw(m_background);
    target.draw(m_border);

    if (rooms.empty()) return;

    // Room box dimensions and grid coordinates relative to Minimap panel
    const float roomBoxW = 42.0f;
    const float roomBoxH = 24.0f;
    const float startX   = m_position.x + 4.0f;
    const float startY   = m_position.y + 4.0f;

    auto getRoomBoxPos = [&](int rId) -> sf::Vector2f {
        switch (rId) {
            case 3: return sf::Vector2f(startX + 10.0f,  startY + 6.0f);  // Armory (North)
            case 0: return sf::Vector2f(startX + 10.0f,  startY + 38.0f); // Entry (Center)
            case 1: return sf::Vector2f(startX + 10.0f,  startY + 70.0f); // Boss (South)
            case 2: return sf::Vector2f(startX + 65.0f,  startY + 38.0f); // Warren (East)
            case 4: return sf::Vector2f(startX + 120.0f, startY + 38.0f); // Crypt (Far East)
            default: return sf::Vector2f(startX, startY);
        }
    };

    // Draw corridor links between adjacent chambers
    auto drawCorridor = [&](sf::Vector2f pos, sf::Vector2f sz, bool lit) {
        sf::RectangleShape corr(sz);
        corr.setPosition(pos);
        corr.setFillColor(lit ? sf::Color(80, 85, 110, 200) : sf::Color(30, 32, 45, 120));
        target.draw(corr);
    };

    // North vertical connector: Room 3 <-> Room 0
    drawCorridor(sf::Vector2f(startX + 28.0f, startY + 30.0f), sf::Vector2f(6.0f, 8.0f),
                 isRoomDiscovered(3) && isRoomDiscovered(0));

    // South vertical connector: Room 0 <-> Room 1
    drawCorridor(sf::Vector2f(startX + 28.0f, startY + 62.0f), sf::Vector2f(6.0f, 8.0f),
                 isRoomDiscovered(0) && isRoomDiscovered(1));

    // East horizontal connector: Room 0 <-> Room 2
    drawCorridor(sf::Vector2f(startX + 52.0f, startY + 47.0f), sf::Vector2f(13.0f, 6.0f),
                 isRoomDiscovered(0) && isRoomDiscovered(2));

    // Far East horizontal connector: Room 2 <-> Room 4
    drawCorridor(sf::Vector2f(startX + 107.0f, startY + 47.0f), sf::Vector2f(13.0f, 6.0f),
                 isRoomDiscovered(2) && isRoomDiscovered(4));

    // Render each room chamber
    for (size_t i = 0; i < rooms.size(); ++i) {
        int rId = rooms[i].getId();
        bool discovered = isRoomDiscovered(rId);
        sf::Vector2f boxPos = getRoomBoxPos(rId);

        sf::RectangleShape roomBox(sf::Vector2f(roomBoxW, roomBoxH));
        roomBox.setPosition(boxPos);

        if (discovered) {
            if (rooms[i].getType() == RoomType::Boss) {
                roomBox.setFillColor(sf::Color(70, 20, 25, 230));
                roomBox.setOutlineColor(sf::Color(220, 60, 70));
            } else if (rooms[i].getType() == RoomType::Treasure) {
                roomBox.setFillColor(sf::Color(65, 55, 25, 230));
                roomBox.setOutlineColor(sf::Color(220, 190, 60));
            } else {
                roomBox.setFillColor(sf::Color(35, 45, 65, 230));
                roomBox.setOutlineColor(sf::Color(100, 130, 180));
            }
            roomBox.setOutlineThickness(1.5f);

            // Highlight active room with gold border
            if (rId == currentRoomId) {
                roomBox.setOutlineColor(sf::Color(255, 225, 80));
                roomBox.setOutlineThickness(2.0f);
            }
        } else {
            // Fog of war: dim silhouette
            roomBox.setFillColor(sf::Color(12, 14, 20, 180));
            roomBox.setOutlineColor(sf::Color(45, 50, 65, 140));
            roomBox.setOutlineThickness(1.0f);
        }

        target.draw(roomBox);

        // Render player indicator blip if this is the active room
        if (rId == currentRoomId && m_showPlayerBlip) {
            float roomPixelW = rooms[i].getTileMap().getPixelWidth();
            float roomPixelH = rooms[i].getTileMap().getPixelHeight();
            if (roomPixelW <= 0.0f) roomPixelW = 1.0f;
            if (roomPixelH <= 0.0f) roomPixelH = 1.0f;

            float normX = std::max(0.0f, std::min(1.0f, playerPos.x / roomPixelW));
            float normY = std::max(0.0f, std::min(1.0f, playerPos.y / roomPixelH));

            sf::CircleShape playerDot(3.0f);
            playerDot.setOrigin(3.0f, 3.0f);
            playerDot.setPosition(boxPos.x + 3.0f + normX * (roomBoxW - 6.0f),
                                  boxPos.y + 3.0f + normY * (roomBoxH - 6.0f));
            playerDot.setFillColor(sf::Color(80, 255, 140));
            playerDot.setOutlineColor(sf::Color::White);
            playerDot.setOutlineThickness(1.0f);
            target.draw(playerDot);
        }
    }
}
