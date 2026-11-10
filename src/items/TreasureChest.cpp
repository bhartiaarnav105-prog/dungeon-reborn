#include "items/TreasureChest.hpp"
#include "entities/Player.hpp"
#include <iostream>
#include <cmath>

// ============================================================
// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Initializes Interactable base class with tag "TreasureChest"
// and sets closed chest texture rect (304, 288, 16, 16).
// ============================================================
TreasureChest::TreasureChest(sf::Vector2f position,
                             const sf::Texture& tileset,
                             int gold,
                             std::vector<std::shared_ptr<Item>> items)
    : Interactable("TreasureChest")
    , m_position(position)
    , m_isOpen(false)
    , m_lootItems(std::move(items))
    , m_gold(gold)
    , m_sprite(tileset, sf::IntRect(304, 288, 16, 16)) {

    m_sprite.setOrigin(8.0f, 8.0f);
    m_sprite.setScale(3.0f, 3.0f);
    m_sprite.setPosition(m_position);
}

// ============================================================
// OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
// Why: Overrides Interactable::interact(player). Grants contents directly
// to player inventory and gold purse when triggered.
// ============================================================
void TreasureChest::interact(Player& player) {
    if (m_isOpen) return;

    m_isOpen = true;
    m_sprite.setTextureRect(sf::IntRect(320, 288, 16, 16)); // Open chest sprite

    if (m_gold > 0) {
        player.getInventory().addGold(m_gold);
        std::cout << "[TreasureChest] Plundered " << m_gold << " Gold!\n";
        m_gold = 0;
    }

    for (const auto& item : m_lootItems) {
        if (item) {
            player.getInventory().addItem(item, 1);
            std::cout << "[TreasureChest] Received " << item->getName() << "!\n";
        }
    }
    m_lootItems.clear();
}

void TreasureChest::update(float deltaTime, Player& player, SoundManager* soundMgr) {
    (void)deltaTime;
    if (!m_isOpen && player.isAlive()) {
        float dist = std::hypot(m_position.x - player.getPosition().x,
                                m_position.y - player.getPosition().y);
        // [E] key or close proximity opens chest
        if (dist <= 65.0f && (sf::Keyboard::isKeyPressed(sf::Keyboard::E) || dist <= 38.0f)) {
            interact(player);
            if (soundMgr) soundMgr->playSound("chest");
        }
    }
}

// ============================================================
// OOP CONCEPT: ENCAPSULATION
// Why: Enforces invariants: once opened, the chest changes texture rect
// to (320, 288, 16, 16) and ejects its contents as LootDrop entities.
// ============================================================
bool TreasureChest::open(std::vector<LootDrop>& outDrops,
                         const sf::Texture& tileset,
                         SoundManager* soundMgr) {
    if (m_isOpen) return false;

    m_isOpen = true;
    m_sprite.setTextureRect(sf::IntRect(320, 288, 16, 16)); // Open chest sprite

    if (soundMgr) {
        soundMgr->playSound("chest");
    }

    std::cout << "[TreasureChest] Chest popped open!\n";

    // Spawn gold drop
    if (m_gold > 0) {
        outDrops.emplace_back(
            sf::Vector2f(m_position.x - 24.0f, m_position.y + 20.0f),
            m_gold,
            tileset
        );
    }

    // Spawn item drops
    float offsetX = 0.0f;
    for (const auto& item : m_lootItems) {
        if (item) {
            outDrops.emplace_back(
                sf::Vector2f(m_position.x + offsetX, m_position.y + 24.0f),
                item,
                1,
                tileset
            );
            offsetX += 24.0f;
        }
    }

    m_lootItems.clear();
    m_gold = 0;
    return true;
}

void TreasureChest::render(sf::RenderTarget& target) const {
    target.draw(m_sprite);
}

sf::FloatRect TreasureChest::getGlobalBounds() const {
    return m_sprite.getGlobalBounds();
}

std::string TreasureChest::getPrompt() const {
    return m_isOpen ? "" : "[E] Open Chest";
}
