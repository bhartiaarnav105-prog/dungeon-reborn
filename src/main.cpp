#include "core/Game.hpp"
#include <iostream>
#include <exception>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static Game* g_activeGame = nullptr;

extern "C" {
EMSCRIPTEN_KEEPALIVE
void startGameFromWeb() {
    if (g_activeGame) {
        g_activeGame->setState(GameState::Playing);
        std::cout << "[Game] START GAME received from browser! State switched to Playing.\n";
    }
}
}
#endif

// OOP CONCEPT: CLASSES AND OBJECTS
// Why: main.cpp acts only as the minimal entry point. It creates a Game object
// on the stack, demonstrating automatic lifetime management (RAII), and delegates
// all execution to Game::run().
int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "      DUNGEON REBORN - OOP C++ SFML      " << std::endl;
        std::cout << "========================================" << std::endl;

        // OOP CONCEPT: OBJECT LIFETIME AND MEMORY MANAGEMENT (RAII)
        // Why: On desktop, the Game object is created with automatic storage duration on the stack.
        // Under WebAssembly, static storage keeps the engine persistent across browser frames.
#ifdef __EMSCRIPTEN__
        static Game game(960, 640, "Dungeon Reborn - College OOP Project");
        g_activeGame = &game;
#else
        Game game(960, 640, "Dungeon Reborn - College OOP Project");
#endif
        
        // OOP CONCEPT: ABSTRACTION
        // Why: Calling run() initiates the complete game engine loop without main()
        // knowing any internal rendering, event handling, or audio details.
        game.run();

    } catch (const std::exception& ex) {
        std::cerr << "[Fatal Error] Unhandled exception: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[Fatal Error] Unknown exception occurred." << std::endl;
        return 2;
    }

    return 0;
}

