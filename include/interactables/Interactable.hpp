#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Forward declaration to prevent circular include dependencies
class Player;
class SoundManager;

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: Interactable is an abstract base class defining the uniform
// interface for every object in the dungeon that the player can inspect,
// trigger, unlock, or collect (Doors, Traps, Chests, Keys, Shrines).
// It cannot be instantiated directly because interact(), render(),
// and getBounds() are pure virtual (= 0).
//
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: Game subsystems (Dungeon, Room) store collections of base pointers
// (Interactable* or std::shared_ptr<Interactable>) and invoke interact(),
// update(), and render() without knowing the concrete derived type.
// Virtual dispatch routes calls to Door, SpikeTrap, FireTrap, etc. at runtime.
// ============================================================
class Interactable {
// OOP CONCEPT: ACCESS SPECIFIERS (PROTECTED)
// Why: Shared properties like active state and identifier tags are accessible
// to derived classes while protected from arbitrary external mutation.
protected:
    bool        m_active;
    std::string m_tag;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
    // Why: Parameterized constructor initializes base active flag and tag.
    // The virtual destructor is essential: deleting derived objects through
    // an Interactable* or std::shared_ptr<Interactable> must invoke the
    // proper derived destructor without resource or memory leaks.
    // ============================================================
    explicit Interactable(const std::string& tag = "Interactable")
        : m_active(true)
        , m_tag(tag) {}

    virtual ~Interactable() = default;

    // ============================================================
    // OOP CONCEPT: PURE VIRTUAL FUNCTIONS (ABSTRACT CONTRACT)
    // Why: Each derived class defines its own interaction mechanics:
    // - Door checks for keys and opens.
    // - TreasureChest dispenses loot and changes sprite.
    // - Trap damages player and springs animation.
    // - KeyPickup places a Key into Player's Inventory.
    // ============================================================
    virtual void interact(Player& player) = 0;

    // Optional frame-by-frame update for animated or timed interactables (e.g. traps)
    virtual void update(float deltaTime, Player& player, SoundManager* soundMgr = nullptr) {
        (void)deltaTime;
        (void)player;
        (void)soundMgr;
    }

    // Pure virtual rendering
    virtual void render(sf::RenderTarget& target) const = 0;

    // Pure virtual world-space collision bounding box
    virtual sf::FloatRect getBounds() const = 0;

    // ============================================================
    // OOP CONCEPT: VIRTUAL FUNCTIONS & ENCAPSULATION
    // Why: Determines whether this interactable physically blocks player movement
    // (e.g., closed/locked doors block, while floor traps or open doors do not).
    // ============================================================
    virtual bool isBlocking() const { return false; }

    // State queries
    virtual bool isActive() const { return m_active; }
    virtual void setActive(bool active) { m_active = active; }

    const std::string& getTag() const { return m_tag; }

    // Optional UI prompt to show the player when nearby (e.g., "[E] Unlock Boss Door")
    virtual std::string getPrompt() const { return ""; }
};

