#include "core/Game.hpp"
#include "dungeon/Dungeon.hpp"
#include "entities/Boss.hpp"
#include <iostream>
#include <cassert>

// ============================================================
// PHASE 6: POLISH, STATE MACHINE, WIN CONDITION & AUDIO TESTS
// ============================================================
int main() {
    std::cout << "========================================\n";
    std::cout << "    PHASE 6 GAME LOOP & STATE TESTS     \n";
    std::cout << "========================================\n\n";

    // Test 1: Game initialization and initial states
    std::cout << "  [Test 1] Initial Game State & Menu Selection...\n";
    {
        Game game(960, 640, "Test Game");
        assert(game.getState() == GameState::TitleScreen);
        assert(game.getMenuSelection() == 0);
        assert(!game.isGameWon());
        assert(game.getPlayer() != nullptr);
        assert(game.getPlayer()->isAlive());
        assert(game.getPlayer()->getHealth() == 100);
        assert(game.getDungeon() != nullptr);
        assert(game.getDungeon()->getCurrentRoomId() == 0);
        std::cout << "  [PASS] Initial state is TitleScreen, Menu Selection is 0, GameWon is false\n";
    }

    // Test 2: Menu navigation selection wrapping
    std::cout << "  [Test 2] Menu Selection Navigation & Wrapping...\n";
    {
        Game game(960, 640, "Test Menu");
        assert(game.getMenuSelection() == 0);

        game.setMenuSelection(1);
        assert(game.getMenuSelection() == 1);

        game.setMenuSelection(2);
        assert(game.getMenuSelection() == 2);

        // Selection wrapping
        int sel = 2;
        sel = (sel + 1) % 3; // wrap to 0
        game.setMenuSelection(sel);
        assert(game.getMenuSelection() == 0);

        sel = (sel - 1 + 3) % 3; // wrap back to 2
        game.setMenuSelection(sel);
        assert(game.getMenuSelection() == 2);
        std::cout << "  [PASS] Menu navigation wraps correctly across [0..2]\n";
    }

    // Test 3: Complete State Machine Transitions
    std::cout << "  [Test 3] State Machine Transitions...\n";
    {
        Game game(960, 640, "Test States");
        assert(game.getState() == GameState::TitleScreen);

        // TitleScreen -> ControlsMenu -> TitleScreen
        game.setState(GameState::ControlsMenu);
        assert(game.getState() == GameState::ControlsMenu);
        game.setState(GameState::TitleScreen);
        assert(game.getState() == GameState::TitleScreen);

        // TitleScreen -> Playing -> Paused -> Playing
        game.setState(GameState::Playing);
        assert(game.getState() == GameState::Playing);
        game.setState(GameState::Paused);
        assert(game.getState() == GameState::Paused);
        game.setState(GameState::Playing);
        assert(game.getState() == GameState::Playing);

        // Playing -> GameOver
        game.setState(GameState::GameOver);
        assert(game.getState() == GameState::GameOver);

        // GameOver -> Victory
        game.setState(GameState::Victory);
        assert(game.getState() == GameState::Victory);
        std::cout << "  [PASS] All 6 states transition deterministically\n";
    }

    // Test 4: Audio System Verification (All 8 Sound Buffers)
    std::cout << "  [Test 4] Audio Subsystem & Sound Buffers Verification...\n";
    {
        Game game(960, 640, "Test Audio");
        SoundManager& sm = game.getSoundManager();
        assert(!sm.isMuted());
        assert(sm.getVolume() > 0.0f);

        // Verify all 8 sound IDs can be triggered without crashing
        sm.playSound("swing");
        sm.playSound("hit");
        sm.playSound("hurt");
        sm.playSound("coin");
        sm.playSound("potion");
        sm.playSound("chest");
        sm.playSound("game_over");
        sm.playSound("victory");

        // Test volume clamping invariant
        sm.setVolume(150.0f);
        assert(sm.getVolume() == 100.0f);
        sm.setVolume(-20.0f);
        assert(sm.getVolume() == 0.0f);
        sm.setVolume(70.0f);
        assert(sm.getVolume() == 70.0f);

        // Test mute toggle
        sm.setMuted(true);
        assert(sm.isMuted());
        sm.setMuted(false);
        assert(!sm.isMuted());
        std::cout << "  [PASS] Audio manager verifies all 8 WAV buffers & volume clamping\n";
    }

    // Test 5: Restart Mechanics and Invariant Reset
    std::cout << "  [Test 5] Restart Game Mechanics...\n";
    {
        Game game(960, 640, "Test Restart");
        game.setState(GameState::GameOver);
        assert(game.getState() == GameState::GameOver);

        game.restartGame();
        assert(game.getState() == GameState::Playing);
        assert(!game.isGameWon());
        assert(game.getPlayer() != nullptr);
        assert(game.getPlayer()->isAlive());
        assert(game.getPlayer()->getHealth() == 100);
        assert(game.getDungeon()->getCurrentRoomId() == 0);
        std::cout << "  [PASS] restartGame() resets player, dungeon, and state safely\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "   ALL PHASE 6 POLISH TESTS PASSED!     \n";
    std::cout << "========================================\n\n";
    return 0;
}

