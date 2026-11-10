#include "entities/Character.hpp"
#include "entities/Player.hpp"
#include "entities/Enemy.hpp"
#include "entities/Skeleton.hpp"
#include "entities/Goblin.hpp"
#include "entities/Boss.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>

void testPolymorphicHierarchy() {
    std::cout << "[Test 1] Testing Character hierarchy, polymorphism, and dynamic binding..." << std::endl;

    sf::Texture dummyTexture;
    dummyTexture.create(256, 256);

    // OOP CONCEPT: RUNTIME POLYMORPHISM & OBJECT LIFETIME (SMART POINTERS)
    // Why: Storing derived objects via std::unique_ptr<Character> in a heterogeneous container.
    // Dynamic binding ensures calls to virtual functions execute the derived class's override.
    std::vector<std::unique_ptr<Character>> roster;
    roster.push_back(std::make_unique<Player>(dummyTexture, sf::Vector2f(100.f, 100.f)));
    roster.push_back(std::make_unique<Skeleton>(dummyTexture, sf::Vector2f(200.f, 200.f)));
    roster.push_back(std::make_unique<Goblin>(dummyTexture, sf::Vector2f(300.f, 300.f)));
    roster.push_back(std::make_unique<Boss>(dummyTexture, sf::Vector2f(400.f, 400.f)));

    // Verify dynamic type resolution via virtual function
    assert(roster[0]->getCharacterType() == "Player");
    assert(roster[1]->getCharacterType() == "Skeleton");
    assert(roster[2]->getCharacterType() == "Goblin");
    assert(roster[3]->getCharacterType() == "DemonBoss");

    // Verify initial health stats
    assert(roster[0]->getHealth() == 100);
    assert(roster[1]->getHealth() == 45);
    assert(roster[2]->getHealth() == 30);
    assert(roster[3]->getHealth() == 250);

    // Verify dynamic_cast across the multilevel hierarchy
    assert(dynamic_cast<Enemy*>(roster[0].get()) == nullptr); // Player is not an Enemy
    assert(dynamic_cast<Enemy*>(roster[1].get()) != nullptr); // Skeleton is an Enemy
    assert(dynamic_cast<Enemy*>(roster[2].get()) != nullptr); // Goblin is an Enemy
    assert(dynamic_cast<Enemy*>(roster[3].get()) != nullptr); // Boss is an Enemy

    // Execute polymorphic attacks
    for (auto& character : roster) {
        character->performAttack(); // Invokes overridden attack dynamically
    }

    std::cout << "  -> Polymorphic hierarchy tests PASSED!" << std::endl;
}

void testDamageEncapsulationAndDeath() {
    std::cout << "[Test 2] Testing damage encapsulation, healing invariants, and death..." << std::endl;

    sf::Texture dummyTexture;
    dummyTexture.create(256, 256);

    Skeleton skeleton(dummyTexture, sf::Vector2f(0.f, 0.f));
    assert(skeleton.getHealth() == 45);
    assert(skeleton.isAlive());

    // Take partial damage
    skeleton.takeDamage(20);
    assert(skeleton.getHealth() == 25);
    assert(skeleton.isAlive());

    // Heal back up (clamped to maxHealth)
    skeleton.heal(100);
    assert(skeleton.getHealth() == 45); // Cannot exceed maxHealth (45)

    // Fatal damage
    skeleton.takeDamage(50);
    assert(skeleton.getHealth() == 0); // Clamped at 0
    assert(!skeleton.isAlive());
    assert(skeleton.getState() == CharacterState::Dead);

    // Further damage when dead is ignored
    skeleton.takeDamage(10);
    assert(skeleton.getHealth() == 0);

    std::cout << "  -> Damage encapsulation tests PASSED!" << std::endl;
}

void testBossEnragePhase() {
    std::cout << "[Test 3] Testing Boss specialized mechanics and Enrage transition..." << std::endl;

    sf::Texture dummyTexture;
    dummyTexture.create(256, 256);

    Boss boss(dummyTexture, sf::Vector2f(0.f, 0.f));
    assert(boss.getHealth() == 250);
    assert(!boss.isEnraged());
    assert(boss.getSpeed() == 75.0f);

    // Take 100 damage -> 150 HP (still > 50%, not enraged)
    boss.takeDamage(100);
    assert(boss.getHealth() == 150);
    assert(!boss.isEnraged());

    // Take another 30 damage -> 120 HP (<= 125, triggers Enrage phase!)
    boss.takeDamage(30);
    assert(boss.getHealth() == 120);
    assert(boss.isEnraged());
    assert(boss.getSpeed() == 125.0f); // Boosted speed

    std::cout << "  -> Boss Enrage phase tests PASSED!" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  CHARACTER HIERARCHY OOP TEST SUITE   " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        testPolymorphicHierarchy();
        testDamageEncapsulationAndDeath();
        testBossEnragePhase();
        std::cout << "\nALL PHASE 2 CHARACTER TESTS PASSED!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

