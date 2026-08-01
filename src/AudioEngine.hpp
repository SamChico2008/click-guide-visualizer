#pragma once
#include <string>
#include <vector>
#include <fmod.hpp>

enum class SoundPack {
    Click,
    Beep,
    Wood,
    Snap
};

class AudioEngine {
public:
    static AudioEngine& get();

    void init();
    void playCue(SoundPack pack, bool isRelease = false);

private:
    AudioEngine() = default;
    ~AudioEngine();

    void createSynthSounds();
    FMOD::Sound* generateToneSound(float frequency, float durationSec, float attackSec, float decaySec, int packType);

    FMOD::System* m_fmodSystem = nullptr;
    FMOD::Sound* m_sounds[4][2] = { {nullptr, nullptr} }; // [Pack][Press/Release]
    bool m_initialized = false;
};
