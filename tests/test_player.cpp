#include "graphics/Animation.hpp"
#include "entities/Player.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

// Helper floating-point comparison
bool approxEqual(float a, float b, float epsilon = 0.001f) {
    return std::abs(a - b) < epsilon;
}

// Test suite demonstrating OOP invariants and unit verification
void testAnimation() {
    std::cout << "[Test 1] Testing Animation class encapsulation and frame progression..." << std::endl;

    Animation anim(0.1f, true);
    assert(anim.getFrameCount() == 0);

    anim.addFrame(sf::IntRect(0, 0, 16, 28));
    anim.addFrame(sf::IntRect(16, 0, 16, 28));
    anim.addFrame(sf::IntRect(32, 0, 16, 28));
    assert(anim.getFrameCount() == 3);
    assert(anim.getCurrentFrameIndex() == 0);

    // Update with 0.05s -> still on frame 0
    anim.update(0.05f);
    assert(anim.getCurrentFrameIndex() == 0);

    // Update with another 0.06s (total 0.11s) -> advances to frame 1
    anim.update(0.06f);
    assert(anim.getCurrentFrameIndex() == 1);

    // Advance to frame 2
    anim.update(0.1f);
    assert(anim.getCurrentFrameIndex() == 2);

    // Advance past end with looping -> cycles back to frame 0
    anim.update(0.1f);
    assert(anim.getCurrentFrameIndex() == 0);

    std::cout << "  -> Animation tests PASSED!" << std::endl;
}

void testPlayerMechanics() {
    std::cout << "[Test 2] Testing Player encapsulation, delta-time movement, and bounds..." << std::endl;

    // Create a dummy 1x1 texture for headless testing
    sf::Texture dummyTexture;
    dummyTexture.create(256, 256);

    Player player(dummyTexture, sf::Vector2f(100.0f, 100.0f));

    // Verify initial encapsulation
    assert(approxEqual(player.getPosition().x, 100.0f));
    assert(approxEqual(player.getPosition().y, 100.0f));
    assert(player.getState() == CharacterState::Idle);
    assert(!player.isMoving());

    // Test velocity and delta time movement
    player.setVelocity(sf::Vector2f(1.0f, 0.0f)); // Moving right
    player.setSpeed(200.0f);
    
    // In 0.5s at 200 px/s, player should travel 100 pixels to x=200
    player.update(0.5f);
    assert(approxEqual(player.getPosition().x, 200.0f));
    assert(approxEqual(player.getPosition().y, 100.0f));

    // Test boundary clamping
    player.clampToBounds(50.0f, 50.0f, 150.0f, 150.0f);
    assert(approxEqual(player.getPosition().x, 150.0f)); // Clamped from 200 to 150

    // Test facing direction
    player.setFacingDirection(Direction::Left);
    assert(player.getFacingDirection() == Direction::Left);
    player.setFacingDirection(Direction::Right);
    assert(player.getFacingDirection() == Direction::Right);

    std::cout << "  -> Player mechanics tests PASSED!" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   DUNGEON REBORN - UNIT TEST SUITE    " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        testAnimation();
        testPlayerMechanics();
        std::cout << "\nALL UNIT TESTS PASSED SUCCESSFULLY!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

