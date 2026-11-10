#include "entities/Enemy.hpp"
#include "entities/Skeleton.hpp"
#include "entities/Goblin.hpp"
#include "entities/Archer.hpp"
#include "entities/Boss.hpp"
#include "entities/EnemyFactory.hpp"
#include "entities/Player.hpp"
#include "dungeon/CollisionSystem.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

void testEnemyFactoryAndPolymorphism() {
    std::cout << "[Test 1] Testing EnemyFactory & Heterogeneous Polymorphic Collection...\n";

    sf::Texture dummyTex;
    dummyTex.create(32, 32);

    // OOP CONCEPT: FACTORY PATTERN & ABSTRACTION
    // Centralized creation returns std::unique_ptr<Enemy>
    std::vector<std::unique_ptr<Enemy>> roster;
    roster.push_back(EnemyFactory::createEnemy(EnemyType::Skeleton, dummyTex, sf::Vector2f(100.f, 100.f)));
    roster.push_back(EnemyFactory::createEnemy(EnemyType::Goblin, dummyTex, sf::Vector2f(200.f, 200.f)));
    roster.push_back(EnemyFactory::createEnemy(EnemyType::Archer, dummyTex, sf::Vector2f(300.f, 300.f)));
    roster.push_back(EnemyFactory::createEnemy(EnemyType::Boss, dummyTex, sf::Vector2f(400.f, 400.f)));

    assert(roster.size() == 4);

    // OOP CONCEPT: RUNTIME POLYMORPHISM & DYNAMIC BINDING
    // Querying virtual getCharacterType() dispatches to concrete subclass
    assert(roster[0]->getCharacterType() == "Skeleton");
    assert(roster[1]->getCharacterType() == "Goblin");
    assert(roster[2]->getCharacterType() == "Archer");
    assert(roster[3]->getCharacterType() == "DemonBoss");

    // Base attributes verified
    assert(roster[0]->getMaxHealth() == 45);  // Skeleton
    assert(roster[1]->getMaxHealth() == 30);  // Goblin
    assert(roster[2]->getMaxHealth() == 35);  // Archer
    assert(roster[3]->getMaxHealth() == 250); // Boss

    std::cout << "  -> EnemyFactory and polymorphic collection verified.\n";
}

void testSkeletonBehavior() {
    std::cout << "[Test 2] Testing Skeleton Relentless Pursuit & Melee Strike...\n";

    sf::Texture dummyTex;
    dummyTex.create(32, 32);

    Skeleton skeleton(dummyTex, sf::Vector2f(100.f, 100.f));
    Player player(dummyTex, sf::Vector2f(160.f, 100.f)); // 60px away (within detection radius)

    // AI update towards player
    skeleton.updateAI(0.016f, player.getPosition());
    assert(skeleton.isChasing());
    assert(skeleton.getVelocity().x > 0.0f); // Moving right towards player

    // Move skeleton into melee strike range
    skeleton.setPosition(sf::Vector2f(130.f, 100.f)); // 30px away (within attack range)
    skeleton.updateAI(0.016f, player.getPosition());
    assert(skeleton.getAIState() == AIState::Attack);

    int hpBefore = player.getHealth();
    std::vector<Projectile> projs;
    skeleton.updateCombat(0.016f, player, projs, dummyTex);
    assert(player.getHealth() < hpBefore); // Took melee claw damage!
    assert(projs.empty()); // Skeletons do not fire projectiles

    std::cout << "  -> Skeleton pursuit and melee combat verified.\n";
}

void testGoblinSkirmishAndRetreat() {
    std::cout << "[Test 3] Testing Goblin Flanking & Low-Health Panic Retreat...\n";

    sf::Texture dummyTex;
    dummyTex.create(32, 32);

    Goblin goblin(dummyTex, sf::Vector2f(100.f, 100.f));
    Player player(dummyTex, sf::Vector2f(200.f, 100.f));

    // Healthy goblin chases / approaches
    goblin.updateAI(0.016f, player.getPosition());
    assert(goblin.isChasing());

    // When goblin is severely damaged (HP <= 12), it MUST retreat / flee!
    goblin.takeDamage(20); // 30 - 20 = 10 HP remaining!
    assert(goblin.getHealth() == 10);

    goblin.updateAI(0.016f, player.getPosition());
    // Velocity must be directed away from the player (player is to the right at x=200, so velocity.x must be < 0)
    assert(goblin.getVelocity().x < 0.0f);

    std::cout << "  -> Goblin low-health panic retreat verified.\n";
}

void testArcherKitingAndRangedCombat() {
    std::cout << "[Test 4] Testing Archer Kiting Distance & Arrow Projectiles...\n";

    sf::Texture dummyTex;
    dummyTex.create(32, 32);

    Archer archer(dummyTex, sf::Vector2f(200.f, 200.f));
    Player player(dummyTex, sf::Vector2f(250.f, 200.f)); // 50px away (Too close! Under minDistance = 120px)

    // Kiting test: player is too close, archer MUST back away
    archer.updateAI(0.016f, player.getPosition());
    assert(archer.isChasing());
    assert(archer.getVelocity().x < 0.0f); // Backing away to the left!

    // Place player at ideal firing standoff distance (200px)
    player.setPosition(sf::Vector2f(400.f, 200.f));
    archer.updateAI(0.016f, player.getPosition());

    std::vector<Projectile> arrows;
    // Advance time by 2.0s to allow shoot timer
    archer.updateCombat(2.0f, player, arrows, dummyTex);
    assert(!arrows.empty()); // Archer fired an arrow projectile!
    assert(arrows[0].getDamage() == archer.getArrowDamage());

    std::cout << "  -> Archer kiting behavior and arrow projectile combat verified.\n";
}

void testBossStateMachineAndPhases() {
    std::cout << "[Test 5] Testing Boss State Machine (Idle, Chase, Attack, Retreat, Enraged)...\n";

    sf::Texture dummyTex;
    dummyTex.create(32, 32);

    Boss boss(dummyTex, sf::Vector2f(500.f, 500.f));
    Player player(dummyTex, sf::Vector2f(900.f, 900.f)); // Outside detection radius (350px)

    // 1. Idle state
    boss.updateAI(0.016f, player.getPosition());
    assert(boss.getBossState() == BossState::Idle);

    // 2. Chase state: player approaches
    player.setPosition(sf::Vector2f(650.f, 500.f)); // 150px away
    boss.updateAI(0.016f, player.getPosition());
    assert(boss.getBossState() == BossState::Chase);

    // 3. Attack state: player in slam range (50px)
    player.setPosition(sf::Vector2f(550.f, 500.f));
    boss.updateAI(0.016f, player.getPosition());
    std::vector<Projectile> shockwaves;
    boss.updateCombat(0.016f, player, shockwaves, dummyTex);
    assert(boss.isWindingUp());
    assert(boss.getBossState() == BossState::Attack);

    // 4. Complete windup and execute slam -> enters Retreat state
    boss.updateCombat(0.6f, player, shockwaves, dummyTex);
    boss.updateAI(0.016f, player.getPosition());
    assert(boss.getBossState() == BossState::Retreat);

    // 5. Enraged state: drop boss below 50% HP (<= 125 HP)
    boss.takeDamage(130);
    assert(boss.isEnraged());
    assert(boss.getBossState() == BossState::Enraged);
    player.setPosition(sf::Vector2f(650.f, 500.f)); // 150px away
    boss.updateAI(0.016f, player.getPosition());
    assert(boss.getBossState() == BossState::Enraged);

    std::cout << "  -> Boss full state machine transitions verified.\n";
}

void testEnemyWallCollisionResolution() {
    std::cout << "[Test 6] Testing Enemy Dungeon Wall & Boundary Respect...\n";

    sf::FloatRect enemyBounds(100.f, 100.f, 32.f, 32.f);
    sf::Vector2f desiredMove(20.f, 0.f); // Wants to move right into a wall

    std::vector<sf::FloatRect> solidWalls;
    solidWalls.emplace_back(120.f, 90.f, 48.f, 48.f); // Wall blocking x=120..168

    // OOP CONCEPT: ENCAPSULATION
    // resolveCollision ensures enemy cannot penetrate solid wall rects
    sf::Vector2f safeMove = CollisionSystem::resolveCollision(enemyBounds, desiredMove, solidWalls);
    assert(safeMove.x <= 0.0f); // Stopped by wall!

    std::cout << "  -> Enemy wall collision boundary compliance verified.\n";
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "PHASE 3: OOP ENEMY AI AND DUNGEON COMBAT TEST SUITE\n";
    std::cout << "=========================================================\n";

    testEnemyFactoryAndPolymorphism();
    testSkeletonBehavior();
    testGoblinSkirmishAndRetreat();
    testArcherKitingAndRangedCombat();
    testBossStateMachineAndPhases();
    testEnemyWallCollisionResolution();

    std::cout << "=========================================================\n";
    std::cout << "ALL PHASE 3 ENEMY AI TESTS PASSED SUCCESSFULLY (6/6)!\n";
    std::cout << "=========================================================\n";
    return 0;
}
