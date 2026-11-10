#include "interactables/Door.hpp"
#include "entities/Player.hpp"
#include "core/SoundManager.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Initializes base Interactable with tag "Door" and configures
// the door's physical bounding box and sprite visual representation.
// ============================================================
Door::Door(const std::string& doorName,
           sf::Vector2f position,
           sf::Vector2f size,
           int targetRoomId,
           sf::Vector2f spawnOffset,
           DoorState initialState,
           const std::string& requiredKeyId,
           const sf::Texture* tileset,
           bool consumeKey)
    : Interactable("Door")
    , m_state(initialState)
    , m_doorName(doorName)
    , m_requiredKeyId(requiredKeyId)
    , m_targetRoomId(targetRoomId)
    , m_spawnOffset(spawnOffset)
    , m_bounds(position.x, position.y, size.x, size.y)
    , m_consumeKey(consumeKey)
    , m_closedRect(64, 48, 16, 16)
    , m_openRect(80, 48, 16, 16) {

    if (tileset) {
        m_sprite.setTexture(*tileset);
        m_sprite.setTextureRect(m_state == DoorState::Open ? m_openRect : m_closedRect);
        m_sprite.setPosition(position);
        float scaleX = size.x / 16.0f;
        float scaleY = size.y / 16.0f;
        m_sprite.setScale(scaleX, scaleY);
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: When the player interacts with a Door (presses [E] or walks into it),
// interact() performs domain logic based on the door's current state:
// - If Locked: inspects player inventory for the key.
// - If Unlocked/Closed: opens the door.
// ============================================================
void Door::interact(Player& player) {
    if (m_state == DoorState::Locked) {
        if (!m_requiredKeyId.empty() && player.getInventory().hasItem(m_requiredKeyId)) {
            unlock(player);
        } else {
            std::cout << "[Door] " << m_doorName << " is firmly locked! You need the key ["
                      << m_requiredKeyId << "] to proceed.\n";
        }
    } else if (m_state == DoorState::Closed || m_state == DoorState::Unlocked) {
        open();
    }
}

bool Door::unlock(Player& player) {
    if (m_state != DoorState::Locked) return false;

    if (!m_requiredKeyId.empty()) {
        if (!player.getInventory().hasItem(m_requiredKeyId)) {
            std::cout << "[Door] Cannot unlock: Missing key [" << m_requiredKeyId << "]!\n";
            return false;
        }

        if (m_consumeKey) {
            player.getInventory().removeItemById(m_requiredKeyId, 1);
            std::cout << "[Door] Used key [" << m_requiredKeyId << "] to unlock the door.\n";
        } else {
            std::cout << "[Door] Unlocked " << m_doorName << " with key [" << m_requiredKeyId << "]!\n";
        }
    }

    m_state = DoorState::Open;
    m_sprite.setTextureRect(m_openRect);
    return true;
}

void Door::open() {
    if (m_state == DoorState::Locked) return;
    m_state = DoorState::Open;
    m_sprite.setTextureRect(m_openRect);
    std::cout << "[Door] " << m_doorName << " opened!\n";
}

void Door::close() {
    m_state = DoorState::Closed;
    m_sprite.setTextureRect(m_closedRect);
}

void Door::setState(DoorState state) {
    m_state = state;
    if (m_state == DoorState::Open) {
        m_sprite.setTextureRect(m_openRect);
    } else {
        m_sprite.setTextureRect(m_closedRect);
    }
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: Overrides isBlocking(). Collision resolution calls this polymorphically
// to treat locked/closed doors as solid obstacles.
// ============================================================
bool Door::isBlocking() const {
    return (m_state == DoorState::Locked || m_state == DoorState::Closed);
}

void Door::update(float deltaTime, Player& player, SoundManager* soundMgr) {
    (void)deltaTime;
    // Check if player is near and interact if key is pressed or walking against it
    if (player.isAlive()) {
        sf::FloatRect pBounds = player.getCollisionBounds();
        // Slightly enlarged interaction zone
        sf::FloatRect triggerZone(m_bounds.left - 10.0f, m_bounds.top - 10.0f,
                                  m_bounds.width + 20.0f, m_bounds.height + 20.0f);

        if (triggerZone.intersects(pBounds)) {
            if (m_state == DoorState::Locked) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) || m_bounds.intersects(pBounds)) {
                    if (!m_requiredKeyId.empty() && player.getInventory().hasItem(m_requiredKeyId)) {
                        bool unlocked = unlock(player);
                        if (unlocked && soundMgr) {
                            soundMgr->playSound("coin");
                        }
                    }
                }
            } else if (m_state == DoorState::Closed) {
                open();
            }
        }
    }
}

void Door::render(sf::RenderTarget& target) const {
    target.draw(m_sprite);

    // If locked, render an aesthetic lock tint / indicator
    if (m_state == DoorState::Locked) {
        sf::RectangleShape lockGlow(sf::Vector2f(m_bounds.width, m_bounds.height));
        lockGlow.setPosition(m_bounds.left, m_bounds.top);
        lockGlow.setFillColor(sf::Color(255, 60, 60, 40));
        lockGlow.setOutlineColor(sf::Color(255, 180, 0, 180));
        lockGlow.setOutlineThickness(1.5f);
        target.draw(lockGlow);
    }
}

std::string Door::getPrompt() const {
    if (m_state == DoorState::Locked) {
        return "[Locked Door] Requires " + m_requiredKeyId;
    }
    return "";
}

