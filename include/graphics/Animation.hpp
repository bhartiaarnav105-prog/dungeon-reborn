#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// OOP CONCEPT: CLASSES AND OBJECTS
// Why: Animation encapsulates the logic for cycling through sprite sheet frame rectangles
// based on elapsed time, representing an animation clip as a distinct object.
class Animation {
// OOP CONCEPT: ACCESS SPECIFIERS (PRIVATE)
// Why: The internal frame collection, timer, and current index are encapsulated
// to prevent external code from corrupting animation playback timing.
private:
    std::vector<sf::IntRect> m_frames;
    size_t m_currentFrameIndex;
    float m_frameDuration;
    float m_elapsedTime;
    bool m_isLooping;

// OOP CONCEPT: ACCESS SPECIFIERS (PUBLIC)
// Why: Provides a clean, well-defined public API for adding frames, ticking time,
// and querying the active frame rectangle.
public:
    // OOP CONCEPT: CONSTRUCTORS AND DESTRUCTORS
    // Why: Parameterized constructor initializes the animation with a specific frame
    // rate and looping policy. Destructor is defaulted via RAII.
    explicit Animation(float frameDuration = 0.15f, bool isLooping = true);
    ~Animation() = default;

    // OOP CONCEPT: ENCAPSULATION
    // Why: Adding frames and advancing time is strictly handled by member functions
    // that maintain valid internal frame indices.
    void addFrame(const sf::IntRect& frameRect);
    void update(float deltaTime);
    void reset();

    // OOP CONCEPT: CONST-CORRECTNESS AND APPROPRIATE USE OF REFERENCES
    // Why: Query methods do not modify internal animation state and return
    // const reference to avoid unnecessary memory copying.
    const sf::IntRect& getCurrentFrameRect() const;
    size_t getFrameCount() const;
    size_t getCurrentFrameIndex() const;
    bool isFinished() const;

    void setFrameDuration(float duration);
    float getFrameDuration() const;
};
