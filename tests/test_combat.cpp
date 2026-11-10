#include "entities/Character.hpp"
#include "entities/Player.hpp"
#include "entities/Enemy.hpp"
#include "entities/Skeleton.hpp"
#include "entities/Goblin.hpp"
#include "entities/Boss.hpp"
#include "entities/Projectile.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <cmath>

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: Test runner encapsulates assertions into distinct test units.
// ============================================================

void testPlayerAttackMechanics() {
    std::cout << "  [Test 1] Player directional attack hitbox & single-hit flag...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Player player(dummy, sf::Vector2f(200.f, 200.f));

    // Initially not attacking
    assert(!player.isAttacking());
    assert(!player.canDeliverHit());

    // Facing right attack hitbox
    player.setFacingDirection(Direction::Right);
    sf::FloatRect rightHitbox = player.getAttackHitbox();
    assert(rightHitbox.left >= player.getPosition().x && "Right hitbox must extend to the right of player");

    // Facing left attack hitbox
    player.setFacingDirection(Direction::Left);
    sf::FloatRect leftHitbox = player.getAttackHitbox();
    assert(leftHitbox.left < player.getPosition().x && "Left hitbox must extend to the left of player");

    // Initiate attack
    player.performAttack();
    assert(player.isAttacking());
    assert(player.getState() == CharacterState::Attacking);
    assert(player.getAttackCooldownTimer() > 0.0f);

    // Update slightly so attack frame is active
    player.update(0.05f);
    assert(player.canDeliverHit() && "Attack should be ready to deliver hit");

    // Mark hit delivered
    player.markAttackHitDelivered();
    assert(!player.canDeliverHit() && "Controlled single-hit: must NOT deliver hit twice in one attack!");

    // Update through remainder of attack duration
    player.update(0.3f);
    assert(!player.isAttacking() && "Attack duration should expire");

    std::cout << "  [PASS] Player directional attack & single-hit delivery\n";
}

void testSkeletonCombat() {
    std::cout << "  [Test 2] Skeleton close-range attack and cooldown...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Player player(dummy, sf::Vector2f(200.f, 200.f));
    Skeleton skeleton(dummy, sf::Vector2f(210.f, 200.f)); // 10px away (melee range)

    assert(skeleton.getAttackDamage() == 12);
    assert(player.getHealth() == 100);

    std::vector<Projectile> projectiles;

    // Skeleton attacks player in melee range
    skeleton.updateCombat(0.01f, player, projectiles, dummy);
    assert(player.getHealth() == 88 && "Player should take 12 damage from skeleton strike");

    // Next frame skeleton should be on cooldown (1.2s), no double damage
    skeleton.updateCombat(0.01f, player, projectiles, dummy);
    assert(player.getHealth() == 88 && "Skeleton must respect attack cooldown!");

    std::cout << "  [PASS] Skeleton close-range combat & cooldown\n";
}

void testGoblinTacticCombat() {
    std::cout << "  [Test 3] Goblin hit-and-run skirmish tactics...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Player player(dummy, sf::Vector2f(300.f, 300.f));
    Goblin goblin(dummy, sf::Vector2f(310.f, 300.f)); // 10px away

    assert(goblin.getAttackDamage() == 8);
    assert(goblin.getSpeed() == 150.0f); // High speed

    std::vector<Projectile> projectiles;

    // Goblin attacks during lunge
    goblin.updateCombat(0.01f, player, projectiles, dummy);
    assert(player.getHealth() == 92 && "Player should take 8 damage from goblin stab");

    // Goblin immediately triggers retreat tactic
    goblin.updateAI(0.05f, player.getPosition());
    assert(goblin.getVelocity().x > 0.0f && "Goblin should retreat away from player to the right");

    std::cout << "  [PASS] Goblin hit-and-run skirmish tactics\n";
}

void testBossMultiPatternCombat() {
    std::cout << "  [Test 4] Boss multi-pattern attacks (telegraphed slam, projectile, enrage)...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Player player(dummy, sf::Vector2f(400.f, 400.f));
    Boss boss(dummy, sf::Vector2f(420.f, 400.f)); // 20px away (melee range)

    assert(boss.getHealth() == 250);
    assert(!boss.isEnraged());

    std::vector<Projectile> projectiles;

    // 1. Melee Ground Slam telegraph trigger
    boss.updateCombat(0.01f, player, projectiles, dummy);
    assert(boss.isWindingUp() && "Boss should begin telegraph windup before melee slam");

    // Player takes no damage yet while boss is telegraphing
    assert(player.getHealth() == 100);

    // Advance through windup (0.55s)
    boss.updateCombat(0.6f, player, projectiles, dummy);
    assert(!boss.isWindingUp() && "Windup should complete");
    assert(player.getHealth() == 75 && "Player should take 25 slam damage after windup completes");

    // 2. Ranged Fiery Shockwave: move player to mid-range (150px away)
    Player farPlayer(dummy, sf::Vector2f(400.f + 150.f, 400.f));
    boss.updateCombat(3.5f, farPlayer, projectiles, dummy); // advance past ranged cooldown
    assert(!projectiles.empty() && "Boss should spawn fiery shockwave projectile at mid-range");
    assert(projectiles.back().getDamage() == boss.getRangedDamage());

    // 3. Enrage phase trigger
    boss.takeDamage(130);
    assert(boss.getHealth() == 120);
    assert(boss.isEnraged() && "Boss must enter Enraged phase when HP drops <= 50%");
    assert(boss.getSpeed() == 125.0f && "Enraged boss should have boosted speed");
    assert(boss.getAttackDamage() == 35 && "Enraged boss should deal 35 melee damage");

    std::cout << "  [PASS] Boss multi-pattern attacks & Enrage phase\n";
}

void testProjectilePhysics() {
    std::cout << "  [Test 5] Projectile motion, lifespan, and destruction...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Projectile p(sf::Vector2f(100.f, 100.f), sf::Vector2f(1.f, 0.f), 200.f, 15, 1.0f, dummy, sf::IntRect(0, 0, 16, 16));
    assert(p.isAlive());
    assert(p.getDamage() == 15);

    // Update 0.5s -> moved 100px along X
    p.update(0.5f);
    assert(p.isAlive());
    assert(std::abs(p.getPosition().x - 200.f) < 1.0f);

    // Update past lifetime
    p.update(0.6f);
    assert(!p.isAlive() && "Projectile must expire when lifetime exceeds maxLifetime");

    std::cout << "  [PASS] Projectile motion and expiration\n";
}

void testPlayerDeathAndDamageFeedback() {
    std::cout << "  [Test 6] Player damage feedback and death state...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Player player(dummy, sf::Vector2f(100.f, 100.f));

    // Take damage
    player.takeDamage(40);
    assert(player.getHealth() == 60);
    assert(player.isHurt() && "Hurt feedback timer should be active");

    // Tick feedback timer
    player.updateDamageFeedback(0.2f);
    assert(!player.isHurt() && "Hurt feedback should clear after duration");

    // Fatal damage
    player.takeDamage(60);
    assert(player.getHealth() == 0);
    assert(!player.isAlive());
    assert(player.getState() == CharacterState::Dead);

    std::cout << "  [PASS] Player damage feedback and death\n";
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM TEST
// Why: Demonstrates invoking updateCombat() through a heterogeneous
// container of Enemy* pointers without inspecting concrete types.
// ============================================================
void testPolymorphicCombatDispatch() {
    std::cout << "  [Test 7] Heterogeneous container polymorphic combat dispatch...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Player player(dummy, sf::Vector2f(100.f, 100.f));
    std::vector<std::unique_ptr<Enemy>> enemyRoster;
    enemyRoster.push_back(std::make_unique<Skeleton>(dummy, sf::Vector2f(105.f, 100.f)));
    enemyRoster.push_back(std::make_unique<Goblin>(dummy, sf::Vector2f(105.f, 100.f)));
    enemyRoster.push_back(std::make_unique<Boss>(dummy, sf::Vector2f(105.f, 100.f)));

    std::vector<Projectile> projectiles;

    // Execute polymorphic combat tick through base class pointers
    for (auto& enemy : enemyRoster) {
        enemy->updateCombat(0.01f, player, projectiles, dummy);
    }

    // Dynamic binding correctly executed without needing any switch statements!
    assert(enemyRoster[0]->getCharacterType() == "Skeleton");
    assert(enemyRoster[1]->getCharacterType() == "Goblin");
    assert(enemyRoster[2]->getCharacterType() == "DemonBoss");

    std::cout << "  [PASS] Polymorphic combat dispatch through base pointers\n";
}

int main() {
    std::cout << "\n========================================\n";
    std::cout << "    PHASE 4 COMBAT & AI UNIT TESTS      \n";
    std::cout << "========================================\n\n";

    testPlayerAttackMechanics();
    testSkeletonCombat();
    testGoblinTacticCombat();
    testBossMultiPatternCombat();
    testProjectilePhysics();
    testPlayerDeathAndDamageFeedback();
    testPolymorphicCombatDispatch();

    std::cout << "\n========================================\n";
    std::cout << "   ALL PHASE 4 COMBAT TESTS PASSED!     \n";
    std::cout << "========================================\n\n";
    return 0;
}

