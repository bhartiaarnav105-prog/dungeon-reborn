#include "core/ResourceManager.hpp"
#include <iostream>

// OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
// Why: Initializes the manager with empty lookup tables.
ResourceManager::ResourceManager() {
}

// OOP CONCEPT: ENCAPSULATION
// Why: File loading is wrapped within this method. If loading fails, it reports
// the error cleanly and preserves the invariant that only valid textures are stored.
bool ResourceManager::loadTexture(const std::string& id, const std::string& filepath) {
    if (m_textures.find(id) != m_textures.end()) {
        return true; // Already loaded
    }

    sf::Texture texture;
    if (!texture.loadFromFile(filepath)) {
        std::cerr << "[ResourceManager] Error: Failed to load texture from " << filepath << std::endl;
        return false;
    }

    m_textures.emplace(id, std::move(texture));
    std::cout << "[ResourceManager] Loaded texture: " << id << " from " << filepath << std::endl;
    return true;
}

// OOP CONCEPT: ENCAPSULATION
// Why: Encapsulates font parsing and caching.
bool ResourceManager::loadFont(const std::string& id, const std::string& filepath) {
    if (m_fonts.find(id) != m_fonts.end()) {
        return true;
    }

    sf::Font font;
    if (!font.loadFromFile(filepath)) {
        std::cerr << "[ResourceManager] Error: Failed to load font from " << filepath << std::endl;
        return false;
    }

    m_fonts.emplace(id, std::move(font));
    std::cout << "[ResourceManager] Loaded font: " << id << " from " << filepath << std::endl;
    return true;
}

// OOP CONCEPT: ENCAPSULATION
// Why: Encapsulates audio buffer loading and caching.
bool ResourceManager::loadSoundBuffer(const std::string& id, const std::string& filepath) {
    if (m_soundBuffers.find(id) != m_soundBuffers.end()) {
        return true;
    }

    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filepath)) {
        std::cerr << "[ResourceManager] Error: Failed to load sound buffer from " << filepath << std::endl;
        return false;
    }

    m_soundBuffers.emplace(id, std::move(buffer));
    std::cout << "[ResourceManager] Loaded sound buffer: " << id << " from " << filepath << std::endl;
    return true;
}

// OOP CONCEPT: CONST-CORRECTNESS AND REFERENCES
// Why: Returns a reference to avoid deep copying of large texture memory;
// const prevents callers from inadvertently modifying the stored texture.
const sf::Texture& ResourceManager::getTexture(const std::string& id) const {
    auto it = m_textures.find(id);
    if (it == m_textures.end()) {
        throw std::runtime_error("[ResourceManager] Requested texture not found: " + id);
    }
    return it->second;
}

// OOP CONCEPT: CONST-CORRECTNESS AND REFERENCES
// Why: Provides read-only access to cached fonts.
const sf::Font& ResourceManager::getFont(const std::string& id) const {
    auto it = m_fonts.find(id);
    if (it == m_fonts.end()) {
        throw std::runtime_error("[ResourceManager] Requested font not found: " + id);
    }
    return it->second;
}

// OOP CONCEPT: CONST-CORRECTNESS AND REFERENCES
// Why: Provides read-only access to cached sound buffers.
const sf::SoundBuffer& ResourceManager::getSoundBuffer(const std::string& id) const {
    auto it = m_soundBuffers.find(id);
    if (it == m_soundBuffers.end()) {
        throw std::runtime_error("[ResourceManager] Requested sound buffer not found: " + id);
    }
    return it->second;
}

// OOP CONCEPT: CONST-CORRECTNESS
// Why: Pure state inspection function marked const.
bool ResourceManager::hasTexture(const std::string& id) const {
    return m_textures.find(id) != m_textures.end();
}

bool ResourceManager::hasFont(const std::string& id) const {
    return m_fonts.find(id) != m_fonts.end();
}

bool ResourceManager::hasSoundBuffer(const std::string& id) const {
    return m_soundBuffers.find(id) != m_soundBuffers.end();
}

