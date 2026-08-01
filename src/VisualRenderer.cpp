#include "VisualRenderer.hpp"
#include <cmath>

USING_NS_CC;

VisualStyle stringToVisualStyle(const std::string& str) {
    if (str == "Classic") return VisualStyle::Classic;
    if (str == "Converge") return VisualStyle::Converge;
    if (str == "Pulse") return VisualStyle::Pulse;
    return VisualStyle::Ring;
}

VisualRenderer* VisualRenderer::create() {
    auto ret = new VisualRenderer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool VisualRenderer::init() {
    if (!CCNode::init()) return false;

    m_drawNode = CCDrawNode::create();
    this->addChild(m_drawNode);
    return true;
}

void VisualRenderer::updateCueState(
    const CCPoint& playerPos,
    const std::optional<MacroInput>& nextInput,
    uint64_t currentFrame,
    float targetFps,
    double calibratedOffsetFrames,
    VisualStyle style
) {
    m_drawNode->clear();
    if (!nextInput.has_value()) return;

    double targetFrame = static_cast<double>(nextInput->frame) + calibratedOffsetFrames;
    double deltaFrames = targetFrame - static_cast<double>(currentFrame);

    if (deltaFrames < -5.0 || deltaFrames > 60.0) return;

    bool isHit = std::abs(deltaFrames) <= 1.5;

    switch (style) {
        case VisualStyle::Ring:
            renderRing(playerPos, static_cast<float>(deltaFrames), isHit);
            break;
        case VisualStyle::Classic:
            renderClassic(playerPos, static_cast<float>(deltaFrames), isHit);
            break;
        case VisualStyle::Converge:
            renderConverge(playerPos, static_cast<float>(deltaFrames), isHit);
            break;
        case VisualStyle::Pulse:
            renderPulse(playerPos, static_cast<float>(deltaFrames), isHit);
            break;
    }
}

void VisualRenderer::renderRing(const CCPoint& pos, float deltaFrames, bool isHit) {
    float progress = std::max(0.0f, deltaFrames / 60.0f);
    float radius = 15.0f + 65.0f * (progress * progress);
    ccColor4F color = isHit ? ccc4f(0.1f, 0.95f, 0.2f, 0.9f) : ccc4f(0.0f, 0.8f, 1.0f, 0.75f);

    m_drawNode->drawCircle(pos, radius, color, 1.5f, 36);
    if (isHit) {
        m_drawNode->drawDot(pos, 4.0f, color);
    }
}

void VisualRenderer::renderClassic(const CCPoint& pos, float deltaFrames, bool isHit) {
    ccColor4F windowColor = ccc4f(1.0f, 1.0f, 1.0f, 0.15f);
    m_drawNode->drawRect(ccp(pos.x - 20.0f, pos.y - 25.0f), ccp(pos.x + 20.0f, pos.y + 25.0f), windowColor, 1.0f, ccc4f(1.0f, 1.0f, 1.0f, 0.4f));

    float offsetX = deltaFrames * 3.5f;
    ccColor4F lineColor = isHit ? ccc4f(0.1f, 0.95f, 0.2f, 1.0f) : ccc4f(0.0f, 0.8f, 1.0f, 0.85f);
    m_drawNode->drawSegment(
        ccp(pos.x - offsetX, pos.y - 30.0f),
        ccp(pos.x - offsetX, pos.y + 30.0f),
        1.5f,
        lineColor
    );
}

void VisualRenderer::renderConverge(const CCPoint& pos, float deltaFrames, bool isHit) {
    float offset = deltaFrames * 4.0f;
    ccColor4F color = isHit ? ccc4f(0.1f, 0.95f, 0.2f, 1.0f) : ccc4f(0.0f, 0.8f, 1.0f, 0.85f);

    m_drawNode->drawSegment(
        ccp(pos.x - offset - 10.0f, pos.y - 20.0f),
        ccp(pos.x - offset - 10.0f, pos.y + 20.0f),
        1.5f,
        color
    );

    m_drawNode->drawSegment(
        ccp(pos.x + offset + 10.0f, pos.y - 20.0f),
        ccp(pos.x + offset + 10.0f, pos.y + 20.0f),
        1.5f,
        color
    );
}

void VisualRenderer::renderPulse(const CCPoint& pos, float deltaFrames, bool isHit) {
    float alpha = isHit ? 1.0f : (1.0f - std::min(1.0f, deltaFrames / 60.0f)) * 0.7f;
    float size = isHit ? 22.0f : 12.0f;
    ccColor4F color = isHit ? ccc4f(0.1f, 0.95f, 0.2f, alpha) : ccc4f(0.0f, 0.8f, 1.0f, alpha);

    m_drawNode->drawSegment(ccp(pos.x - size, pos.y), ccp(pos.x + size, pos.y), 1.2f, color);
    m_drawNode->drawSegment(ccp(pos.x, pos.y - size), ccp(pos.x, pos.y + size), 1.2f, color);
    if (isHit) {
        m_drawNode->drawCircle(pos, 16.0f, color, 1.5f, 24);
    }
}

