#pragma once

#include "interactables/Interactable.hpp"
#include "items/Item.hpp"
#include "items/LootDrop.hpp"
#include "core/SoundManager.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

// ============================================================
// OOP CONCEPT: INHERITANCE
// Why: TreasureChest IS-AN Interactable. It inherits the abstract
// Interactable interface and implements its own polymorphic interaction,
// rendering, and collision bounds.
//
// OOP CONCEPT: COMPOSITION
// Why: TreasureChest HAS-A list of Item objects (m_lootItems) and gold currency.
//
// OOP CONCEPT: ENCAPSULATION
// Why: Loot contents and opened state are private. External systems open
// the chest through interact() or open(), which enforces the invariant
// that a chest cannot be looted twice.
// ============================================================
class TreasureChest : public Interactable {
private:
    sf::Vector2f                        m_position;
    bool                                m_isOpen;
    std::vector<std::shared_ptr<Item>>  m_lootItems;
    int                                 m_gold;
    sf::Sprite                          m_sprite;

public:
    // ============================================================
    // OOP CONCEPT: CONSTRUCTORS
    // Why: Initializes chest position, closed sprite, and stored loot.
    // ============================================================
    TreasureChest(sf::Vector2f position,
                  const sf::Texture& tileset,
                  int gold = 50,
                  std::vector<std::shared_ptr<Item>> items = {});

    ~TreasureChest() override = default;

    // ============================================================
    // OOP CONCEPT: RUNTIME POLYMORPHISM & METHOD OVERRIDING
    // Why: Implements Interactable::interact(player). Transfers stored gold
    // and items to the player, changes sprite to open chest, and logs rewards.
    // ============================================================
    void interact(Player& player) override;

    void update(float deltaTime, Player& player, SoundManager* soundMgr = nullptr) override;

    // Direct drop-spawning open method
    bool open(std::vector<LootDrop>& outDrops,
              const sf::Texture& tileset,
              SoundManager* soundMgr = nullptr);

    void render(sf::RenderTarget& target) const override;

    sf::FloatRect getBounds() const override { return getGlobalBounds(); }
    sf::FloatRect getGlobalBounds() const;
    sf::Vector2f  getPosition() const { return m_position; }
    bool          isOpen() const { return m_isOpen; }

    std::string getPrompt() const override;
};
