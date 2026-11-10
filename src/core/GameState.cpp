#include "core/GameState.hpp"
#include "core/Game.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: FACTORY PATTERN FOR STATE OBJECTS
// Why: Centralizes instantiation of polymorphic GameState instances.
// ============================================================
std::unique_ptr<GameState> GameState::createState(GameState::Type type) {
    switch (type) {
        case GameState::TitleScreen:
            return std::make_unique<MainMenuState>();
        case GameState::ControlsMenu:
            return std::make_unique<ControlsMenuState>();
        case GameState::Playing:
            return std::make_unique<PlayingState>();
        case GameState::Paused:
            return std::make_unique<PauseState>();
        case GameState::GameOver:
            return std::make_unique<GameOverState>();
        case GameState::Victory:
            return std::make_unique<VictoryState>();
        default:
            return std::make_unique<MainMenuState>();
    }
}

// ============================================================
// 1. MAIN MENU STATE
// ============================================================
void MainMenuState::handleInput(Game& game, const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) return;

    if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
        game.m_menuSelection = (game.m_menuSelection - 1 + 3) % 3;
        if (game.m_soundManager) game.m_soundManager->playSound("coin");
    } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
        game.m_menuSelection = (game.m_menuSelection + 1) % 3;
        if (game.m_soundManager) game.m_soundManager->playSound("coin");
    } else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
        if (game.m_menuSelection == 0) { // Play
            game.setState(GameState::Playing);
            if (game.m_soundManager) game.m_soundManager->playSound("coin");
        } else if (game.m_menuSelection == 1) { // Controls & OOP Guide
            game.setState(GameState::ControlsMenu);
            if (game.m_soundManager) game.m_soundManager->playSound("coin");
        } else if (game.m_menuSelection == 2) { // Exit
            game.m_window.close();
            game.m_isRunning = false;
        }
    } else if (event.key.code == sf::Keyboard::Escape) {
        game.m_window.close();
        game.m_isRunning = false;
    }
}

void MainMenuState::update(Game& game, float deltaTime) {
    // Menu background animation / title pulse can be updated here
    (void)game;
    (void)deltaTime;
}

void MainMenuState::render(Game& game, sf::RenderTarget& target) {
    (void)target;
    game.renderTitleScreen();
}

// ============================================================
// 2. CONTROLS MENU STATE
// ============================================================
void ControlsMenuState::handleInput(Game& game, const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) return;

    if (event.key.code == sf::Keyboard::Escape ||
        event.key.code == sf::Keyboard::Enter ||
        event.key.code == sf::Keyboard::Space ||
        event.key.code == sf::Keyboard::Tab) {
        game.setState(GameState::TitleScreen);
        if (game.m_soundManager) game.m_soundManager->playSound("coin");
    }
}

void ControlsMenuState::update(Game& game, float deltaTime) {
    (void)game;
    (void)deltaTime;
}

void ControlsMenuState::render(Game& game, sf::RenderTarget& target) {
    (void)target;
    game.renderControlsMenu();
}

// ============================================================
// 3. PLAYING STATE
// ============================================================
void PlayingState::handleInput(Game& game, const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) return;

    if (event.key.code == sf::Keyboard::P || event.key.code == sf::Keyboard::Escape) {
        game.setState(GameState::Paused);
        if (game.m_soundManager) game.m_soundManager->playSound("coin");
    } else if (event.key.code == sf::Keyboard::Tab) {
        game.setState(GameState::TitleScreen);
    }

    // [Space] Play sword swing audio when attack is initiated
    if (event.key.code == sf::Keyboard::Space) {
        if (game.m_player && !game.m_player->isAttacking() && game.m_player->getAttackCooldownTimer() <= 0.0f) {
            if (game.m_soundManager) game.m_soundManager->playSound("swing");
        }
    }

    // [K] Damage nearest alive enemy (debug key)
    if (event.key.code == sf::Keyboard::K) {
        for (auto& enemy : game.m_enemies) {
            if (enemy->isAlive()) {
                enemy->takeDamage(20);
                if (game.m_soundManager) game.m_soundManager->playSound("hit");
                break;
            }
        }
    }

    // [B] Trigger boss enrage (debug key)
    if (event.key.code == sf::Keyboard::B) {
        for (auto& enemy : game.m_enemies) {
            if (enemy->getCharacterType() == "DemonBoss" && enemy->isAlive()) {
                std::cout << "\n[Demo] Triggering 130 damage on Boss...\n";
                enemy->takeDamage(130);
                if (game.m_soundManager) game.m_soundManager->playSound("hurt");
                break;
            }
        }
    }

    // [H] Heal player
    if (event.key.code == sf::Keyboard::H && game.m_player) {
        game.m_player->heal(25);
        if (game.m_soundManager) game.m_soundManager->playSound("potion");
        std::cout << "[Player] Healed to " << game.m_player->getHealth() << " HP\n";
    }

    // [R] Reset enemies for current room
    if (event.key.code == sf::Keyboard::R && game.m_dungeon) {
        game.spawnEnemiesForRoom(game.m_dungeon->getCurrentRoomId());
        if (game.m_soundManager) game.m_soundManager->playSound("coin");
    }
}

void PlayingState::update(Game& game, float deltaTime) {
    if (!game.m_player || !game.m_dungeon) return;

    // Check player death and transition to GameOverState
    if (!game.m_player->isAlive()) {
        game.setState(GameState::GameOver);
        if (game.m_soundManager) game.m_soundManager->playSound("game_over");
        std::cout << "\n>>> GAME OVER: The Player has fallen! <<<\n";
        return;
    }

    // Check win condition: Demon Boss defeated in Boss Chamber (RoomType::Boss / Room 1)
    if ((game.m_dungeon->getCurrentRoom().getType() == RoomType::Boss || game.m_dungeon->getCurrentRoomId() == 1) && !game.m_gameWon) {
        bool bossFound = false;
        bool bossAlive = false;
        for (const auto& enemy : game.m_enemies) {
            if (enemy->getCharacterType() == "DemonBoss") {
                bossFound = true;
                if (enemy->isAlive()) bossAlive = true;
            }
        }
        if (bossFound && !bossAlive) {
            game.m_gameWon = true;
            game.setState(GameState::Victory);
            if (game.m_soundManager) game.m_soundManager->playSound("victory");
            std::cout << "\n============================================\n";
            std::cout << ">>> VICTORY ACHIEVED: DEMON BOSS SLAIN! <<<\n";
            std::cout << "============================================\n";
            return;
        }
    }

    // Delegate to Game's playing loop execution
    game.updatePlayingLogic(deltaTime);
}

void PlayingState::render(Game& game, sf::RenderTarget& target) {
    (void)target;
    game.renderPlaying();
}

// ============================================================
// 4. PAUSE STATE
// ============================================================
void PauseState::handleInput(Game& game, const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) return;

    if (event.key.code == sf::Keyboard::P ||
        event.key.code == sf::Keyboard::Escape ||
        event.key.code == sf::Keyboard::Enter) {
        game.setState(GameState::Playing);
        if (game.m_soundManager) game.m_soundManager->playSound("coin");
    } else if (event.key.code == sf::Keyboard::R) {
        game.restartGame();
    } else if (event.key.code == sf::Keyboard::Tab) {
        game.setState(GameState::TitleScreen);
    }
}

void PauseState::update(Game& game, float deltaTime) {
    (void)game;
    (void)deltaTime;
}

void PauseState::render(Game& game, sf::RenderTarget& target) {
    (void)target;
    game.renderPlaying();
    game.renderPaused();
}

// ============================================================
// 5. GAME OVER STATE
// ============================================================
void GameOverState::handleInput(Game& game, const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) return;

    if (event.key.code == sf::Keyboard::R ||
        event.key.code == sf::Keyboard::Enter ||
        event.key.code == sf::Keyboard::Space) {
        game.restartGame();
    } else if (event.key.code == sf::Keyboard::Escape ||
               event.key.code == sf::Keyboard::Tab) {
        game.setState(GameState::TitleScreen);
    }
}

void GameOverState::update(Game& game, float deltaTime) {
    (void)game;
    (void)deltaTime;
}

void GameOverState::render(Game& game, sf::RenderTarget& target) {
    (void)target;
    game.renderPlaying();
    game.renderGameOver();
}

// ============================================================
// 6. VICTORY STATE
// ============================================================
void VictoryState::handleInput(Game& game, const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) return;

    if (event.key.code == sf::Keyboard::R ||
        event.key.code == sf::Keyboard::Enter ||
        event.key.code == sf::Keyboard::Space) {
        game.restartGame();
    } else if (event.key.code == sf::Keyboard::Escape ||
               event.key.code == sf::Keyboard::Tab) {
        game.setState(GameState::TitleScreen);
    }
}

void VictoryState::update(Game& game, float deltaTime) {
    (void)game;
    (void)deltaTime;
}

void VictoryState::render(Game& game, sf::RenderTarget& target) {
    (void)target;
    game.renderPlaying();
    game.renderVictory();
}

