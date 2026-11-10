#include "entities/EnemyFactory.hpp"
#include <iostream>

// ============================================================
// OOP CONCEPT: FACTORY PATTERN & RUNTIME POLYMORPHISM
// Why: Centralized creation site for all Enemy subclasses.
// Dynamic allocation wraps concrete instances into std::unique_ptr<Enemy>
// ensuring deterministic RAII lifecycle management and polymorphism.
// ============================================================
std::unique_ptr<Enemy> EnemyFactory::createEnemy(EnemyType type,
                                                 const sf::Texture& texture,
                                                 sf::Vector2f position) {
    switch (type) {
        case EnemyType::Skeleton:
            return std::make_unique<Skeleton>(texture, position);

        case EnemyType::Goblin:
            return std::make_unique<Goblin>(texture, position);

        case EnemyType::Archer:
            return std::make_unique<Archer>(texture, position);

        case EnemyType::Boss:
            return std::make_unique<Boss>(texture, position);
    }
    // Fallback: default to Skeleton if unexpected enum value
    return std::make_unique<Skeleton>(texture, position);
}

EnemyType EnemyFactory::stringToType(const std::string& typeName) {
    if (typeName == "Goblin") return EnemyType::Goblin;
    if (typeName == "Archer") return EnemyType::Archer;
    if (typeName == "Boss" || typeName == "DemonBoss") return EnemyType::Boss;
    return EnemyType::Skeleton;
}

