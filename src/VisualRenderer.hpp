#pragma once
#include <cocos2d.h>
#include "MacroParser.hpp"

enum class VisualStyle {
    Ring,
    Classic,
    Converge,
    Pulse
};

VisualStyle stringToVisualStyle(const std::string& str);

class VisualRenderer : public cocos2d::CCNode {
public:
    static VisualRenderer* create();
    bool init() override;

    void updateCueState(
        const cocos2d::CCPoint& playerPos,
        const std::optional<MacroInput>& nextInput,
        uint64_t currentFrame,
        float targetFps,
        double calibratedOffsetFrames,
        VisualStyle style
    );

private:
    void renderRing(const cocos2d::CCPoint& pos, float deltaFrames, bool isHit);
    void renderClassic(const cocos2d::CCPoint& pos, float deltaFrames, bool isHit);
    void renderConverge(const cocos2d::CCPoint& pos, float deltaFrames, bool isHit);
    void renderPulse(const cocos2d::CCPoint& pos, float deltaFrames, bool isHit);

    cocos2d::CCDrawNode* m_drawNode = nullptr;
};
