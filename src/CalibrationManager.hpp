#pragma once
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <filesystem>

enum class Gamemode {
    Cube,
    Ship,
    Ball,
    Ufo,
    Wave,
    Robot,
    Spider,
    Swing,
    Unknown
};

std::string gamemodeToString(Gamemode mode);
Gamemode stringToGamemode(const std::string& str);

struct ClickRecord {
    double offsetMs = 0.0;
    int frameDiff = 0;
    bool accurate = false;
};

class CalibrationManager {
public:
    static CalibrationManager& get();

    void recordClick(Gamemode mode, int frameDiff, float fps);
    void resetAttempt();
    void resetAll();

    double getActiveOffsetMs(Gamemode mode) const;
    double getActiveOffsetFrames(Gamemode mode, float fps) const;
    
    double getRollingOffsetMs(Gamemode mode) const;
    double getJitterMs(Gamemode mode) const;
    double getAccuracyPercent(Gamemode mode) const;
    int getCurrentStreak(Gamemode mode) const;

    void save();
    void load();

private:
    CalibrationManager();
    ~CalibrationManager();

    std::map<Gamemode, std::deque<ClickRecord>> m_history;
    std::map<Gamemode, double> m_frozenActiveOffsets;
    std::map<Gamemode, int> m_streaks;

    std::filesystem::path getSavePath() const;
};
