#pragma once

#include "interactables/Interactable.hpp"

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: Trap IS-AN Interactable. It inherits the abstract base interface
// and adds trap-specific invariants: damage values, trigger cooldowns,
// and arming states.
//
// OOP CONCEPT: ABSTRACTION
// Why: Trap is itself an abstract class. The pure virtual function
// trigger(Player& player) = 0 mandates that each derived trap subclass
// (SpikeTrap, FireTrap) defines its own specialized damage and hazard logic.
//
// OOP CONCEPT: ENCAPSULATION
// Why: Cooldown timers, damage attributes, and triggered flags are protected
// or private, preventing arbitrary tampering by external systems.
// ============================================================
class Trap : public Interactable {
protected:
    int   m_damage;
    float m_cooldown;
    float m_cooldownTimer;
    bool  m_isTriggered;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
    // Why: Parameterized constructor initializes base tag, trap damage,
    // and re-arm cooldown period. Virtual destructor ensures safe deletion.
    // ============================================================
    Trap(const std::string& trapName, int damage, float cooldown = 1.0f);

    ~Trap() override = default;

    // ============================================================
    // OOP CONCEPT: PURE VIRTUAL FUNCTIONS
    // Why: Subclasses must define what happens when a player triggers the trap.
    // Spike traps deal puncture damage; fire traps inflict burning damage.
    // ============================================================
    virtual void trigger(Player& player) = 0;

    // Implements Interactable::interact by calling trigger()
    void interact(Player& player) override {
        trigger(player);
    }

    // Common update timer logic for trap cooldowns
    void update(float deltaTime, Player& player, SoundManager* soundMgr = nullptr) override;

    // Queries
    int   getDamage() const { return m_damage; }
    bool  isTriggered() const { return m_isTriggered; }
    float getCooldown() const { return m_cooldown; }
};

