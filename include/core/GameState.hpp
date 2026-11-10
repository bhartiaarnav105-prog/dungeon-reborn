#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

class Game;

// ============================================================
// OOP CONCEPT: ABSTRACTION & DESIGN PATTERNS (STATE PATTERN)
// Why: GameState is an abstract base class that encapsulates all behavior
// associated with a particular state of the Game (Input, Update, Render).
// This eliminates monolithic procedural if-else or switch ladders in Game.
//
// OOP CONCEPT: RUNTIME POLYMORPHISM & VIRTUAL DESTRUCTOR
// Why: Dynamic dispatch executes the derived state's overridden behavior
// through a base GameState pointer. The virtual destructor ensures clean
// memory reclamation of derived state objects.
// ============================================================
class GameState {
public:
    enum Type {
        TitleScreen,
        ControlsMenu,
        Playing,
        Paused,
        GameOver,
        Victory
    };

    virtual ~GameState() = default;

    // OOP CONCEPT: PURE VIRTUAL FUNCTIONS (ABSTRACT CONTRACT)
    virtual void handleInput(Game& game, const sf::Event& event) = 0;
    virtual void update(Game& game, float deltaTime) = 0;
    virtual void render(Game& game, sf::RenderTarget& target) = 0;
    virtual Type getType() const = 0;
    virtual std::string getName() const = 0;

    // Factory method to instantiate polymorphic state objects
    static std::unique_ptr<GameState> createState(Type type);
};

// ============================================================
// OOP CONCEPT: INHERITANCE & POLYMORPHISM
// Why: Each derived state class specializes lifecycle methods for its phase.
// ============================================================

class MainMenuState : public GameState {
public:
    void handleInput(Game& game, const sf::Event& event) override;
    void update(Game& game, float deltaTime) override;
    void render(Game& game, sf::RenderTarget& target) override;
    Type getType() const override { return GameState::TitleScreen; }
    std::string getName() const override { return "MainMenuState"; }
};

class ControlsMenuState : public GameState {
public:
    void handleInput(Game& game, const sf::Event& event) override;
    void update(Game& game, float deltaTime) override;
    void render(Game& game, sf::RenderTarget& target) override;
    Type getType() const override { return GameState::ControlsMenu; }
    std::string getName() const override { return "ControlsMenuState"; }
};

class PlayingState : public GameState {
public:
    void handleInput(Game& game, const sf::Event& event) override;
    void update(Game& game, float deltaTime) override;
    void render(Game& game, sf::RenderTarget& target) override;
    Type getType() const override { return GameState::Playing; }
    std::string getName() const override { return "PlayingState"; }
};

class PauseState : public GameState {
public:
    void handleInput(Game& game, const sf::Event& event) override;
    void update(Game& game, float deltaTime) override;
    void render(Game& game, sf::RenderTarget& target) override;
    Type getType() const override { return GameState::Paused; }
    std::string getName() const override { return "PauseState"; }
};

class GameOverState : public GameState {
public:
    void handleInput(Game& game, const sf::Event& event) override;
    void update(Game& game, float deltaTime) override;
    void render(Game& game, sf::RenderTarget& target) override;
    Type getType() const override { return GameState::GameOver; }
    std::string getName() const override { return "GameOverState"; }
};

class VictoryState : public GameState {
public:
    void handleInput(Game& game, const sf::Event& event) override;
    void update(Game& game, float deltaTime) override;
    void render(Game& game, sf::RenderTarget& target) override;
    Type getType() const override { return GameState::Victory; }
    std::string getName() const override { return "VictoryState"; }
};

