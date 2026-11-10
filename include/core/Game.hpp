#pragma once

#include "core/ResourceManager.hpp"
#include "core/SoundManager.hpp"
#include "core/GameState.hpp"
#include "graphics/HealthBar.hpp"
#include "entities/Player.hpp"
#include "entities/Enemy.hpp"
#include "entities/Skeleton.hpp"
#include "entities/Goblin.hpp"
#include "entities/Archer.hpp"
#include "entities/Boss.hpp"
#include "entities/EnemyFactory.hpp"
#include "dungeon/Dungeon.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

// OOP CONCEPT: CLASSES AND OBJECTS
// Why: Game encapsulates the entire application lifecycle, coordinating the window,
// resources, player, and the polymorphic enemy hierarchy.
class Game {
    // OOP CONCEPT: DESIGN PATTERNS (STATE PATTERN)
    // Why: State classes are friends of Game so they can coordinate state transitions
    // and access subsystem renderers without exposing raw internals to external callers.
    friend class GameState;
    friend class MainMenuState;
    friend class ControlsMenuState;
    friend class PlayingState;
    friend class PauseState;
    friend class GameOverState;
    friend class VictoryState;

// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Core engine components and entity containers are private.
private:
    sf::RenderWindow m_window;
    ResourceManager m_resources;
    std::unique_ptr<SoundManager> m_soundManager;
    std::unique_ptr<Player> m_player;

    // OOP CONCEPT: RUNTIME POLYMORPHISM AND OBJECT LIFETIME (SMART POINTERS)
    // Why: m_enemies stores derived Enemy types (Skeleton, Goblin, Boss) polymorphically
    // via std::unique_ptr<Enemy>. Dynamic binding resolves calls to updateAI() and
    // performAttack() at runtime, preventing object slicing and memory leaks.
    std::vector<std::unique_ptr<Enemy>> m_enemies;

    sf::Clock m_clock;

    // OOP CONCEPT: STATE PATTERN & POLYMORPHISM
    // Why: m_currentState holds the currently active GameState object.
    std::unique_ptr<GameState> m_currentState;

    int m_menuSelection;
    bool m_gameWon;
    std::string m_title;
    unsigned int m_windowWidth;
    unsigned int m_windowHeight;
    bool m_isRunning;

    // OOP CONCEPT: COMPOSITION
    // Why: Game has-a Dungeon. The Dungeon owns all Rooms, TileMaps,
    // Tiles, and the CameraController — a full composition sub-tree.
    std::unique_ptr<Dungeon> m_dungeon;

    // OOP CONCEPT: COMPOSITION & ENCAPSULATION
    // Why: HealthBar objects encapsulate dynamic health ratio calculations,
    // color gradients (green/amber/red), background, border, and text display.
    HealthBar m_playerHealthBar;
    HealthBar m_bossHealthBar;

    // Internal game loop helper functions
    void processEvents();
    void update(float deltaTime);
    void render();

    // Specific state renderers
    void renderTitleScreen();
    void renderControlsMenu();
    void renderPlaying();
    void renderPaused();
    void renderGameOver();
    void renderVictory();

    // Internal initialization helpers
    bool loadInitialAssets();
    void initDungeon();
    void spawnEnemiesForRoom(int roomId);
    void updatePlayingLogic(float deltaTime);

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Clean public interface for client code in main.cpp.
public:
    // OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
    // Why: Constructor initializes window, resources, player, and enemies.
    // Destructor cleanly tears down all polymorphic entities via RAII.
    Game(unsigned int width = 960, unsigned int height = 640, const std::string& title = "Dungeon Reborn");
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // OOP CONCEPT: ABSTRACTION & MODULARITY
    // Why: step() advances the game simulation by exactly one discrete frame.
    // This decouples event processing, delta time calculation, physics update,
    // and rendering from the outer execution loop.
    // On desktop, run() loops step() synchronously. Under WebAssembly (Emscripten),
    // step() is invoked asynchronously by the browser's requestAnimationFrame callback.
    void step();

    // OOP CONCEPT: ABSTRACTION
    // Why: Hides all game loop complexity behind run().
    void run();

    // OOP CONCEPT: CONST-CORRECTNESS & STATE PATTERN
    bool isRunning() const;
    void changeState(std::unique_ptr<GameState> newState);
    GameState::Type getState() const;
    void setState(GameState::Type state);
    const GameState* getCurrentStateObject() const { return m_currentState.get(); }

    int getMenuSelection() const;
    void setMenuSelection(int selection);
    bool isGameWon() const;
    void restartGame();

    const sf::RenderWindow& getWindow() const;
    const ResourceManager& getResources() const;
    ResourceManager& getResources();
    SoundManager& getSoundManager();
    const Player* getPlayer() const;
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const;
    const Dungeon* getDungeon() const;
};
