#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
#include "dungeon/Minimap.hpp"
#include "items/Weapon.hpp"
#include "items/Sword.hpp"
#include "items/Bow.hpp"
#include "items/MagicWeapon.hpp"
#include "core/GameState.hpp"
#include "entities/Player.hpp"
#include "entities/Projectile.hpp"

// Dummy 16x16 texture for test allocations
static sf::Texture createDummyTexture() {
    sf::Image img;
    img.create(64, 64, sf::Color::Blue);
    sf::Texture tex;
    tex.loadFromImage(img);
    return tex;
}

void test_minimap_encapsulation() {
    std::cout << "[Test 1] Testing Minimap Encapsulation & Fog of War Discovery...\n";
    Minimap minimap(sf::Vector2f(10.0f, 10.0f), sf::Vector2f(150.0f, 80.0f));

    // Room 0 is revealed by default at entrance
    assert(minimap.isRoomDiscovered(0));
    assert(!minimap.isRoomDiscovered(1));
    assert(!minimap.isRoomDiscovered(2));

    // Discover room 1 (Grand Maze)
    minimap.discoverRoom(1);
    assert(minimap.isRoomDiscovered(1));
    assert(!minimap.isRoomDiscovered(2));

    // Discover room 2 (Boss Lair)
    minimap.discoverRoom(2);
    assert(minimap.isRoomDiscovered(2));

    std::cout << " -> PASSED: Minimap correctly encapsulates room discovery and fog-of-war state.\n";
}

void test_weapon_polymorphism() {
    std::cout << "[Test 2] Testing Weapon Hierarchy & Runtime Polymorphism...\n";
    sf::Texture dummyTex = createDummyTexture();

    // Polymorphic pointer collection of base class Weapon*
    std::vector<std::shared_ptr<Weapon>> weaponArsenal;
    weaponArsenal.push_back(Weapon::createIronSword());
    weaponArsenal.push_back(Weapon::createHunterBow());
    weaponArsenal.push_back(Weapon::createFireStaff());

    assert(weaponArsenal[0]->getWeaponCategory() == "Sword");
    assert(weaponArsenal[1]->getWeaponCategory() == "Bow");
    assert(weaponArsenal[2]->getWeaponCategory() == "MagicWeapon");

    // Dynamic casting check
    auto sword = std::dynamic_pointer_cast<Sword>(weaponArsenal[0]);
    assert(sword != nullptr);
    assert(sword->getDamage() > 0);

    auto bow = std::dynamic_pointer_cast<Bow>(weaponArsenal[1]);
    assert(bow != nullptr);
    assert(bow->getArrowSpeed() > 0.0f);

    auto magic = std::dynamic_pointer_cast<MagicWeapon>(weaponArsenal[2]);
    assert(magic != nullptr);
    assert(magic->getDamage() > 0);

    // Test polymorphic attack execution
    Player player(dummyTex, sf::Vector2f(200.0f, 200.0f));
    std::vector<Projectile> projectiles;

    // 1. Bow fires arrow projectile
    weaponArsenal[1]->executeAttack(player, projectiles, dummyTex);
    assert(projectiles.size() == 1);
    assert(projectiles[0].isAlive());

    // 2. Magic staff fires magical orb projectile
    weaponArsenal[2]->executeAttack(player, projectiles, dummyTex);
    assert(projectiles.size() == 2);

    std::cout << " -> PASSED: Weapon runtime polymorphism successfully executed unique attack behaviors.\n";
}

void test_state_pattern_hierarchy() {
    std::cout << "[Test 3] Testing GameState Pattern Hierarchy & Dynamic Dispatch...\n";

    // Test Factory Pattern creation
    auto menuState = GameState::createState(GameState::TitleScreen);
    assert(menuState != nullptr);
    assert(menuState->getType() == GameState::TitleScreen);
    assert(menuState->getName() == "MainMenuState");

    auto playState = GameState::createState(GameState::Playing);
    assert(playState != nullptr);
    assert(playState->getType() == GameState::Playing);
    assert(playState->getName() == "PlayingState");

    auto pauseState = GameState::createState(GameState::Paused);
    assert(pauseState != nullptr);
    assert(pauseState->getType() == GameState::Paused);
    assert(pauseState->getName() == "PauseState");

    auto overState = GameState::createState(GameState::GameOver);
    assert(overState != nullptr);
    assert(overState->getType() == GameState::GameOver);
    assert(overState->getName() == "GameOverState");

    auto winState = GameState::createState(GameState::Victory);
    assert(winState != nullptr);
    assert(winState->getType() == GameState::Victory);
    assert(winState->getName() == "VictoryState");

    std::cout << " -> PASSED: State Pattern hierarchy correctly handles dynamic states and factory instantiation.\n";
}

void test_player_weapon_composition() {
    std::cout << "[Test 4] Testing Player-Weapon Composition & Delegation...\n";
    sf::Texture dummyTex = createDummyTexture();
    Player player(dummyTex, sf::Vector2f(100.0f, 100.0f));

    // Player starts with Iron Sword
    assert(player.getEquippedWeapon() != nullptr);
    assert(player.getEquippedWeapon()->getName() == "Iron Sword");

    // Equip Bow
    auto bow = Weapon::createHunterBow();
    player.equipWeapon(bow);
    assert(player.getEquippedWeapon()->getName() == "Hunter's Shortbow");
    assert(player.getAttackDamage() == bow->getDamage());

    // Perform attack with weapon delegation
    std::vector<Projectile> projectiles;
    player.performAttackWithWeapon(projectiles, dummyTex);
    assert(projectiles.size() == 1);

    std::cout << " -> PASSED: Player composed with Weapon delegates attacks polymorphically.\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "   RUNNING PHASE 4 POLISH UNIT TESTS    \n";
    std::cout << "========================================\n";

    test_minimap_encapsulation();
    test_weapon_polymorphism();
    test_state_pattern_hierarchy();
    test_player_weapon_composition();

    std::cout << "========================================\n";
    std::cout << " ALL PHASE 4 UNIT TESTS PASSED (4/4)!   \n";
    std::cout << "========================================\n";
    return 0;
}
