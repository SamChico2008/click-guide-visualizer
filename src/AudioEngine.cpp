#include "AudioEngine.hpp"
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cmath>
#include <cstring>

AudioEngine& AudioEngine::get() {
    static AudioEngine instance;
    return instance;
}

AudioEngine::~AudioEngine() {
    for (int p = 0; p < 4; ++p) {
        for (int r = 0; r < 2; ++r) {
            if (m_sounds[p][r]) {
                m_sounds[p][r]->release();
                m_sounds[p][r] = nullptr;
            }
        }
    }
}

void AudioEngine::init() {
    if (m_initialized) return;

    auto engine = FMODAudioEngine::sharedEngine();
    if (engine) {
        m_fmodSystem = engine->m_system;
        if (m_fmodSystem) {
            createSynthSounds();
            m_initialized = true;
        }
    }
}

FMOD::Sound* AudioEngine::generateToneSound(float frequency, float durationSec, float attackSec, float decaySec, int packType) {
    if (!m_fmodSystem) return nullptr;

    const int sampleRate = 44100;
    int totalSamples = static_cast<int>(sampleRate * durationSec);
    std::vector<int16_t> pcmData(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        float envelope = 1.0f;

        if (t < attackSec) {
            envelope = t / attackSec;
        } else if (t > durationSec - decaySec) {
            envelope = (durationSec - t) / decaySec;
        }

        float sampleVal = 0.0f;

        switch (packType) {
            case 0: // Click: Sharp impulse + noise burst
                sampleVal = std::sin(2.0f * M_PI * frequency * t) * std::exp(-t * 120.0f);
                sampleVal += ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * std::exp(-t * 200.0f) * 0.4f;
                break;
            case 1: // Beep: Pure sine tone
                sampleVal = std::sin(2.0f * M_PI * frequency * t) * envelope;
                break;
            case 2: // Wood: Percussive low pop
                sampleVal = std::sin(2.0f * M_PI * frequency * t * std::exp(-t * 30.0f)) * std::exp(-t * 50.0f);
                break;
            case 3: // Snap: Transient white noise snap
                sampleVal = ((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * std::exp(-t * 90.0f);
                break;
        }

        sampleVal = std::clamp(sampleVal, -1.0f, 1.0f);
        pcmData[i] = static_cast<int16_t>(sampleVal * 32767.0f);
    }

    FMOD_CREATESAMPLEEXINFO exinfo;
    std::memset(&exinfo, 0, sizeof(FMOD_CREATESAMPLEEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESAMPLEEXINFO);
    exinfo.length = static_cast<unsigned int>(pcmData.size() * sizeof(int16_t));
    exinfo.defaultfrequency = sampleRate;
    exinfo.numchannels = 1;
    exinfo.format = FMOD_SOUND_FORMAT_PCM16;

    FMOD::Sound* sound = nullptr;
    FMOD_RESULT res = m_fmodSystem->createSound(
        reinterpret_cast<const char*>(pcmData.data()),
        FMOD_OPENRAW | FMOD_CREATESAMPLE,
        &exinfo,
        &sound
    );

    return (res == FMOD_OK) ? sound : nullptr;
}

void AudioEngine::createSynthSounds() {
    // Pack 0: Click (Press: 1200Hz, Release: 800Hz)
    m_sounds[0][0] = generateToneSound(1200.0f, 0.05f, 0.002f, 0.02f, 0);
    m_sounds[0][1] = generateToneSound(800.0f, 0.05f, 0.002f, 0.02f, 0);

    // Pack 1: Beep (Press: 1000Hz, Release: 650Hz)
    m_sounds[1][0] = generateToneSound(1000.0f, 0.06f, 0.005f, 0.02f, 1);
    m_sounds[1][1] = generateToneSound(650.0f, 0.06f, 0.005f, 0.02f, 1);

    // Pack 2: Wood (Press: 450Hz, Release: 300Hz)
    m_sounds[2][0] = generateToneSound(450.0f, 0.08f, 0.003f, 0.03f, 2);
    m_sounds[2][1] = generateToneSound(300.0f, 0.08f, 0.003f, 0.03f, 2);

    // Pack 3: Snap (Press: 2200Hz burst, Release: 1500Hz burst)
    m_sounds[3][0] = generateToneSound(2200.0f, 0.04f, 0.001f, 0.015f, 3);
    m_sounds[3][1] = generateToneSound(1500.0f, 0.04f, 0.001f, 0.015f, 3);
}

void AudioEngine::playCue(SoundPack pack, bool isRelease) {
    if (!m_initialized) init();
    if (!m_fmodSystem) return;

    int packIdx = static_cast<int>(pack);
    int relIdx = isRelease ? 1 : 0;

    FMOD::Sound* sound = m_sounds[packIdx][relIdx];
    if (sound) {
        FMOD::Channel* channel = nullptr;
        m_fmodSystem->playSound(sound, nullptr, false, &channel);
        if (channel) {
            channel->setVolume(0.85f);
        }
    }
}
