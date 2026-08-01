#pragma once
#include <cocos2d.h>
#include "CalibrationManager.hpp"

class HUDOverlay : public cocos2d::CCNode {
public:
    static HUDOverlay* create();
    bool init() override;

    void updateStats(Gamemode mode, int lastFrameDiff, float fps);
    void setFeedbackText(int frameDiff);

private:
    cocos2d::CCLabelBMFont* m_statsLabel = nullptr;
    cocos2d::CCLabelBMFont* m_feedbackLabel = nullptr;
    float m_feedbackTimer = 0.0f;
    void hideFeedback(float dt);
};
