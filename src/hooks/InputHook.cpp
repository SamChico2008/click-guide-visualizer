#include "assist/AssistState.hpp"
#include "parse/Framerate.hpp"
#include "runtime/Runtime.hpp"
#include "settings/Settings.hpp"
#include "store/MacroStore.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <cmath>

using namespace geode::prelude;

namespace {

constexpr int kJumpButton = 1;

int currentMacroFrameOf(PlayLayer* playLayer) {
    double tickRate = cgv::MacroStore::get().framerate();
    if (tickRate <= 0.0) tickRate = cgv::kDefaultTickRate;
    return static_cast<int>(std::lround(playLayer->m_attemptTime * tickRate));
}

bool shouldHoldForLater(PlayLayer* playLayer, bool isPlayer1) {
    if (!cgv::cheatsAreActive()) return false;

    auto& state = cgv::AssistState::get();
    if (!state.active() || state.releasing()) return false;
    if (!state.buffer().hasTargets()) return false;

    bool held = state.buffer().tryBuffer(playLayer->m_attemptTime,
                                         cgv::settings().assistBufferSeconds,
                                         !isPlayer1);
    if (held) state.noteBuffered();
    return held;
}

} // namespace

struct GuideInputLayer : geode::Modify<GuideInputLayer, GJBaseGameLayer> {
    void handleButton(bool down, int button, bool isPlayer1) {
        auto* playLayer = typeinfo_cast<PlayLayer*>(static_cast<GJBaseGameLayer*>(this));

        if (down && button == kJumpButton && playLayer && shouldHoldForLater(playLayer, isPlayer1)) {
            return;
        }

        GJBaseGameLayer::handleButton(down, button, isPlayer1);

        if (button == kJumpButton && playLayer && cgv::settings().showGuide) {
            std::string soundPackSetting = Mod::get()->getSettingValue<std::string>("sound-pack");
            SoundPack pack = SoundPack::Click;
            if (soundPackSetting == "Beep") pack = SoundPack::Beep;
            else if (soundPackSetting == "Wood") pack = SoundPack::Wood;
            else if (soundPackSetting == "Snap") pack = SoundPack::Snap;

            AudioEngine::get().playCue(pack, !down);
        }

        if (!down || button != kJumpButton || !playLayer) return;
        if (!cgv::settings().showGuide) return;

        cgv::Runtime::get().queuePress(currentMacroFrameOf(playLayer), !isPlayer1);
    }
};

