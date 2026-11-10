#pragma once

#include "core/ResourceManager.hpp"
#include <SFML/Audio.hpp>
#include <vector>
#include <string>

// OOP CONCEPT: CLASSES AND OBJECTS
// Why: SoundManager encapsulates the audio subsystem, managing voice channels,
// volume adjustments, and sound buffer bindings.
class SoundManager {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: Sound channels and volume levels are internal state. Modifying volume
// triggers internal updates across all active sound instances.
private:
    // OOP CONCEPT: OBJECT RELATIONSHIPS (ASSOCIATION / REFERENCE)
    // Why: SoundManager depends on ResourceManager for audio buffers but does not
    // own ResourceManager's lifecycle. We store a const reference.
    const ResourceManager& m_resourceManager;

    std::vector<sf::Sound> m_soundPool;
    size_t m_maxChannels;
    float m_volume;
    bool m_muted;

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Exposes methods to trigger sounds, adjust volume, and toggle mute state.
public:
    // OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
    // Why: Explicit constructor requires a valid ResourceManager reference,
    // ensuring SoundManager cannot exist in an uninitialized or dangling state.
    explicit SoundManager(const ResourceManager& resourceManager, size_t maxChannels = 16);
    ~SoundManager() = default;

    // OOP CONCEPT: ENCAPSULATION
    // Why: playSound cleans up inactive channels, assigns a free voice channel,
    // applies the current volume, and plays the sound without exposing sf::Sound details.
    void playSound(const std::string& soundId);

    void setVolume(float volume);
    float getVolume() const;

    void setMuted(bool muted);
    bool isMuted() const;

    void stopAll();
};

