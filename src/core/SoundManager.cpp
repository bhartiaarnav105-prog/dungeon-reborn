#include "core/SoundManager.hpp"
#include <iostream>
#include <algorithm>

// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Member initialization list directly initializes the const reference
// and sets up default voice pool capacity.
SoundManager::SoundManager(const ResourceManager& resourceManager, size_t maxChannels)
    : m_resourceManager(resourceManager)
    , m_maxChannels(maxChannels)
    , m_volume(70.0f)
    , m_muted(false) {
}

// OOP CONCEPT: ENCAPSULATION
// Why: Allocates and reuses sf::Sound instances from an internal pool to prevent
// memory spikes and ensure voices do not cut off prematurely.
void SoundManager::playSound(const std::string& soundId) {
    if (m_muted) {
        return;
    }

    if (!m_resourceManager.hasSoundBuffer(soundId)) {
        std::cerr << "[SoundManager] Warning: Sound buffer '" << soundId << "' not found." << std::endl;
        return;
    }

    const sf::SoundBuffer& buffer = m_resourceManager.getSoundBuffer(soundId);

    // Reuse a stopped voice channel if available
    for (auto& sound : m_soundPool) {
        if (sound.getStatus() == sf::Sound::Stopped) {
            sound.setBuffer(buffer);
            sound.setVolume(m_volume);
            sound.play();
            return;
        }
    }

    // If all channels are busy and pool has room, allocate a new channel
    if (m_soundPool.size() < m_maxChannels) {
        m_soundPool.emplace_back();
        m_soundPool.back().setBuffer(buffer);
        m_soundPool.back().setVolume(m_volume);
        m_soundPool.back().play();
    } else {
        // Pool is full: overwrite the first channel
        m_soundPool[0].stop();
        m_soundPool[0].setBuffer(buffer);
        m_soundPool[0].setVolume(m_volume);
        m_soundPool[0].play();
    }
}

// OOP CONCEPT: ENCAPSULATION
// Why: Modifying volume clamps the value safely to [0, 100] and propagates
// it across all currently playing voice channels.
void SoundManager::setVolume(float volume) {
    m_volume = std::max(0.0f, std::min(100.0f, volume));
    for (auto& sound : m_soundPool) {
        sound.setVolume(m_volume);
    }
}

// OOP CONCEPT: CONST-CORRECTNESS
// Why: Read-only query function marked const.
float SoundManager::getVolume() const {
    return m_volume;
}

void SoundManager::setMuted(bool muted) {
    m_muted = muted;
    if (m_muted) {
        stopAll();
    }
}

bool SoundManager::isMuted() const {
    return m_muted;
}

void SoundManager::stopAll() {
    for (auto& sound : m_soundPool) {
        sound.stop();
    }
}

