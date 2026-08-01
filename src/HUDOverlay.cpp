#include "HUDOverlay.hpp"
#include <sstream>
#include <iomanip>

USING_NS_CC;

HUDOverlay* HUDOverlay::create() {
    auto ret = new HUDOverlay();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool HUDOverlay::init() {
    if (!CCNode::init()) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_statsLabel = CCLabelBMFont::create("Lead: 0ms | Jitter: 0ms | Acc: 100% | Streak: 0", "bigFont.fnt");
    m_statsLabel->setScale(0.35f);
    m_statsLabel->setAnchorPoint(ccp(0.0f, 1.0f));
    m_statsLabel->setPosition(ccp(10.0f, winSize.height - 10.0f));
    m_statsLabel->setOpacity(200);
    this->addChild(m_statsLabel);

    m_feedbackLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_feedbackLabel->setScale(0.5f);
    m_feedbackLabel->setAnchorPoint(ccp(0.5f, 0.5f));
    m_feedbackLabel->setPosition(ccp(winSize.width / 2.0f, winSize.height - 40.0f));
    m_feedbackLabel->setVisible(false);
    this->addChild(m_feedbackLabel);

    return true;
}

void HUDOverlay::updateStats(Gamemode mode, int lastFrameDiff, float fps) {
    auto& cal = CalibrationManager::get();
    double leadMs = cal.getRollingOffsetMs(mode);
    double jitterMs = cal.getJitterMs(mode);
    double accPercent = cal.getAccuracyPercent(mode);
    int streak = cal.getCurrentStreak(mode);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1)
       << "Lead: " << (leadMs >= 0 ? "+" : "") << leadMs << "ms"
       << " | Jitter: " << jitterMs << "ms"
       << " | Acc: " << accPercent << "%"
       << " | Streak: " << streak;

    m_statsLabel->setString(ss.str().c_str());
}

void HUDOverlay::setFeedbackText(int frameDiff) {
    m_feedbackLabel->stopAllActions();
    
    std::string text;
    ccColor3B color;

    if (frameDiff == 0) {
        text = "PERFECT";
        color = ccc3(40, 240, 60);
    } else if (frameDiff < 0) {
        text = std::to_string(-frameDiff) + "F EARLY";
        color = ccc3(240, 180, 40);
    } else {
        text = std::to_string(frameDiff) + "F LATE";
        color = ccc3(240, 60, 40);
    }

    m_feedbackLabel->setString(text.c_str());
    m_feedbackLabel->setColor(color);
    m_feedbackLabel->setOpacity(255);
    m_feedbackLabel->setVisible(true);

    auto fade = CCFadeOut::create(0.8f);
    auto hide = CCCallFunc::create(this, callfunc_selector(HUDOverlay::hideFeedback));
    m_feedbackLabel->runAction(CCSequence::create(fade, hide, nullptr));
}

void HUDOverlay::hideFeedback(float dt) {
    m_feedbackLabel->setVisible(false);
}
