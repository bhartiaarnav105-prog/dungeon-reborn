#pragma once

#include "entities/Enemy.hpp"
#include "entities/Skeleton.hpp"
#include "entities/Goblin.hpp"
#include "entities/Archer.hpp"
#include "entities/Boss.hpp"
#include <memory>
#include <string>

// ============================================================
// OOP CONCEPT: CLASSES AND OBJECTS (SCOPED ENUM)
// Why: EnemyType enumerates all concrete enemy species in the game
// without relying on error-prone magic numbers or strings.
// ============================================================
enum class EnemyType {
    Skeleton,
    Goblin,
    Archer,
    Boss
};

// ============================================================
// OOP CONCEPT: FACTORY PATTERN & ABSTRACTION
// Why: EnemyFactory centralizes and abstracts the creation of diverse
// concrete Enemy derived classes (Skeleton, Goblin, Archer, Boss).
// Game code requests enemies via EnemyType and receives an owning
// std::unique_ptr<Enemy> base pointer, decoupling room population
// from concrete constructor parameters.
//
// OOP CONCEPT: RUNTIME POLYMORPHISM
// Why: The return type is std::unique_ptr<Enemy> (base class pointer).
// Callers interact exclusively with the polymorphic Enemy interface.
// ============================================================
class EnemyFactory {
public:
    // Pure static utility class — prevent instantiation
    EnemyFactory() = delete;

    // ============================================================
    // OOP CONCEPT: FACTORY METHOD (STATIC FACTORY)
    // Why: Instantiates the appropriate derived subclass based on type
    // and returns it upcast to std::unique_ptr<Enemy>.
    // ============================================================
    static std::unique_ptr<Enemy> createEnemy(EnemyType type,
                                             const sf::Texture& texture,
                                             sf::Vector2f position);

    // Helper: string to EnemyType conversion for parsing
    static EnemyType stringToType(const std::string& typeName);
};

