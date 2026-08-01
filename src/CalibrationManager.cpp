#include "CalibrationManager.hpp"
#include <Geode/Geode.hpp>
#include <matjson.hpp>
#include <fstream>
#include <cmath>
#include <numeric>
#include <algorithm>

std::string gamemodeToString(Gamemode mode) {
    switch (mode) {
        case Gamemode::Cube: return "cube";
        case Gamemode::Ship: return "ship";
        case Gamemode::Ball: return "ball";
        case Gamemode::Ufo: return "ufo";
        case Gamemode::Wave: return "wave";
        case Gamemode::Robot: return "robot";
        case Gamemode::Spider: return "spider";
        case Gamemode::Swing: return "swing";
        default: return "cube";
    }
}

Gamemode stringToGamemode(const std::string& str) {
    if (str == "ship") return Gamemode::Ship;
    if (str == "ball") return Gamemode::Ball;
    if (str == "ufo") return Gamemode::Ufo;
    if (str == "wave") return Gamemode::Wave;
    if (str == "robot") return Gamemode::Robot;
    if (str == "spider") return Gamemode::Spider;
    if (str == "swing") return Gamemode::Swing;
    return Gamemode::Cube;
}

CalibrationManager& CalibrationManager::get() {
    static CalibrationManager instance;
    return instance;
}

CalibrationManager::CalibrationManager() {
    load();
}

CalibrationManager::~CalibrationManager() {
    save();
}

std::filesystem::path CalibrationManager::getSavePath() const {
    return geode::Mod::get()->getSaveDir() / "calibration.json";
}

void CalibrationManager::recordClick(Gamemode mode, int frameDiff, float fps) {
    if (fps <= 0.0f) fps = 240.0f;
    double offsetMs = (static_cast<double>(frameDiff) / static_cast<double>(fps)) * 1000.0;
    
    // Clamp offset to [-150 ms, +300 ms]
    offsetMs = std::clamp(offsetMs, -150.0, 300.0);

    bool accurate = std::abs(frameDiff) <= 2;
    if (accurate) {
        m_streaks[mode]++;
    } else {
        m_streaks[mode] = 0;
    }

    ClickRecord rec{ offsetMs, frameDiff, accurate };
    m_history[mode].push_back(rec);
    if (m_history[mode].size() > 40) {
        m_history[mode].pop_front();
    }

    save();
}

void CalibrationManager::resetAttempt() {
    for (int i = 0; i <= static_cast<int>(Gamemode::Swing); ++i) {
        Gamemode gm = static_cast<Gamemode>(i);
        m_frozenActiveOffsets[gm] = getRollingOffsetMs(gm);
    }
}

void CalibrationManager::resetAll() {
    m_history.clear();
    m_frozenActiveOffsets.clear();
    m_streaks.clear();
    save();
}

double CalibrationManager::getActiveOffsetMs(Gamemode mode) const {
    auto it = m_frozenActiveOffsets.find(mode);
    if (it != m_frozenActiveOffsets.end()) {
        return it->second;
    }
    return getRollingOffsetMs(mode);
}

double CalibrationManager::getActiveOffsetFrames(Gamemode mode, float fps) const {
    if (fps <= 0.0f) fps = 240.0f;
    double ms = getActiveOffsetMs(mode);
    return (ms / 1000.0) * static_cast<double>(fps);
}

double CalibrationManager::getRollingOffsetMs(Gamemode mode) const {
    auto it = m_history.find(mode);
    if (it == m_history.end() || it->second.empty()) return 0.0;

    double sum = 0.0;
    for (const auto& rec : it->second) {
        sum += rec.offsetMs;
    }
    return sum / static_cast<double>(it->second.size());
}

double CalibrationManager::getJitterMs(Gamemode mode) const {
    auto it = m_history.find(mode);
    if (it == m_history.end() || it->second.size() < 2) return 0.0;

    double mean = getRollingOffsetMs(mode);
    double accum = 0.0;
    for (const auto& rec : it->second) {
        accum += (rec.offsetMs - mean) * (rec.offsetMs - mean);
    }
    return std::sqrt(accum / static_cast<double>(it->second.size() - 1));
}

double CalibrationManager::getAccuracyPercent(Gamemode mode) const {
    auto it = m_history.find(mode);
    if (it == m_history.end() || it->second.empty()) return 100.0;

    size_t accurateCount = 0;
    for (const auto& rec : it->second) {
        if (rec.accurate) accurateCount++;
    }
    return (static_cast<double>(accurateCount) / static_cast<double>(it->second.size())) * 100.0;
}

int CalibrationManager::getCurrentStreak(Gamemode mode) const {
    auto it = m_streaks.find(mode);
    if (it != m_streaks.end()) {
        return it->second;
    }
    return 0;
}

void CalibrationManager::save() {
    matjson::Value root = matjson::Object();

    for (const auto& [mode, records] : m_history) {
        std::string key = gamemodeToString(mode);
        matjson::Value arr = matjson::Array();
        for (const auto& rec : records) {
            matjson::Value obj = matjson::Object();
            obj["offsetMs"] = rec.offsetMs;
            obj["frameDiff"] = rec.frameDiff;
            obj["accurate"] = rec.accurate;
            arr.push(obj);
        }
        root[key] = arr;
    }

    std::ofstream file(getSavePath());
    if (file.is_open()) {
        file << root.dump(matjson::NO_INDENTATION);
    }
}

void CalibrationManager::load() {
    auto path = getSavePath();
    if (!std::filesystem::exists(path)) return;

    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    auto res = matjson::parse(content);
    if (!res) return;

    m_history.clear();
    auto root = res.unwrap();

    for (int i = 0; i <= static_cast<int>(Gamemode::Swing); ++i) {
        Gamemode gm = static_cast<Gamemode>(i);
        std::string key = gamemodeToString(gm);
        if (root.contains(key) && root[key].isArray()) {
            for (const auto& item : root[key].asArray().unwrap()) {
                ClickRecord rec;
                rec.offsetMs = item["offsetMs"].asDouble().unwrapOr(0.0);
                rec.frameDiff = item["frameDiff"].asInt().unwrapOr(0);
                rec.accurate = item["accurate"].asBool().unwrapOr(false);
                m_history[gm].push_back(rec);
            }
        }
    }
}
