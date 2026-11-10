#include "graphics/Animation.hpp"
#include <stdexcept>

// OOP CONCEPT: CONSTRUCTORS (INITIALIZATION LIST)
// Why: Directly initializes timing parameters and loop states before constructor body.
Animation::Animation(float frameDuration, bool isLooping)
    : m_frames()
    , m_currentFrameIndex(0)
    , m_frameDuration(frameDuration)
    , m_elapsedTime(0.0f)
    , m_isLooping(isLooping) {
}

// OOP CONCEPT: ENCAPSULATION
// Why: Enforces that new frame coordinates are stored safely within the vector.
void Animation::addFrame(const sf::IntRect& frameRect) {
    m_frames.push_back(frameRect);
}

// OOP CONCEPT: ENCAPSULATION
// Why: Frame progression is calculated using delta time so playback is frame-rate
// independent. Handles looping and end-of-animation boundaries cleanly.
void Animation::update(float deltaTime) {
    if (m_frames.empty()) {
        return;
    }

    m_elapsedTime += deltaTime;
    while (m_elapsedTime >= m_frameDuration && m_frameDuration > 0.0f) {
        m_elapsedTime -= m_frameDuration;
        if (m_currentFrameIndex + 1 < m_frames.size()) {
            m_currentFrameIndex++;
        } else if (m_isLooping) {
            m_currentFrameIndex = 0;
        } else {
            // Reached last frame in non-looping animation
            break;
        }
    }
}

void Animation::reset() {
    m_currentFrameIndex = 0;
    m_elapsedTime = 0.0f;
}

// OOP CONCEPT: CONST-CORRECTNESS AND APPROPRIATE USE OF REFERENCES
// Why: Returns a reference to the active frame rect without modifying internal state.
const sf::IntRect& Animation::getCurrentFrameRect() const {
    if (m_frames.empty()) {
        static const sf::IntRect emptyRect(0, 0, 0, 0);
        return emptyRect;
    }
    return m_frames[m_currentFrameIndex];
}

// OOP CONCEPT: CONST-CORRECTNESS
// Why: Query methods do not alter the Animation object and are marked const.
size_t Animation::getFrameCount() const {
    return m_frames.size();
}

size_t Animation::getCurrentFrameIndex() const {
    return m_currentFrameIndex;
}

bool Animation::isFinished() const {
    if (m_isLooping) return false;
    return (m_currentFrameIndex + 1 >= m_frames.size());
}

void Animation::setFrameDuration(float duration) {
    m_frameDuration = duration;
}

float Animation::getFrameDuration() const {
    return m_frameDuration;
}
