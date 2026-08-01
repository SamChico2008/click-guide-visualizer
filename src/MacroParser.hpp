#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <optional>

struct MacroInput {
    uint64_t frame = 0;
    bool down = false;
    bool player2 = false;
    int button = 1; // 1 = Jump, 2 = Left, 3 = Right
};

class MacroParser {
public:
    static MacroParser& get();
    
    bool parseFile(const std::filesystem::path& path);
    void clear();

    const std::vector<MacroInput>& getInputs() const { return m_inputs; }
    float getTargetFps() const { return m_targetFps; }
    bool hasData() const { return !m_inputs.empty(); }

    std::optional<MacroInput> getNextInput(uint64_t currentFrame, bool player2 = false) const;
    std::optional<MacroInput> getExactInput(uint64_t currentFrame, bool player2 = false) const;

private:
    MacroParser() = default;

    bool parseJsonString(const std::string& content);
    bool parseBinaryGDR(const std::vector<uint8_t>& buffer);
    bool parseBinaryZBot(const std::vector<uint8_t>& buffer);
    bool parseBinaryMHR(const std::vector<uint8_t>& buffer);

    std::vector<MacroInput> m_inputs;
    float m_targetFps = 240.0f;
};
