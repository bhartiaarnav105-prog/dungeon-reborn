#include "interactables/Trap.hpp"
#include "entities/Player.hpp"

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Initializes base Interactable with the trap's specific tag,
// and sets damage and cooldown before derived constructors run.
// ============================================================
Trap::Trap(const std::string& trapName, int damage, float cooldown)
    : Interactable(trapName)
    , m_damage(damage)
    , m_cooldown(cooldown)
    , m_cooldownTimer(0.0f)
    , m_isTriggered(false) {
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: update() manages the countdown of m_cooldownTimer safely.
// When the timer elapses, the trap resets its armed state.
// ============================================================
void Trap::update(float deltaTime, Player& player, SoundManager* soundMgr) {
    (void)player;
    (void)soundMgr;
    if (m_cooldownTimer > 0.0f) {
        m_cooldownTimer -= deltaTime;
        if (m_cooldownTimer <= 0.0f) {
            m_cooldownTimer = 0.0f;
            m_isTriggered = false;
        }
    }
}

