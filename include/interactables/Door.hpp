#pragma once

#include "interactables/Interactable.hpp"
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS (SCOPED ENUM)
// Why: DoorState formalizes the operational lifecycle of a dungeon door
// (Locked, Unlocked, Open, Closed) preventing invalid states.
// ============================================================
enum class DoorState {
    Locked,
    Unlocked,
    Open,
    Closed
};

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Door IS-AN Interactable. It specializes Interactable by adding
// lock-and-key verification, room transit coordinates, and movement obstruction.
//
// OOP CONCEPT: ENCAPSULATION
// Why: Door strictly encapsulates its lock state (m_state), key requirements,
// and transition parameters. All state changes occur through verified methods
// like interact(), unlock(), and open().
// ============================================================
class Door : public Interactable {
private:
    DoorState     m_state;
    std::string   m_doorName;
    std::string   m_requiredKeyId;
    int           m_targetRoomId;
    sf::Vector2f  m_spawnOffset;
    sf::FloatRect m_bounds;
    bool          m_consumeKey;
    sf::Sprite    m_sprite;
    sf::IntRect   m_closedRect;
    sf::IntRect   m_openRect;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Parameterized constructor initializes door position, dimensions,
    // lock state, and destination room.
    // ============================================================
    Door(const std::string& doorName,
         sf::Vector2f position,
         sf::Vector2f size,
         int targetRoomId,
         sf::Vector2f spawnOffset,
         DoorState initialState = DoorState::Closed,
         const std::string& requiredKeyId = "",
         const sf::Texture* tileset = nullptr,
         bool consumeKey = false);

    ~Door() override = default;

    // ============================================================
    // OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
    // Why: Implements Interactable::interact(player). Checks player's
    // inventory for the matching key. If present, unlocks and opens the door;
    // otherwise informs the player that the door is securely locked.
    // ============================================================
    void interact(Player& player) override;

    void update(float deltaTime, Player& player, SoundManager* soundMgr = nullptr) override;

    void render(sf::RenderTarget& target) const override;

    sf::FloatRect getBounds() const override { return m_bounds; }

    // ============================================================
    // OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
    // Why: Overrides Interactable::isBlocking(). A locked or closed door
    // blocks character movement; an open door allows free passage.
    // ============================================================
    bool isBlocking() const override;

    // Door state operations
    bool unlock(Player& player);
    void open();
    void close();

    // Getters and queries
    DoorState          getState()          const { return m_state; }
    void               setState(DoorState state);
    bool               isOpen()            const { return m_state == DoorState::Open; }
    bool               isLocked()          const { return m_state == DoorState::Locked; }
    int                getTargetRoomId()   const { return m_targetRoomId; }
    const sf::Vector2f& getSpawnOffset()   const { return m_spawnOffset; }
    const std::string& getRequiredKeyId()  const { return m_requiredKeyId; }
    const std::string& getDoorName()       const { return m_doorName; }

    std::string getPrompt() const override;
};

