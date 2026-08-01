#include "MacroParser.hpp"
#include <Geode/Geode.hpp>
#include <matjson.hpp>
#include <fstream>
#include <algorithm>
#include <cstring>

MacroParser& MacroParser::get() {
    static MacroParser instance;
    return instance;
}

void MacroParser::clear() {
    m_inputs.clear();
    m_targetFps = 240.0f;
}

bool MacroParser::parseFile(const std::filesystem::path& path) {
    clear();
    if (!std::filesystem::exists(path)) return false;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) return false;

    std::string strContent(buffer.begin(), buffer.end());
    if (parseJsonString(strContent)) return true;
    if (parseBinaryGDR(buffer)) return true;
    if (parseBinaryZBot(buffer)) return true;
    if (parseBinaryMHR(buffer)) return true;

    return false;
}

bool MacroParser::parseJsonString(const std::string& content) {
    auto res = matjson::parse(content);
    if (!res) return false;
    auto root = res.unwrap();

    if (root.contains("fps")) {
        if (root["fps"].isNumber()) {
            m_targetFps = static_cast<float>(root["fps"].asDouble().unwrapOr(240.0));
        }
    } else if (root.contains("framerate")) {
        if (root["framerate"].isNumber()) {
            m_targetFps = static_cast<float>(root["framerate"].asDouble().unwrapOr(240.0));
        }
    }

    auto parseInputObj = [&](const matjson::Value& obj) {
        MacroInput in;
        if (obj.contains("frame")) {
            in.frame = static_cast<uint64_t>(obj["frame"].asUInt().unwrapOr(0));
        } else if (obj.contains("f")) {
            in.frame = static_cast<uint64_t>(obj["f"].asUInt().unwrapOr(0));
        }

        if (obj.contains("down")) {
            in.down = obj["down"].asBool().unwrapOr(false);
        } else if (obj.contains("press")) {
            in.down = obj["press"].asBool().unwrapOr(false);
        } else if (obj.contains("d")) {
            in.down = obj["d"].asBool().unwrapOr(false);
        } else if (obj.contains("holding")) {
            in.down = obj["holding"].asBool().unwrapOr(false);
        }

        if (obj.contains("player2")) {
            in.player2 = obj["player2"].asBool().unwrapOr(false);
        } else if (obj.contains("p2")) {
            in.player2 = obj["p2"].asBool().unwrapOr(false);
        } else if (obj.contains("p")) {
            in.player2 = (obj["p"].asInt().unwrapOr(1) == 2);
        }

        if (obj.contains("button")) {
            in.button = obj["button"].asInt().unwrapOr(1);
        } else if (obj.contains("btn")) {
            in.button = obj["btn"].asInt().unwrapOr(1);
        } else if (obj.contains("b")) {
            in.button = obj["b"].asInt().unwrapOr(1);
        }

        m_inputs.push_back(in);
    };

    if (root.contains("events") && root["events"].isArray()) {
        for (const auto& item : root["events"].asArray().unwrap()) {
            parseInputObj(item);
        }
    } else if (root.contains("inputs") && root["inputs"].isArray()) {
        for (const auto& item : root["inputs"].asArray().unwrap()) {
            parseInputObj(item);
        }
    } else if (root.contains("macro") && root["macro"].isArray()) {
        for (const auto& item : root["macro"].asArray().unwrap()) {
            parseInputObj(item);
        }
    } else if (root.isArray()) {
        for (const auto& item : root.asArray().unwrap()) {
            parseInputObj(item);
        }
    }

    std::sort(m_inputs.begin(), m_inputs.end(), [](const MacroInput& a, const MacroInput& b) {
        return a.frame < b.frame;
    });

    return !m_inputs.empty();
}

bool MacroParser::parseBinaryGDR(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 12) return false;
    if (buffer[0] != 'G' || buffer[1] != 'D' || buffer[2] != 'R') return false;

    size_t offset = 4;
    float fps = 240.0f;
    std::memcpy(&fps, buffer.data() + offset, sizeof(float));
    if (fps > 0.0f) m_targetFps = fps;
    offset += 4;

    uint32_t count = 0;
    std::memcpy(&count, buffer.data() + offset, sizeof(uint32_t));
    offset += 4;

    for (uint32_t i = 0; i < count && offset + 6 <= buffer.size(); ++i) {
        MacroInput in;
        uint32_t frame = 0;
        std::memcpy(&frame, buffer.data() + offset, sizeof(uint32_t));
        offset += 4;
        in.frame = frame;
        in.down = buffer[offset++] != 0;
        in.player2 = buffer[offset++] != 0;
        m_inputs.push_back(in);
    }
    return !m_inputs.empty();
}

bool MacroParser::parseBinaryZBot(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 8) return false;
    float fps = 240.0f;
    std::memcpy(&fps, buffer.data(), sizeof(float));
    if (fps < 1.0f || fps > 1000.0f) return false;

    m_targetFps = fps;
    size_t offset = 4;
    uint32_t count = 0;
    std::memcpy(&count, buffer.data() + offset, sizeof(uint32_t));
    offset += 4;

    for (uint32_t i = 0; i < count && offset + 6 <= buffer.size(); ++i) {
        MacroInput in;
        uint32_t frame = 0;
        std::memcpy(&frame, buffer.data() + offset, sizeof(uint32_t));
        offset += 4;
        in.frame = frame;
        in.down = buffer[offset++] != 0;
        in.player2 = buffer[offset++] != 0;
        m_inputs.push_back(in);
    }
    return !m_inputs.empty();
}

bool MacroParser::parseBinaryMHR(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 8) return false;
    if (buffer[0] != 'M' || buffer[1] != 'H' || buffer[2] != 'R') return false;

    float fps = 240.0f;
    std::memcpy(&fps, buffer.data() + 4, sizeof(float));
    if (fps > 0.0f) m_targetFps = fps;

    size_t offset = 8;
    while (offset + 6 <= buffer.size()) {
        MacroInput in;
        uint32_t frame = 0;
        std::memcpy(&frame, buffer.data() + offset, sizeof(uint32_t));
        offset += 4;
        in.frame = frame;
        in.down = buffer[offset++] != 0;
        in.player2 = buffer[offset++] != 0;
        m_inputs.push_back(in);
    }
    return !m_inputs.empty();
}

std::optional<MacroInput> MacroParser::getNextInput(uint64_t currentFrame, bool player2) const {
    if (m_inputs.empty()) return std::nullopt;

    auto it = std::lower_bound(m_inputs.begin(), m_inputs.end(), currentFrame,
        [](const MacroInput& in, uint64_t val) {
            return in.frame < val;
        });

    while (it != m_inputs.end()) {
        if (it->player2 == player2 && it->down) {
            return *it;
        }
        ++it;
    }
    return std::nullopt;
}

std::optional<MacroInput> MacroParser::getExactInput(uint64_t currentFrame, bool player2) const {
    if (m_inputs.empty()) return std::nullopt;

    auto it = std::lower_bound(m_inputs.begin(), m_inputs.end(), currentFrame,
        [](const MacroInput& in, uint64_t val) {
            return in.frame < val;
        });

    if (it != m_inputs.end() && it->frame == currentFrame && it->player2 == player2) {
        return *it;
    }
    return std::nullopt;
}

