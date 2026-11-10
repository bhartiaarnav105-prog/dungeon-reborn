#include "entities/Player.hpp"
#include "items/Item.hpp"
#include "items/Weapon.hpp"
#include "items/Potion.hpp"
#include "items/Inventory.hpp"
#include "items/LootDrop.hpp"
#include "items/TreasureChest.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

// ============================================================
// OOP CONCEPT: ABSTRACTION
// Why: Encapsulates test suites into modular functions.
// ============================================================

void testItemAbstractionAndPolymorphism() {
    std::cout << "  [Test 1] Item abstraction, cloning, and type queries...\n";

    std::shared_ptr<Item> weapon = Weapon::createRustyDagger();
    std::shared_ptr<Item> potion = Potion::createHealthPotion();

    assert(weapon->getItemType() == "Weapon");
    assert(potion->getItemType() == "Potion");
    assert(!weapon->isStackable());
    assert(potion->isStackable());

    // Polymorphic clone
    auto cloneWeapon = weapon->clone();
    assert(cloneWeapon->getId() == weapon->getId());
    assert(cloneWeapon->getName() == weapon->getName());

    std::cout << "  [PASS] Item abstraction and polymorphic cloning\n";
}

void testInventoryCapacityAndStacking() {
    std::cout << "  [Test 2] Inventory capacity enforcement and item stacking...\n";

    Inventory inv(3); // Small capacity for strict testing
    assert(inv.getCapacity() == 3);
    assert(inv.getItemCount() == 0);
    assert(!inv.isFull());

    // Add stackable potion
    auto pot = Potion::createHealthPotion();
    bool added = inv.addItem(pot, 2);
    assert(added && inv.getItemCount() == 1);
    assert(inv.getSlot(0)->quantity == 2);

    // Add more of same stackable potion -> should stack into slot 0
    added = inv.addItem(pot, 3);
    assert(added && inv.getItemCount() == 1 && "Stackable item must not take new slot!");
    assert(inv.getSlot(0)->quantity == 5);

    // Add 2 non-stackable weapons to fill remaining slots
    inv.addItem(Weapon::createRustyDagger(), 1);
    inv.addItem(Weapon::createIronSword(), 1);
    assert(inv.isFull() && inv.getItemCount() == 3);

    // Attempting to add a 4th unique item must fail (capacity enforcement)
    bool overfill = inv.addItem(Weapon::createGoldenBroadsword(), 1);
    assert(!overfill && "Adding beyond capacity must return false!");
    assert(inv.getItemCount() == 3);

    std::cout << "  [PASS] Inventory capacity & stacking\n";
}

void testWeaponEquippingAndStats() {
    std::cout << "  [Test 3] Weapon equipping and stat modification...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Player player(dummy, sf::Vector2f(100.f, 100.f));

    // Player starts with Iron Sword (25 damage, 0.35s cooldown)
    assert(player.getEquippedWeapon() != nullptr);
    assert(player.getAttackDamage() == 25);
    assert(std::abs(player.getAttackCooldown() - 0.35f) < 0.01f);

    // Equip Rusty Dagger (15 damage, 0.22s cooldown)
    auto dagger = Weapon::createRustyDagger();
    player.equipWeapon(dagger);
    assert(player.getEquippedWeapon()->getId() == "rusty_dagger");
    assert(player.getAttackDamage() == 15);
    assert(std::abs(player.getAttackCooldown() - 0.22f) < 0.01f);

    // Equip Golden Broadsword (45 damage, 0.45s cooldown)
    auto golden = Weapon::createGoldenBroadsword();
    player.equipWeapon(golden);
    assert(player.getEquippedWeapon()->getId() == "golden_broadsword");
    assert(player.getAttackDamage() == 45);
    assert(std::abs(player.getAttackCooldown() - 0.45f) < 0.01f);

    std::cout << "  [PASS] Weapon equipping and dynamic stat binding\n";
}

void testPotionUsageAndConsumption() {
    std::cout << "  [Test 4] Potion healing, consumption, and overheal prevention...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Player player(dummy, sf::Vector2f(100.f, 100.f));
    player.getInventory().clear();

    // Damage player to 50/100 HP
    player.takeDamage(50);
    assert(player.getHealth() == 50);

    // Add 2 health potions (heals 40 HP each)
    player.getInventory().addItem(Potion::createHealthPotion(), 2);
    assert(player.getInventory().getSlot(0)->quantity == 2);

    // Use 1 potion -> heals 40 HP to 90 HP, quantity decrements to 1
    bool used = player.getInventory().useItem(0, player);
    assert(used);
    assert(player.getHealth() == 90);
    assert(player.getInventory().getSlot(0)->quantity == 1);

    // Use 2nd potion -> heals to 100 HP (clamped to maxHealth), slot is removed!
    used = player.getInventory().useItem(0, player);
    assert(used);
    assert(player.getHealth() == 100);
    assert(player.getInventory().getItemCount() == 0 && "Depleted slot must be removed!");

    // Add a potion while at 100 HP; using it should return false (prevent waste)
    player.getInventory().addItem(Potion::createHealthPotion(), 1);
    used = player.getInventory().useItem(0, player);
    assert(!used && "Cannot consume healing potion while at full health!");
    assert(player.getInventory().getSlot(0)->quantity == 1 && "Item must not be consumed on failed use!");

    std::cout << "  [PASS] Potion healing, consumption, and safety checks\n";
}

void testLootDropCollection() {
    std::cout << "  [Test 5] LootDrop world pickup and currency collection...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    Player player(dummy, sf::Vector2f(100.f, 100.f));
    int initialGold = player.getInventory().getGold();

    // Gold drop of 50 coins
    LootDrop goldDrop(sf::Vector2f(100.f, 100.f), 50, dummy);
    assert(goldDrop.isGold());
    assert(!goldDrop.isCollected());

    bool collected = goldDrop.collect(player);
    assert(collected);
    assert(goldDrop.isCollected());
    assert(player.getInventory().getGold() == initialGold + 50);

    // Item drop (Rusty Dagger)
    LootDrop itemDrop(sf::Vector2f(100.f, 100.f), Weapon::createRustyDagger(), 1, dummy);
    assert(!itemDrop.isGold());
    collected = itemDrop.collect(player);
    assert(collected && itemDrop.isCollected());

    std::cout << "  [PASS] LootDrop collection and currency\n";
}

void testTreasureChestMechanics() {
    std::cout << "  [Test 6] TreasureChest opening and loot dispensing...\n";

    sf::Texture dummy;
    dummy.create(64, 64);

    std::vector<std::shared_ptr<Item>> chestItems;
    chestItems.push_back(Potion::createHealthPotion());
    chestItems.push_back(Weapon::createRustyDagger());

    TreasureChest chest(sf::Vector2f(200.f, 200.f), dummy, 40, chestItems);
    assert(!chest.isOpen());

    std::vector<LootDrop> worldDrops;

    // Open chest
    bool opened = chest.open(worldDrops, dummy);
    assert(opened && chest.isOpen());
    assert(worldDrops.size() == 3 && "Chest should drop 1 gold pile + 2 items = 3 drops");

    // Cannot open again
    bool reOpened = chest.open(worldDrops, dummy);
    assert(!reOpened && "Cannot open an already opened chest!");

    std::cout << "  [PASS] TreasureChest opening and loot dispensing\n";
}

void testInvalidOperations() {
    std::cout << "  [Test 7] Invalid operations and bounds safety...\n";

    Inventory inv(4);

    // Negative or zero quantity
    assert(!inv.addItem(Weapon::createIronSword(), 0));
    assert(!inv.addItem(Weapon::createIronSword(), -5));

    // Null item
    assert(!inv.addItem(nullptr, 1));

    // Out of bounds removal and usage
    assert(!inv.removeItem(99, 1));
    sf::Texture dummy;
    dummy.create(64, 64);
    Player player(dummy, sf::Vector2f(0.f, 0.f));
    assert(!inv.useItem(99, player));

    std::cout << "  [PASS] Invalid operations and boundary enforcement\n";
}

int main() {
    std::cout << "\n========================================\n";
    std::cout << "    PHASE 5 INVENTORY & LOOT TESTS      \n";
    std::cout << "========================================\n\n";

    testItemAbstractionAndPolymorphism();
    testInventoryCapacityAndStacking();
    testWeaponEquippingAndStats();
    testPotionUsageAndConsumption();
    testLootDropCollection();
    testTreasureChestMechanics();
    testInvalidOperations();

    std::cout << "\n========================================\n";
    std::cout << "   ALL PHASE 5 INVENTORY TESTS PASSED!  \n";
    std::cout << "========================================\n\n";
    return 0;
}

