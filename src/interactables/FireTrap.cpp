#include "interactables/FireTrap.hpp"
#include "entities/Player.hpp"
#include "core/SoundManager.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (MULTI-LEVEL INITIALIZATION)
// Why: Invokes Trap("FireTrap", damage, 0.5f cooldown between damage ticks).
// ============================================================
FireTrap::FireTrap(sf::Vector2f position,
                   const sf::Texture& tileset,
                   int damage,
                   float dormantTime,
                   float eruptTime)
    : Trap("FireTrap", damage, 0.6f)
    , m_position(position)
    , m_bounds(position.x, position.y, 48.0f, 48.0f)
    , m_sprite(tileset, sf::IntRect(128, 96, 16, 16))
    , m_cycleTimer(0.0f)
    , m_dormantTime(dormantTime)
    , m_eruptTime(eruptTime)
    , m_isErupting(false)
    , m_tickDamageTimer(0.0f) {

    m_sprite.setPosition(position);
    m_sprite.setScale(3.0f, 3.0f);
    m_sprite.setColor(sf::Color(180, 100, 70)); // Dark burnt grate appearance
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: Overrides trigger(). When standing inside active flames,
// damages player with fire burn damage.
// ============================================================
void FireTrap::trigger(Player& player) {
    if (!m_isErupting || m_tickDamageTimer > 0.0f) return;

    m_tickDamageTimer = m_cooldown;
    player.takeDamage(m_damage);
    std::cout << "[FireTrap] *WHOOSH!* Fire burst burned player for "
              << m_damage << " damage! (Player HP: " << player.getHealth() << ")\n";
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: update() encapsulates the periodic oscillation between
// dormant cycle and erupting cycle.
// ============================================================
void FireTrap::update(float deltaTime, Player& player, SoundManager* soundMgr) {
    m_cycleTimer += deltaTime;
    float fullCycle = m_dormantTime + m_eruptTime;

    if (m_cycleTimer >= fullCycle) {
        m_cycleTimer -= fullCycle;
    }

    bool wasErupting = m_isErupting;
    m_isErupting = (m_cycleTimer >= m_dormantTime);

    if (!wasErupting && m_isErupting) {
        if (soundMgr) soundMgr->playSound("swing");
    }

    if (m_tickDamageTimer > 0.0f) {
        m_tickDamageTimer -= deltaTime;
    }

    // If erupting, burn player if standing on trap
    if (m_isErupting && player.isAlive()) {
        sf::FloatRect pBounds = player.getCollisionBounds();
        if (m_bounds.intersects(pBounds)) {
            trigger(player);
            if (soundMgr && m_tickDamageTimer >= m_cooldown - 0.05f) {
                soundMgr->playSound("hurt");
            }
        }
    }
}

void FireTrap::render(sf::RenderTarget& target) const {
    target.draw(m_sprite);

    // If erupting, draw intense orange/red fire hazard flames
    if (m_isErupting) {
        sf::RectangleShape flameOverlay(sf::Vector2f(m_bounds.width, m_bounds.height));
        flameOverlay.setPosition(m_bounds.left, m_bounds.top);
        flameOverlay.setFillColor(sf::Color(255, 120, 20, 160));
        target.draw(flameOverlay);

        sf::CircleShape flameCore(12.0f);
        flameCore.setPosition(m_bounds.left + 12.0f, m_bounds.top + 12.0f);
        flameCore.setFillColor(sf::Color(255, 240, 80, 220));
        target.draw(flameCore);
    }
}

