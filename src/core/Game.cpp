#include "core/Game.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALISER LIST)
// Why: Every sub-object is constructed before the constructor
// body runs, ensuring invariants are met from the first line.
// ============================================================
Game::Game(unsigned int width, unsigned int height, const std::string& title)
    : m_window(sf::VideoMode(width, height), title, sf::Style::Titlebar | sf::Style::Close)
    , m_resources()
    , m_soundManager(nullptr)
    , m_player(nullptr)
    , m_enemies()
    , m_clock()
    , m_currentState(GameState::createState(GameState::TitleScreen))
    , m_menuSelection(0)
    , m_gameWon(false)
    , m_title(title)
    , m_windowWidth(width)
    , m_windowHeight(height)
    , m_isRunning(true)
    , m_dungeon(nullptr)
    , m_playerHealthBar(sf::Vector2f(20.0f, 15.0f), sf::Vector2f(200.0f, 22.0f), "KNIGHT HP")
    , m_bossHealthBar(sf::Vector2f((width - 340.0f) * 0.5f, 15.0f), sf::Vector2f(340.0f, 24.0f), "DEMON BOSS")
{
#ifdef __EMSCRIPTEN__
    // Web target: browser's requestAnimationFrame synchronizes frames; avoid synchronous sleep
    m_window.setFramerateLimit(0);
    m_window.setVerticalSyncEnabled(false);
#else
    m_window.setFramerateLimit(60);
    m_window.setVerticalSyncEnabled(true);
#endif

    // OOP CONCEPT: OBJECT LIFETIME AND MEMORY MANAGEMENT (SMART POINTERS)
    // Why: std::make_unique establishes exclusive ownership with RAII.
    m_soundManager = std::make_unique<SoundManager>(m_resources);

    if (!loadInitialAssets()) {
        std::cerr << "[Game ERROR] Critical assets failed to load! Check 'assets/' directory.\n";
    } else {
        std::cout << "[Game] All core assets loaded successfully.\n";

        const sf::Texture& tileset = m_resources.getTexture("dungeon_tileset");

        // OOP CONCEPT: COMPOSITION
        // Why: Player is composed inside Game. Game delegates player-specific
        // responsibilities (input, animation, attack logic) to the Player object.
        m_player = std::make_unique<Player>(tileset, sf::Vector2f(width / 2.0f, height / 2.0f));

        // OOP CONCEPT: COMPOSITION
        // Why: Dungeon is a composed sub-object of Game. It owns all Rooms
        // (which own TileMaps, which own Tiles) — a full composition tree.
        initDungeon();

        m_soundManager->playSound("coin");
    }
}

// OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
// Why: Destructor logs clean shutdown; RAII handles all memory.
Game::~Game() {
    std::cout << "[Game] Engine shutting down cleanly via RAII.\n";
}

bool Game::loadInitialAssets() {
    bool allSucceeded = true;

    if (!m_resources.loadTexture("dungeon_tileset", "assets/images/dungeon_tileset.png")) {
        std::cerr << "[Game ERROR] Failed to load 'assets/images/dungeon_tileset.png'!\n";
        allSucceeded = false;
    }
    if (!m_resources.loadFont("dungeon_font", "assets/fonts/dungeon_font.ttf")) {
        std::cerr << "[Game ERROR] Failed to load 'assets/fonts/dungeon_font.ttf'!\n";
        allSucceeded = false;
    }

    const std::vector<std::pair<std::string,std::string>> soundFiles = {
        {"swing",    "assets/audio/swing.wav"},
        {"hit",      "assets/audio/hit.wav"},
        {"hurt",     "assets/audio/hurt.wav"},
        {"coin",     "assets/audio/coin.wav"},
        {"potion",   "assets/audio/potion.wav"},
        {"chest",    "assets/audio/chest.wav"},
        {"game_over","assets/audio/game_over.wav"},
        {"victory",  "assets/audio/victory.wav"}
    };
    for (const auto& item : soundFiles) {
        if (!m_resources.loadSoundBuffer(item.first, item.second))
            std::cerr << "[Game WARNING] Sound failed: " << item.second << "\n";
    }
    return allSucceeded;
}

// ============================================================
// OOP CONCEPT: COMPOSITION AND OBJECT LIFETIME
// Why: initDungeon() constructs the Dungeon (which builds all
// Rooms, TileMaps, and door connections) and then spawns the
// initial set of enemies for the starting room (room 0).
// ============================================================
void Game::initDungeon() {
    const sf::Texture* ts = m_resources.hasTexture("dungeon_tileset") ? &m_resources.getTexture("dungeon_tileset") : nullptr;
    m_dungeon = std::make_unique<Dungeon>(m_windowWidth, m_windowHeight, ts);

    // Position player at Room 0's spawn point
    const Room& startRoom = m_dungeon->getCurrentRoom();
    if (m_player) {
        m_player->setPosition(startRoom.getPlayerSpawnPosition());
    }

    // Spawn room 0 enemies
    spawnEnemiesForRoom(0);
}

// Restart helper: re-instantiates clean player state and restarts from room 0
void Game::restartGame() {
    if (!m_resources.hasTexture("dungeon_tileset")) return;
    const sf::Texture& tileset = m_resources.getTexture("dungeon_tileset");

    m_player = std::make_unique<Player>(tileset, sf::Vector2f(m_windowWidth / 2.0f, m_windowHeight / 2.0f));
    initDungeon();
    setState(GameState::Playing);
    m_gameWon = false;

    if (m_soundManager) {
        m_soundManager->playSound("coin");
    }
    std::cout << "[Game] Player respawned. Game restarted.\n";
}

// ============================================================
// OOP CONCEPT: DELEGATION & FACTORY PATTERN
// Why: spawnEnemiesForRoom delegates spawning to Dungeon::respawnEnemies,
// which uses EnemyFactory::createEnemy(). Concrete instances (Skeleton, Goblin,
// Archer, Boss) are created and stored polymorphically as std::unique_ptr<Enemy> in m_enemies.
// ============================================================
void Game::spawnEnemiesForRoom(int roomId) {
    if (!m_resources.hasTexture("dungeon_tileset") || !m_dungeon) return;
    const sf::Texture& tileset = m_resources.getTexture("dungeon_tileset");

    m_dungeon->respawnEnemies(roomId, m_enemies, tileset);
}

// ============================================================
// OOP CONCEPT: ABSTRACTION & MODULARITY
// Why: step() encapsulates one discrete tick of the simulation.
// It samples delta time, processes input events, updates state, and renders.
// ============================================================
void Game::step() {
    float deltaTime = m_clock.restart().asSeconds();
    if (deltaTime > 0.1f) deltaTime = 0.1f;

    processEvents();
    update(deltaTime);
    render();
}

#ifdef __EMSCRIPTEN__
static void emscriptenWebLoopCallback(void* userData) {
    Game* game = static_cast<Game*>(userData);
    if (game && game->isRunning()) {
        game->step();
    }
}
#endif

// OOP CONCEPT: ABSTRACTION
// Why: run() drives the game loop without callers needing to know SFML details.
void Game::run() {
    std::cout << "[Game] Window open=" << m_window.isOpen()
              << " size=" << m_window.getSize().x << "x" << m_window.getSize().y << "\n";
    if (!m_window.isOpen()) {
        std::cerr << "[Game ERROR] Window failed to open. Cannot start game loop.\n";
        return;
    }
    std::cout << "[Game] Entering main game loop.\n";

#ifdef __EMSCRIPTEN__
    // Web target: browser's requestAnimationFrame drives frame execution asynchronously
    emscripten_set_main_loop_arg(emscriptenWebLoopCallback, this, 0, 1);
#else
    // Desktop target: standard synchronous while loop
    while (m_window.isOpen() && m_isRunning) {
        step();
    }
    std::cout << "[Game] Main loop exited. isOpen()=" << m_window.isOpen() << "\n";
#endif
}

// ============================================================
// OOP CONCEPT: DESIGN PATTERNS (STATE PATTERN) & POLYMORPHISM
// Why: Input polling is encapsulated; events are dispatched polymorphically
// to the active GameState object (MainMenuState, PlayingState, etc.).
// ============================================================
void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
            m_isRunning = false;
        }

        if (m_currentState) {
            m_currentState->handleInput(*this, event);
        }
    }
}

// ============================================================
// OOP CONCEPT: STATE PATTERN & RUNTIME POLYMORPHISM (DYNAMIC BINDING)
// Why: Game::update() delegates execution to m_currentState->update().
// Concrete states manage transitions (e.g. death -> GameOverState, win -> VictoryState)
// without Game needing a giant procedural if/else ladder.
// ============================================================
void Game::update(float deltaTime) {
    if (m_currentState) {
        m_currentState->update(*this, deltaTime);
    }
}

void Game::updatePlayingLogic(float deltaTime) {
    if (!m_player || !m_dungeon) return;

    // Remember current room before dungeon update
    int prevRoomId = m_dungeon->getCurrentRoomId();

    // Handle player input (sets velocity and triggers sword attack)
    m_player->handleInput();

    // Dungeon::update() applies collision, moves player & enemies, checks combat & door triggers
    if (m_resources.hasTexture("dungeon_tileset")) {
        const sf::Texture& tileset = m_resources.getTexture("dungeon_tileset");
        m_dungeon->update(deltaTime, *m_player, m_enemies, tileset, m_soundManager.get());
    }

    // If room changed, respawn enemies for the new room
    int newRoomId = m_dungeon->getCurrentRoomId();
    if (newRoomId != prevRoomId) {
        spawnEnemiesForRoom(newRoomId);
    }
}

// ============================================================
// OOP CONCEPT: DESIGN PATTERNS (STATE PATTERN) & POLYMORPHISM
// Why: Dynamic dispatch to m_currentState->render().
// Eliminates procedural switch/if-else ladders in the game loop.
// ============================================================
void Game::render() {
    m_window.clear(sf::Color(15, 12, 22));

    if (m_currentState) {
        m_currentState->render(*this, m_window);
    }

    m_window.display();
}

void Game::renderTitleScreen() {
    if (!m_resources.hasFont("dungeon_font")) return;
    const sf::Font& font = m_resources.getFont("dungeon_font");

    // 1. Title Banner
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("DUNGEON REBORN");
    titleText.setCharacterSize(62);
    titleText.setFillColor(sf::Color(255, 215, 0));
    sf::FloatRect tb = titleText.getLocalBounds();
    titleText.setPosition((m_windowWidth - tb.width) / 2.f, 50.f);
    m_window.draw(titleText);

    // 2. Subtitle
    sf::Text subText;
    subText.setFont(font);
    subText.setString("A 2D Object-Oriented Dungeon Adventure in C++17 & SFML");
    subText.setCharacterSize(20);
    subText.setFillColor(sf::Color(210, 210, 230));
    sf::FloatRect sb = subText.getLocalBounds();
    subText.setPosition((m_windowWidth - sb.width) / 2.f, 120.f);
    m_window.draw(subText);

    // 3. Pixel-Art Character Previews
    if (m_resources.hasTexture("dungeon_tileset")) {
        const sf::Texture& ts = m_resources.getTexture("dungeon_tileset");
        auto drawPreview = [&](sf::IntRect rect, float scale, float x, float y, const std::string& label) {
            sf::Sprite s(ts, rect);
            s.setScale(scale, scale);
            s.setOrigin(rect.width / 2.f, rect.height / 2.f);
            s.setPosition(x, y);
            m_window.draw(s);

            sf::Text lbl;
            lbl.setFont(font);
            lbl.setString(label);
            lbl.setCharacterSize(14);
            lbl.setFillColor(sf::Color(180, 180, 200));
            sf::FloatRect lb = lbl.getLocalBounds();
            lbl.setPosition(x - lb.width / 2.f, y + rect.height * scale * 0.5f + 4.f);
            m_window.draw(lbl);
        };

        drawPreview({128, 100, 16, 28}, 3.5f, 210.f, 200.f, "Hero Knight");
        drawPreview({368,  80, 16, 16}, 3.5f, 380.f, 210.f, "Skeleton Scout");
        drawPreview({368,  32, 16, 16}, 3.5f, 550.f, 210.f, "Goblin Skirmisher");
        drawPreview({ 16, 364, 32, 36}, 3.0f, 720.f, 195.f, "Demon Boss");
    }

    // 4. Interactive Menu Options (Play, Controls, Exit)
    const std::vector<std::string> menuOptions = {
        "PLAY ADVENTURE",
        "CONTROLS & ARCHITECTURE",
        "EXIT TO DESKTOP"
    };

    const float startY = 320.0f;
    const float spacing = 55.0f;

    for (size_t i = 0; i < menuOptions.size(); ++i) {
        float yPos = startY + i * spacing;
        bool isSelected = (static_cast<int>(i) == m_menuSelection);

        if (isSelected) {
            // Selected Option Highlight Box
            sf::RectangleShape selBox(sf::Vector2f(420.0f, 44.0f));
            selBox.setOrigin(210.0f, 22.0f);
            selBox.setPosition(m_windowWidth / 2.0f, yPos + 12.0f);
            selBox.setFillColor(sf::Color(60, 45, 20, 230));
            selBox.setOutlineColor(sf::Color(255, 215, 0));
            selBox.setOutlineThickness(2.0f);
            m_window.draw(selBox);
        }

        sf::Text optText;
        optText.setFont(font);
        optText.setString(isSelected ? ">>  " + menuOptions[i] + "  <<" : "    " + menuOptions[i] + "    ");
        optText.setCharacterSize(isSelected ? 24 : 22);
        optText.setFillColor(isSelected ? sf::Color(255, 230, 80) : sf::Color(170, 170, 190));
        sf::FloatRect ob = optText.getLocalBounds();
        optText.setPosition((m_windowWidth - ob.width) / 2.0f, yPos);
        m_window.draw(optText);
    }

    // 5. Navigation Instructions
    sf::Text instrText;
    instrText.setFont(font);
    instrText.setString("[W/S or UP/DOWN] Navigate Menu   |   [ENTER or SPACE] Select   |   [ESC] Exit");
    instrText.setCharacterSize(16);
    instrText.setFillColor(sf::Color(140, 170, 200));
    sf::FloatRect ib = instrText.getLocalBounds();
    instrText.setPosition((m_windowWidth - ib.width) / 2.f, 515.f);
    m_window.draw(instrText);

    // 6. Educational OOP Compliance Banner
    sf::Text hierText;
    hierText.setFont(font);
    hierText.setString("College Viva Project: Demonstrating All 13 Core C++ OOP Principles");
    hierText.setCharacterSize(15);
    hierText.setFillColor(sf::Color(255, 190, 80));
    sf::FloatRect hb = hierText.getLocalBounds();
    hierText.setPosition((m_windowWidth - hb.width) / 2.f, 575.f);
    m_window.draw(hierText);
}

// Controls and Architecture Reference Screen
void Game::renderControlsMenu() {
    if (!m_resources.hasFont("dungeon_font")) return;
    const sf::Font& font = m_resources.getFont("dungeon_font");

    // Header
    sf::Text hdrText;
    hdrText.setFont(font);
    hdrText.setString("CONTROLS & ARCHITECTURAL REFERENCE");
    hdrText.setCharacterSize(34);
    hdrText.setFillColor(sf::Color(255, 215, 0));
    sf::FloatRect hb = hdrText.getLocalBounds();
    hdrText.setPosition((m_windowWidth - hb.width) / 2.0f, 30.0f);
    m_window.draw(hdrText);

    // Section 1: Gameplay Controls
    sf::RectangleShape boxLeft(sf::Vector2f(440.0f, 260.0f));
    boxLeft.setPosition(30.0f, 90.0f);
    boxLeft.setFillColor(sf::Color(25, 20, 35, 220));
    boxLeft.setOutlineColor(sf::Color(100, 140, 220));
    boxLeft.setOutlineThickness(1.5f);
    m_window.draw(boxLeft);

    sf::Text leftTitle;
    leftTitle.setFont(font);
    leftTitle.setString("GAMEPLAY CONTROLS");
    leftTitle.setCharacterSize(20);
    leftTitle.setFillColor(sf::Color(100, 220, 255));
    leftTitle.setPosition(45.0f, 105.0f);
    m_window.draw(leftTitle);

    const std::vector<std::string> gameControls = {
        "WASD / Arrow Keys : Move Knight (Normalized)",
        "[SPACE]           : Sword Slash (Directional Arc)",
        "[1] - [6] Keys    : Hotbar (Equip Weapon / Potion)",
        "[E]               : Open Treasure Chests",
        "[P] or [ESC]      : Pause / Resume Adventure",
        "[TAB]             : Return to Title Screen"
    };
    for (size_t i = 0; i < gameControls.size(); ++i) {
        sf::Text line;
        line.setFont(font);
        line.setString(gameControls[i]);
        line.setCharacterSize(16);
        line.setFillColor(sf::Color(220, 220, 235));
        line.setPosition(45.0f, 140.0f + i * 32.0f);
        m_window.draw(line);
    }

    // Section 2: Academic OOP Live Evaluation Keys
    sf::RectangleShape boxRight(sf::Vector2f(440.0f, 260.0f));
    boxRight.setPosition(490.0f, 90.0f);
    boxRight.setFillColor(sf::Color(25, 20, 35, 220));
    boxRight.setOutlineColor(sf::Color(255, 180, 80));
    boxRight.setOutlineThickness(1.5f);
    m_window.draw(boxRight);

    sf::Text rightTitle;
    rightTitle.setFont(font);
    rightTitle.setString("VIVA DEMONSTRATION HOTKEYS");
    rightTitle.setCharacterSize(20);
    rightTitle.setFillColor(sf::Color(255, 200, 100));
    rightTitle.setPosition(505.0f, 105.0f);
    m_window.draw(rightTitle);

    const std::vector<std::string> vivaControls = {
        "[K] : Strike Nearest Enemy 20 Dmg (Virtual Binding)",
        "[B] : Deal 130 Dmg to Boss (Forces Enrage <= 50% HP)",
        "[H] : Drink Potion +25 HP (Tests Max Cap Invariant)",
        "[R] : Reset Room Enemies / Restart on Over",
        "Throne Room Win : Slay Demon Boss -> Victory!",
        "Dynamic Dispatch: AI and Combat run via Enemy*"
    };
    for (size_t i = 0; i < vivaControls.size(); ++i) {
        sf::Text line;
        line.setFont(font);
        line.setString(vivaControls[i]);
        line.setCharacterSize(16);
        line.setFillColor(sf::Color(240, 230, 210));
        line.setPosition(505.0f, 140.0f + i * 32.0f);
        m_window.draw(line);
    }

    // Section 3: OOP Architecture Highlights
    sf::RectangleShape boxBottom(sf::Vector2f(900.0f, 140.0f));
    boxBottom.setPosition(30.0f, 375.0f);
    boxBottom.setFillColor(sf::Color(20, 18, 30, 220));
    boxBottom.setOutlineColor(sf::Color(120, 220, 140));
    boxBottom.setOutlineThickness(1.5f);
    m_window.draw(boxBottom);

    sf::Text btmTitle;
    btmTitle.setFont(font);
    btmTitle.setString("CORE OOP PRINCIPLES IMPLEMENTED IN CODEBASE");
    btmTitle.setCharacterSize(18);
    btmTitle.setFillColor(sf::Color(120, 255, 160));
    btmTitle.setPosition(45.0f, 388.0f);
    m_window.draw(btmTitle);

    const std::vector<std::string> archPoints = {
        "- Abstraction: Character & Item pure virtual base interfaces with zero code duplication",
        "- Polymorphism: std::vector<unique_ptr<Enemy>> dispatches updateAI(), updateCombat(), dropLoot()",
        "- Composition: Game -> Dungeon -> Room -> TileMap -> Tile (complete object trees)",
        "- Encapsulation & Lifetime: Private invariants, RAII ownership, zero memory leaks"
    };
    for (size_t i = 0; i < archPoints.size(); ++i) {
        sf::Text line;
        line.setFont(font);
        line.setString(archPoints[i]);
        line.setCharacterSize(15);
        line.setFillColor(sf::Color(200, 210, 220));
        line.setPosition(45.0f, 418.0f + i * 23.0f);
        m_window.draw(line);
    }

    // Footer
    sf::Text footerText;
    footerText.setFont(font);
    footerText.setString(">> Press [ESC] or [ENTER] to Return to Title Menu <<");
    footerText.setCharacterSize(22);
    footerText.setFillColor(sf::Color(100, 255, 120));
    sf::FloatRect fb = footerText.getLocalBounds();
    footerText.setPosition((m_windowWidth - fb.width) / 2.0f, 550.0f);
    m_window.draw(footerText);
}

// Pause Overlay Screen
void Game::renderPaused() {
    // 1. Dark semi-transparent veil over the active battlefield
    sf::RectangleShape veil(sf::Vector2f(m_windowWidth, m_windowHeight));
    veil.setFillColor(sf::Color(8, 6, 14, 215));
    m_window.draw(veil);

    if (!m_resources.hasFont("dungeon_font")) return;
    const sf::Font& font = m_resources.getFont("dungeon_font");

    // 2. Centered Modal Box
    const float boxW = 480.0f;
    const float boxH = 280.0f;
    sf::RectangleShape modal(sf::Vector2f(boxW, boxH));
    modal.setPosition((m_windowWidth - boxW) / 2.0f, (m_windowHeight - boxH) / 2.0f);
    modal.setFillColor(sf::Color(24, 18, 34, 245));
    modal.setOutlineColor(sf::Color(255, 215, 0));
    modal.setOutlineThickness(2.5f);
    m_window.draw(modal);

    // Title
    sf::Text title;
    title.setFont(font);
    title.setString("GAME PAUSED");
    title.setCharacterSize(44);
    title.setFillColor(sf::Color(255, 215, 0));
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((m_windowWidth - tb.width) / 2.0f, modal.getPosition().y + 25.0f);
    m_window.draw(title);

    // Status
    sf::Text sub;
    sub.setFont(font);
    sub.setString("Game simulation is frozen. Object state is safely preserved.");
    sub.setCharacterSize(16);
    sub.setFillColor(sf::Color(180, 180, 200));
    sf::FloatRect sb = sub.getLocalBounds();
    sub.setPosition((m_windowWidth - sb.width) / 2.0f, modal.getPosition().y + 85.0f);
    m_window.draw(sub);

    // Options
    const std::vector<std::pair<std::string, sf::Color>> pauseOptions = {
        {"[P] or [ESC] Resume Adventure", sf::Color(100, 255, 120)},
        {"[R] Restart Game",            sf::Color(255, 215, 80)},
        {"[TAB] Return to Main Menu",     sf::Color(160, 190, 240)}
    };

    for (size_t i = 0; i < pauseOptions.size(); ++i) {
        sf::Text opt;
        opt.setFont(font);
        opt.setString(pauseOptions[i].first);
        opt.setCharacterSize(22);
        opt.setFillColor(pauseOptions[i].second);
        sf::FloatRect ob = opt.getLocalBounds();
        opt.setPosition((m_windowWidth - ob.width) / 2.0f, modal.getPosition().y + 130.0f + i * 40.0f);
        m_window.draw(opt);
    }
}

// Victory Screen
void Game::renderVictory() {
    // 1. Dark royal semi-transparent overlay over the cleared dungeon
    sf::RectangleShape veil(sf::Vector2f(m_windowWidth, m_windowHeight));
    veil.setFillColor(sf::Color(12, 10, 25, 230));
    m_window.draw(veil);

    if (!m_resources.hasFont("dungeon_font")) return;
    const sf::Font& font = m_resources.getFont("dungeon_font");

    // 2. Victory Header
    sf::Text title;
    title.setFont(font);
    title.setString("VICTORY ACHIEVED!");
    title.setCharacterSize(64);
    title.setFillColor(sf::Color(255, 215, 0));
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((m_windowWidth - tb.width) / 2.0f, 50.0f);
    m_window.draw(title);

    // Subtitle
    sf::Text sub;
    sub.setFont(font);
    sub.setString("The Demon Overlord has fallen! Peace has returned to the realm!");
    sub.setCharacterSize(22);
    sub.setFillColor(sf::Color(230, 220, 180));
    sf::FloatRect sb = sub.getLocalBounds();
    sub.setPosition((m_windowWidth - sb.width) / 2.0f, 130.0f);
    m_window.draw(sub);

    // 3. Victory Performance Card
    const float cardW = 540.0f;
    const float cardH = 200.0f;
    sf::RectangleShape card(sf::Vector2f(cardW, cardH));
    card.setPosition((m_windowWidth - cardW) / 2.0f, 180.0f);
    card.setFillColor(sf::Color(30, 24, 46, 240));
    card.setOutlineColor(sf::Color(255, 215, 0));
    card.setOutlineThickness(2.0f);
    m_window.draw(card);

    sf::Text cardTitle;
    cardTitle.setFont(font);
    cardTitle.setString("--- QUEST EXPEDITION SUMMARY ---");
    cardTitle.setCharacterSize(20);
    cardTitle.setFillColor(sf::Color(255, 200, 80));
    sf::FloatRect ctb = cardTitle.getLocalBounds();
    cardTitle.setPosition((m_windowWidth - ctb.width) / 2.0f, card.getPosition().y + 15.0f);
    m_window.draw(cardTitle);

    int gold = m_player ? m_player->getInventory().getGold() : 0;
    const Weapon* eqWeapon = m_player ? m_player->getEquippedWeapon() : nullptr;
    std::string weaponName = eqWeapon ? eqWeapon->getName() : "Fists";
    int dmg = m_player ? m_player->getAttackDamage() : 10;
    int hp = m_player ? m_player->getHealth() : 0;
    int maxHp = m_player ? m_player->getMaxHealth() : 100;

    const std::vector<std::string> stats = {
        "Total Gold Plundered : " + std::to_string(gold) + " Gold Coins",
        "Mastered Weapon     : " + weaponName + " (" + std::to_string(dmg) + " Dmg)",
        "Final Hero Vitality : " + std::to_string(hp) + " / " + std::to_string(maxHp) + " HP",
        "Dungeon Conquered    : All 3 Connected Chambers Cleared"
    };

    for (size_t i = 0; i < stats.size(); ++i) {
        sf::Text line;
        line.setFont(font);
        line.setString(stats[i]);
        line.setCharacterSize(18);
        line.setFillColor(sf::Color(220, 225, 240));
        line.setPosition(card.getPosition().x + 40.0f, card.getPosition().y + 55.0f + i * 32.0f);
        m_window.draw(line);
    }

    // 4. Action Calls
    sf::Text restartText;
    restartText.setFont(font);
    restartText.setString(">> Press [R] or [SPACE] to Play Again <<");
    restartText.setCharacterSize(28);
    restartText.setFillColor(sf::Color(100, 255, 120));
    sf::FloatRect rb = restartText.getLocalBounds();
    restartText.setPosition((m_windowWidth - rb.width) / 2.0f, 420.0f);
    m_window.draw(restartText);

    sf::Text menuText;
    menuText.setFont(font);
    menuText.setString("[TAB] or [ESC] Return to Title Screen");
    menuText.setCharacterSize(20);
    menuText.setFillColor(sf::Color(170, 180, 200));
    sf::FloatRect mb = menuText.getLocalBounds();
    menuText.setPosition((m_windowWidth - mb.width) / 2.0f, 485.0f);
    m_window.draw(menuText);
}

void Game::renderPlaying() {
    if (!m_dungeon) return;

    // --- World view: apply camera, render dungeon + entities ---
    m_dungeon->applyCamera(m_window);

    if (m_resources.hasTexture("dungeon_tileset")) {
        const sf::Texture& tileset = m_resources.getTexture("dungeon_tileset");
        m_dungeon->render(m_window, tileset);
    }

    // OOP CONCEPT: RUNTIME POLYMORPHISM (DYNAMIC DISPATCH)
    // Why: Renders all enemies via base pointer enemy->render().
    // Each derived type's overridden render (or the base render) executes correctly.
    for (const auto& enemy : m_enemies) {
        if (enemy->isAlive()) enemy->render(m_window);
    }

    if (m_player) m_player->render(m_window);

    // --- HUD view: reset to default view so HUD is in screen-space ---
    m_window.setView(m_window.getDefaultView());

    if (m_resources.hasFont("dungeon_font") && m_player && m_dungeon) {
        const sf::Font& font = m_resources.getFont("dungeon_font");

        // 1. OOP Encapsulated Player Health Bar
        m_playerHealthBar.update(m_player->getHealth(), m_player->getMaxHealth());
        m_playerHealthBar.render(m_window, &font);

        // 2. Player status & gold text
        std::ostringstream ssPlayer;
        ssPlayer << "HP: " << m_player->getHealth() << "/" << m_player->getMaxHealth()
                 << " | " << (m_player->isAttacking() ? "Slashing!" : (m_player->isMoving() ? "Walking" : "Idle"))
                 << " | Gold: " << m_player->getInventory().getGold();

        sf::Text playerText;
        playerText.setFont(font);
        playerText.setString(ssPlayer.str());
        playerText.setCharacterSize(18);
        playerText.setFillColor(sf::Color(100, 255, 120));
        playerText.setPosition(240.f, 16.f);
        m_window.draw(playerText);

        // Equipped weapon text
        const Weapon* eqWeapon = m_player->getEquippedWeapon();
        std::ostringstream ssWeapon;
        ssWeapon << "Weapon: " << (eqWeapon ? eqWeapon->getName() : "Fists")
                 << " (" << m_player->getAttackDamage() << " Dmg, "
                 << std::fixed << std::setprecision(2) << m_player->getAttackCooldown() << "s CD)";

        sf::Text weaponText;
        weaponText.setFont(font);
        weaponText.setString(ssWeapon.str());
        weaponText.setCharacterSize(18);
        weaponText.setFillColor(sf::Color(255, 230, 100));
        weaponText.setPosition(520.f, 22.f);
        m_window.draw(weaponText);

        // 3. Room info
        const Room& curRoom = m_dungeon->getCurrentRoom();
        sf::Text roomText;
        roomText.setFont(font);
        roomText.setString("Room: " + curRoom.getName());
        roomText.setCharacterSize(20);
        roomText.setFillColor(sf::Color(255, 215, 0));
        roomText.setPosition(20.f, 65.f);
        m_window.draw(roomText);

        // 4. Enemy Roster
        std::ostringstream ssEnemies;
        ssEnemies << "Enemies: ";
        for (const auto& e : m_enemies) {
            ssEnemies << "[" << e->getCharacterType() << ": "
                      << (e->isAlive() ? std::to_string(e->getHealth()) + " HP" : "DEAD")
                      << "] ";
        }
        sf::Text enemiesText;
        enemiesText.setFont(font);
        enemiesText.setString(ssEnemies.str());
        enemiesText.setCharacterSize(18);
        enemiesText.setFillColor(sf::Color(255, 150, 80));
        enemiesText.setPosition(20.f, 90.f);
        m_window.draw(enemiesText);

        // Contextual Interaction Prompt (Locked Door, Key Pickup, Chest)
        std::string prompt = m_dungeon->getActiveInteractionPrompt(*m_player);
        if (!prompt.empty()) {
            sf::Text promptText;
            promptText.setFont(font);
            promptText.setString(prompt);
            promptText.setCharacterSize(22);
            promptText.setFillColor(sf::Color(255, 235, 90));
            sf::FloatRect pb = promptText.getLocalBounds();
            promptText.setPosition((m_windowWidth - pb.width) / 2.0f, m_windowHeight - 125.0f);
            m_window.draw(promptText);
        }

        // 5. Inventory Hotbar UI (Slots 1 to 6)
        if (m_resources.hasTexture("dungeon_tileset")) {
            const sf::Texture& tileset = m_resources.getTexture("dungeon_tileset");
            const auto& inv = m_player->getInventory();
            const float slotSize = 42.0f;
            const float slotSpacing = 6.0f;
            const float barStartX = (m_windowWidth - (6 * (slotSize + slotSpacing))) / 2.0f;
            const float barStartY = m_windowHeight - 85.0f;

            for (size_t i = 0; i < 6; ++i) {
                float slotX = barStartX + i * (slotSize + slotSpacing);
                float slotY = barStartY;

                const InventorySlot* slot = inv.getSlot(i);
                bool isEquipped = (slot && slot->item && eqWeapon && slot->item->getId() == eqWeapon->getId());

                // Slot background box
                sf::RectangleShape slotBox(sf::Vector2f(slotSize, slotSize));
                slotBox.setPosition(slotX, slotY);
                slotBox.setFillColor(sf::Color(25, 20, 35, 220));
                slotBox.setOutlineColor(isEquipped ? sf::Color(255, 215, 0) : sf::Color(100, 100, 140));
                slotBox.setOutlineThickness(isEquipped ? 2.5f : 1.0f);
                m_window.draw(slotBox);

                // Slot index label [1..6]
                sf::Text keyLabel;
                keyLabel.setFont(font);
                keyLabel.setString(std::to_string(i + 1));
                keyLabel.setCharacterSize(12);
                keyLabel.setFillColor(sf::Color(160, 160, 200));
                keyLabel.setPosition(slotX + 3.0f, slotY + 2.0f);
                m_window.draw(keyLabel);

                // Draw item icon & quantity badge
                if (slot && slot->item) {
                    sf::Sprite icon(tileset, slot->item->getIconRect());
                    // Scale 16x16 or 16x32 icon to fit inside 42x42 box
                    float iconScale = (slot->item->getIconRect().height > 16) ? 1.0f : 1.8f;
                    icon.setScale(iconScale, iconScale);
                    icon.setOrigin(slot->item->getIconRect().width / 2.0f, slot->item->getIconRect().height / 2.0f);
                    icon.setPosition(slotX + slotSize / 2.0f, slotY + slotSize / 2.0f);
                    m_window.draw(icon);

                    // Stack quantity label
                    if (slot->quantity > 1) {
                        sf::Text qtyText;
                        qtyText.setFont(font);
                        qtyText.setString("x" + std::to_string(slot->quantity));
                        qtyText.setCharacterSize(14);
                        qtyText.setFillColor(sf::Color(255, 255, 100));
                        qtyText.setPosition(slotX + slotSize - 20.0f, slotY + slotSize - 16.0f);
                        m_window.draw(qtyText);
                    }

                    // Equipped badge
                    if (isEquipped) {
                        sf::Text eqText;
                        eqText.setFont(font);
                        eqText.setString("EQ");
                        eqText.setCharacterSize(12);
                        eqText.setFillColor(sf::Color(255, 215, 0));
                        eqText.setPosition(slotX + 22.0f, slotY + 2.0f);
                        m_window.draw(eqText);
                    }
                }
            }
        }

        // 6. Boss Mega Health Bar (when in Demon's Sanctum)
        // OOP CONCEPT: COMPOSITION & ENCAPSULATION
        // Why: Delegates boss health visualization to the encapsulated HealthBar object.
        if (m_dungeon->getCurrentRoomId() == 1) {
            for (const auto& e : m_enemies) {
                if (e->getCharacterType() == "DemonBoss" && e->isAlive()) {
                    m_bossHealthBar.update(e->getHealth(), e->getMaxHealth());
                    m_bossHealthBar.render(m_window, &font);
                    break;
                }
            }
        }

        // 6. OOP Minimap HUD (top-right corner, HUD space)
        // OOP CONCEPT: COMPOSITION & DELEGATION
        // Why: Delegates minimap display to Dungeon's composed Minimap instance.
        // Displays discovered rooms, fog-of-war tracking, and real-time player radar blip.
        m_dungeon->renderMinimap(m_window, m_player->getPosition());

        // Minimap Title Label
        sf::Text minimapLabel;
        minimapLabel.setFont(font);
        minimapLabel.setString("MAP");
        minimapLabel.setCharacterSize(14);
        minimapLabel.setFillColor(sf::Color(255, 215, 80));
        minimapLabel.setPosition(static_cast<float>(m_windowWidth) - 185.0f, 18.0f);
        m_window.draw(minimapLabel);

        // Controls hint
        sf::Text hintText;
        hintText.setFont(font);
        hintText.setString("[SPACE] Attack | [1-6] Items | [E] Chest | [P] Pause | [K] Dmg | [B] Boss | [H] Heal | [TAB] Menu");
        hintText.setCharacterSize(16);
        hintText.setFillColor(sf::Color(180, 220, 255));
        hintText.setPosition(20.f, m_windowHeight - 25.f);
        m_window.draw(hintText);
    }
}

// Game Over screen rendering
void Game::renderGameOver() {
    // 1. Dim background
    sf::RectangleShape dimOverlay(sf::Vector2f(m_windowWidth, m_windowHeight));
    dimOverlay.setFillColor(sf::Color(20, 5, 10, 235));
    m_window.draw(dimOverlay);

    if (!m_resources.hasFont("dungeon_font")) return;
    const sf::Font& font = m_resources.getFont("dungeon_font");

    // Title
    sf::Text goText;
    goText.setFont(font);
    goText.setString("GAME OVER");
    goText.setCharacterSize(72);
    goText.setFillColor(sf::Color(240, 40, 40));
    sf::FloatRect gb = goText.getLocalBounds();
    goText.setPosition((m_windowWidth - gb.width) / 2.0f, 150.0f);
    m_window.draw(goText);

    // Subtitle
    sf::Text subText;
    subText.setFont(font);
    subText.setString("You were slain in the depths of the dungeon...");
    subText.setCharacterSize(24);
    subText.setFillColor(sf::Color(200, 180, 180));
    sf::FloatRect sb = subText.getLocalBounds();
    subText.setPosition((m_windowWidth - sb.width) / 2.0f, 245.0f);
    m_window.draw(subText);

    // Call to Action: Respawn
    sf::Text restartText;
    restartText.setFont(font);
    restartText.setString(">> Press [R] or [SPACE] to Respawn <<");
    restartText.setCharacterSize(28);
    restartText.setFillColor(sf::Color(100, 255, 120));
    sf::FloatRect rb = restartText.getLocalBounds();
    restartText.setPosition((m_windowWidth - rb.width) / 2.0f, 350.0f);
    m_window.draw(restartText);

    // Menu option
    sf::Text menuText;
    menuText.setFont(font);
    menuText.setString("[ESC] or [TAB] Return to Title Screen");
    menuText.setCharacterSize(20);
    menuText.setFillColor(sf::Color(160, 160, 180));
    sf::FloatRect mb = menuText.getLocalBounds();
    menuText.setPosition((m_windowWidth - mb.width) / 2.0f, 415.0f);
    m_window.draw(menuText);
}

// ============================================================
// OOP CONCEPT: CONST-CORRECTNESS
// Why: Query functions are marked const — they never mutate state.
// ============================================================
bool Game::isRunning() const { return m_isRunning; }

// ============================================================
// OOP CONCEPT: DESIGN PATTERNS (STATE PATTERN)
// Why: getState() and setState() interact with the polymorphic m_currentState object.
// ============================================================
GameState::Type Game::getState() const {
    if (m_currentState) {
        return m_currentState->getType();
    }
    return GameState::TitleScreen;
}

void Game::setState(GameState::Type state) {
    m_currentState = GameState::createState(state);
}

void Game::changeState(std::unique_ptr<GameState> newState) {
    if (newState) {
        m_currentState = std::move(newState);
    }
}

int Game::getMenuSelection() const { return m_menuSelection; }
void Game::setMenuSelection(int selection) { m_menuSelection = selection; }
bool Game::isGameWon() const { return m_gameWon; }

const sf::RenderWindow& Game::getWindow() const { return m_window; }
const ResourceManager& Game::getResources() const { return m_resources; }
ResourceManager& Game::getResources() { return m_resources; }
SoundManager& Game::getSoundManager() { return *m_soundManager; }
const Player* Game::getPlayer() const { return m_player.get(); }
const std::vector<std::unique_ptr<Enemy>>& Game::getEnemies() const { return m_enemies; }
const Dungeon* Game::getDungeon() const { return m_dungeon.get(); }
