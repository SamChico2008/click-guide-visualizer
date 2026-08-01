#include "assist/AssistState.hpp"
#include "assist/JumpRelease.hpp"
#include "render/GuideController.hpp"
#include "runtime/Runtime.hpp"
#include "settings/Settings.hpp"
#include "AudioEngine.hpp"
#include "CalibrationManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <memory>

using namespace geode::prelude;

struct GuidePlayLayer : geode::Modify<GuidePlayLayer, PlayLayer> {
    struct Fields {
        std::shared_ptr<cgv::GuideController> controller = std::make_shared<cgv::GuideController>();
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        AudioEngine::get().init();
        CalibrationManager::get().resetAttempt();

        cgv::SettingsCache::get().refresh();
        m_fields->controller->attach(this);

        WeakRef<PlayLayer> weakSelf = this;
        auto controller = m_fields->controller;
        cgv::Runtime::get().setRedrawHandler([weakSelf, controller]() mutable {
            if (!weakSelf.lock()) return;
            controller->onSettingsChanged();
        });

        return true;
    }

    void update(float dt) {
        cgv::releaseDueJumps(this);
        PlayLayer::update(dt);
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        CalibrationManager::get().resetAttempt();
        cgv::AssistState::get().buffer().resetConsumed();
        m_fields->controller->onLevelReset();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        m_fields->controller->onPostUpdate(dt);
    }

    void onQuit() {
        cgv::Runtime::get().clearRedrawHandler();
        m_fields->controller->detach();
        PlayLayer::onQuit();
    }
};

