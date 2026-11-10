#include "interactables/Interactable.hpp"
#include "interactables/Door.hpp"
#include "interactables/Trap.hpp"
#include "interactables/SpikeTrap.hpp"
#include "interactables/FireTrap.hpp"
#include "interactables/KeyPickup.hpp"
#include "items/Key.hpp"
#include "items/Inventory.hpp"
#include "items/TreasureChest.hpp"
#include "entities/Player.hpp"
#include "dungeon/Dungeon.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

void testInteractablePolymorphism() {
    std::cout << "[Test 1] Testing Interactable Polymorphism & Abstraction...\n";

    // Dummy texture for visual tests
    sf::Texture dummyTex;
    dummyTex.create(32, 32);

    // OOP CONCEPT: RUNTIME POLYMORPHISM & UPCASTING
    // Collection of base class shared_ptrs storing diverse derived objects
    std::vector<std::shared_ptr<Interactable>> items;

    items.push_back(std::make_shared<Door>("Test Gate", sf::Vector2f(0.f, 0.f), sf::Vector2f(48.f, 48.f),
                                           1, sf::Vector2f(50.f, 50.f), DoorState::Locked, "boss_key", &dummyTex));
    items.push_back(std::make_shared<SpikeTrap>(sf::Vector2f(100.f, 100.f), dummyTex, 10, 1.0f));
    items.push_back(std::make_shared<FireTrap>(sf::Vector2f(200.f, 200.f), dummyTex, 15, 2.0f, 1.0f));
    items.push_back(std::make_shared<KeyPickup>(sf::Vector2f(300.f, 300.f), Key::createBossKey(), dummyTex));
    items.push_back(std::make_shared<TreasureChest>(sf::Vector2f(400.f, 400.f), dummyTex, 50));

    assert(items.size() == 5);

    // Check tags and virtual dispatch
    assert(items[0]->getTag() == "Door");
    assert(items[1]->getTag() == "SpikeTrap");
    assert(items[2]->getTag() == "FireTrap");
    assert(items[3]->getTag() == "KeyPickup");
    assert(items[4]->getTag() == "TreasureChest");

    // Door is locked -> isBlocking() must be true
    assert(items[0]->isBlocking() == true);
    // Traps and pickups do not block movement
    assert(items[1]->isBlocking() == false);
    assert(items[2]->isBlocking() == false);
    assert(items[3]->isBlocking() == false);

    std::cout << "  -> Polymorphic collection verified successfully.\n";
}

void testKeyAndInventoryComposition() {
    std::cout << "[Test 2] Testing Key and Inventory Composition...\n";

    // OOP CONCEPT: INHERITANCE
    // Key IS-AN Item
    std::shared_ptr<Key> bossKey = Key::createBossKey();
    assert(bossKey->getId() == "boss_key");
    assert(bossKey->getItemType() == "Key");
    assert(bossKey->getTargetDoorId() == "boss_door");

    // OOP CONCEPT: COMPOSITION
    // Inventory HAS-A collection of Item objects
    Inventory inv(6);
    assert(!inv.hasItem("boss_key"));

    bool added = inv.addItem(bossKey, 1);
    assert(added);
    assert(inv.hasItem("boss_key"));
    assert(inv.getItemCount() == 1);

    std::shared_ptr<Item> retrieved = inv.getItemById("boss_key");
    assert(retrieved != nullptr);
    assert(retrieved->getName() == "Boss Chamber Key");

    // Key use does not consume the key
    Player dummyPlayer(sf::Texture(), sf::Vector2f(0.f, 0.f));
    bool used = inv.useItem(0, dummyPlayer);
    assert(!used); // key use returns false to keep key in inventory
    assert(inv.hasItem("boss_key")); // still present

    std::cout << "  -> Key and Inventory composition verified successfully.\n";
}

void testDoorLockingAndUnlocking() {
    std::cout << "[Test 3] Testing Door State Machine & Unlocking...\n";

    sf::Texture dummyTex;
    dummyTex.create(32, 32);

    Door door("Boss Gate", sf::Vector2f(100.f, 100.f), sf::Vector2f(48.f, 48.f),
              1, sf::Vector2f(200.f, 200.f), DoorState::Locked, "boss_key", &dummyTex);

    assert(door.isLocked());
    assert(door.isBlocking());
    assert(!door.isOpen());

    Player player(dummyTex, sf::Vector2f(100.f, 100.f));

    // Interact without key -> must stay locked
    door.interact(player);
    assert(door.isLocked());
    assert(door.isBlocking());

    // Give player the key
    player.getInventory().addItem(Key::createBossKey(), 1);
    assert(player.getInventory().hasItem("boss_key"));

    // Interact with key -> must unlock and open
    door.interact(player);
    assert(!door.isLocked());
    assert(door.isOpen());
    assert(!door.isBlocking());

    std::cout << "  -> Door locking and unlocking verified successfully.\n";
}

void testTrapHierarchyAndDamage() {
    std::cout << "[Test 4] Testing Trap Hierarchy (SpikeTrap & FireTrap)...\n";

    sf::Texture dummyTex;
    dummyTex.create(32, 32);

    Player player(dummyTex, sf::Vector2f(50.f, 50.f));
    int initialHp = player.getHealth();

    // 1. SpikeTrap testing
    // OOP CONCEPT: INHERITANCE & RUNTIME POLYMORPHISM
    std::unique_ptr<Trap> spike = std::make_unique<SpikeTrap>(sf::Vector2f(50.f, 50.f), dummyTex, 15, 1.0f);
    assert(spike->getDamage() == 15);
    assert(!spike->isTriggered());

    // Trigger spike trap
    spike->trigger(player);
    assert(spike->isTriggered());
    assert(player.getHealth() == initialHp - 15);

    // Cooldown prevents immediate re-trigger
    spike->trigger(player);
    assert(player.getHealth() == initialHp - 15);

    // 2. FireTrap testing
    // OOP CONCEPT: INHERITANCE & POLYMORPHISM
    auto fire = std::make_unique<FireTrap>(sf::Vector2f(50.f, 50.f), dummyTex, 20, 1.0f, 1.0f);
    assert(fire->getDamage() == 20);
    assert(!fire->isErupting());

    // While dormant, trigger does not inflict damage
    fire->trigger(player);
    assert(player.getHealth() == initialHp - 15);

    // Advance cycle by 1.1s so fire erupts
    fire->update(1.1f, player, nullptr);
    assert(fire->isErupting());

    // Erupting fire damages player
    fire->trigger(player);
    assert(player.getHealth() == initialHp - 15 - 20);

    std::cout << "  -> Trap hierarchy and damage mechanics verified successfully.\n";
}

void testDungeonExplorationLoop() {
    std::cout << "[Test 5] Testing Dungeon Exploration Loop...\n";

    sf::Texture dummyTex;
    dummyTex.create(32, 32);

    Dungeon dungeon(800, 600, &dummyTex);
    assert(dungeon.getRoomCount() >= 3);
    assert(dungeon.getCurrentRoomId() == 0);

    const Room& room0 = dungeon.getCurrentRoom();
    assert(room0.getName() == "Entry Crypt");

    // Verify room 0 contains the locked Boss Door
    bool foundBossDoor = false;
    for (const auto& interactable : room0.getInteractables()) {
        if (auto door = std::dynamic_pointer_cast<Door>(interactable)) {
            if (door->getTargetRoomId() == 1 && door->isLocked()) {
                foundBossDoor = true;
                assert(door->getRequiredKeyId() == "boss_key");
            }
        }
    }
    assert(foundBossDoor);

    // Verify room 2 (Goblin Warren) contains the Boss Key pickup
    const Room* room2 = dungeon.getRoomById(2);
    assert(room2 != nullptr);
    bool foundKeyPickup = false;
    for (const auto& interactable : room2->getInteractables()) {
        if (auto pickup = std::dynamic_pointer_cast<KeyPickup>(interactable)) {
            if (pickup->getKey() && pickup->getKey()->getId() == "boss_key") {
                foundKeyPickup = true;
            }
        }
    }
    assert(foundKeyPickup);

    std::cout << "  -> Dungeon exploration progression verified successfully.\n";
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "PHASE 2: OOP EXPLORATION SYSTEM VERIFICATION SUITE\n";
    std::cout << "=========================================================\n";

    testInteractablePolymorphism();
    testKeyAndInventoryComposition();
    testDoorLockingAndUnlocking();
    testTrapHierarchyAndDamage();
    testDungeonExplorationLoop();

    std::cout << "=========================================================\n";
    std::cout << "ALL PHASE 2 EXPLORATION TESTS PASSED SUCCESSFULLY (5/5)!\n";
    std::cout << "=========================================================\n";
    return 0;
}

