cmake_minimum_required(VERSION 3.20)
project(DungeonReborn CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release optimization mode across native and WebAssembly builds
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Global optimization flags
if(EMSCRIPTEN)
    add_compile_options(-O3 -DNDEBUG)
endif()

# Static link GCC runtime for self-contained binaries on Windows
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

# Fetch SFML (Desktop: upstream 2.6.2; WebAssembly: Emscripten-ready port)
include(FetchContent)
if(EMSCRIPTEN)
    message(STATUS "[DungeonReborn] Configuring for WebAssembly (Emscripten)")
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/Zombieschannel/SFML.git
        GIT_TAG 2.6.x-EMCC
    )
else()
    message(STATUS "[DungeonReborn] Configuring for Desktop (Native)")
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.2
    )
endif()

set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML)

# Core game library sources
set(GAME_CORE_SOURCES
    src/core/ResourceManager.cpp
    src/core/SoundManager.cpp
    src/core/GameState.cpp
    src/core/Game.cpp
    src/graphics/Animation.cpp
    src/graphics/HealthBar.cpp
    src/entities/Character.cpp
    src/entities/Player.cpp
    src/entities/Enemy.cpp
    src/entities/Skeleton.cpp
    src/entities/Goblin.cpp
    src/entities/Archer.cpp
    src/entities/Boss.cpp
    src/entities/EnemyFactory.cpp
    src/entities/Projectile.cpp
    # Phase 3: Dungeon environment system
    src/dungeon/Tile.cpp
    src/dungeon/TileMap.cpp
    src/dungeon/Room.cpp
    src/dungeon/Minimap.cpp
    src/dungeon/DungeonGenerator.cpp
    src/dungeon/CollisionSystem.cpp
    src/dungeon/CameraController.cpp
    src/dungeon/Dungeon.cpp
    # Phase 5: Items, weapons, inventory, and loot
    src/items/Item.cpp
    src/items/Weapon.cpp
    src/items/Sword.cpp
    src/items/Bow.cpp
    src/items/MagicWeapon.cpp
    src/items/Potion.cpp
    src/items/Key.cpp
    src/items/Inventory.cpp
    src/items/LootDrop.cpp
    src/items/TreasureChest.cpp
    # Phase 2: Interactables, keys, doors, and traps
    src/interactables/Door.cpp
    src/interactables/KeyPickup.cpp
    src/interactables/Trap.cpp
    src/interactables/SpikeTrap.cpp
    src/interactables/FireTrap.cpp
)

# Static library for core game components
add_library(DungeonCore STATIC ${GAME_CORE_SOURCES})
target_include_directories(DungeonCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(DungeonCore PUBLIC
    sfml-graphics
    sfml-audio
    sfml-window
    sfml-system
)

# Main game executable
add_executable(DungeonReborn src/main.cpp)
target_link_libraries(DungeonReborn PRIVATE DungeonCore)

if(EMSCRIPTEN)
    target_compile_options(DungeonCore PUBLIC -pthread)
    target_compile_options(DungeonReborn PRIVATE -pthread)
    # WebAssembly target properties & linker flags
    set_target_properties(DungeonReborn PROPERTIES SUFFIX ".html")
    target_link_options(DungeonReborn PRIVATE
        "-O3"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES2=1"
        "-sINITIAL_MEMORY=67108864"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sUSE_PTHREADS=1"
        "-pthread"
        "--use-port=freetype"
        "--use-port=ogg"
        "--use-port=vorbis"
        "-sSTACK_SIZE=4mb"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
        "-sEXPORTED_FUNCTIONS=['_main','_startGameFromWeb']"
        "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
        "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
    )
else()
    # Copy assets directory and OpenAL DLL to binary output directory (Desktop only)
    add_custom_command(TARGET DungeonReborn POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:DungeonReborn>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/_deps/sfml-src/extlibs/bin/x64/openal32.dll"
            "$<TARGET_FILE_DIR:DungeonReborn>/openal32.dll"
        COMMENT "Copying game assets and OpenAL DLL to build output directory"
    )

    # Unit test executables (Desktop validation test suites)
    add_executable(DungeonTests tests/test_character_hierarchy.cpp)
    target_link_libraries(DungeonTests PRIVATE DungeonCore)

    add_executable(DungeonTestsPlayer tests/test_player.cpp)
    target_link_libraries(DungeonTestsPlayer PRIVATE DungeonCore)

    add_executable(DungeonTests3 tests/test_dungeon.cpp)
    target_link_libraries(DungeonTests3 PRIVATE DungeonCore)

    add_executable(DungeonTests4 tests/test_combat.cpp)
    target_link_libraries(DungeonTests4 PRIVATE DungeonCore)

    add_executable(DungeonTests5 tests/test_inventory.cpp)
    target_link_libraries(DungeonTests5 PRIVATE DungeonCore)

    add_executable(DungeonTests6 tests/test_game_loop.cpp)
    target_link_libraries(DungeonTests6 PRIVATE DungeonCore)

    add_executable(DungeonTestsExploration tests/test_exploration.cpp)
    target_link_libraries(DungeonTestsExploration PRIVATE DungeonCore)

    add_executable(DungeonTestsEnemyAI tests/test_enemy_ai.cpp)
    target_link_libraries(DungeonTestsEnemyAI PRIVATE DungeonCore)

    add_executable(DungeonTestsPhase4 tests/test_phase4_polish.cpp)
    target_link_libraries(DungeonTestsPhase4 PRIVATE DungeonCore)
endif()
