#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

// OOP CONCEPT: CLASSES AND OBJECTS
// Why: ResourceManager encapsulates resource storage and retrieval logic,
// treating textures, fonts, and sound buffers as managed objects.
class ResourceManager {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Internal data structures (maps holding the loaded resources) are hidden
// to enforce encapsulation and prevent uncontrolled external modifications.
private:
    std::unordered_map<std::string, sf::Texture> m_textures;
    std::unordered_map<std::string, sf::Font> m_fonts;
    std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers;

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Public interface allows client code to request resources cleanly
// while hiding the underlying storage and file I/O details.
public:
    // OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
    // Why: Default constructor initializes the resource manager in a valid, empty state.
    // Destructor (defaulted) guarantees clean RAII teardown of all loaded assets.
    ResourceManager();
    ~ResourceManager() = default;

    // Disallow copying to maintain strict resource ownership (RAII)
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Allow moving for flexible lifetime management
    ResourceManager(ResourceManager&&) noexcept = default;
    ResourceManager& operator=(ResourceManager&&) noexcept = default;

    // OOP CONCEPT: ENCAPSULATION
    // Why: Resource loading verifies existence, handles errors, and stores assets internally.
    // Callers cannot corrupt internal maps or load duplicates unnecessarily.
    bool loadTexture(const std::string& id, const std::string& filepath);
    bool loadFont(const std::string& id, const std::string& filepath);
    bool loadSoundBuffer(const std::string& id, const std::string& filepath);

    // OOP CONCEPT: CONST-CORRECTNESS AND REFERENCES
    // Why: Returning by const reference avoids expensive copies of heavy graphical/audio buffers,
    // while const member function guarantees retrieving a resource doesn't mutate manager state.
    const sf::Texture& getTexture(const std::string& id) const;
    const sf::Font& getFont(const std::string& id) const;
    const sf::SoundBuffer& getSoundBuffer(const std::string& id) const;

    // OOP CONCEPT: CONST-CORRECTNESS
    // Why: Query methods do not modify internal state, strictly marked const.
    bool hasTexture(const std::string& id) const;
    bool hasFont(const std::string& id) const;
    bool hasSoundBuffer(const std::string& id) const;
};

