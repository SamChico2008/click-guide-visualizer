#include "AudioEngine.hpp"
#include <Geode/Geode.hpp>
#include <cocos2d.h>

AudioEngine& AudioEngine::get() {
    static AudioEngine instance;
    return instance;
}

AudioEngine::~AudioEngine() {
}

void AudioEngine::init() {
    if (m_initialized) return;
    m_initialized = true;
}

void AudioEngine::playCue(SoundPack pack, bool isRelease) {
    if (!m_initialized) init();

    auto engine = FMODAudioEngine::sharedEngine();
    if (!engine) return;

    // Use built-in GD sound effects for clean cross-platform execution
    const char* soundFile = "click.ogg";
    switch (pack) {
        case SoundPack::Click:
            soundFile = isRelease ? "clickRelease.ogg" : "click.ogg";
            break;
        case SoundPack::Beep:
            soundFile = isRelease ? "gold02.ogg" : "gold01.ogg";
            break;
        case SoundPack::Wood:
            soundFile = isRelease ? "quitSound01.ogg" : "playSound_01.ogg";
            break;
        case SoundPack::Snap:
            soundFile = isRelease ? "buyItem01.ogg" : "dragNode.ogg";
            break;
    }

    engine->playEffect(soundFile, 1.0f, 0.0f, 0.7f);
}
