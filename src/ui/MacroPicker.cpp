#include "MacroPicker.hpp"
#include <Geode/Geode.hpp>
#include <windows.h>
#include <commdlg.h>
#include <thread>
#include <atomic>

namespace cgv {

static std::atomic<bool> s_picking{false};

void pickMacroFile(PickCallback callback) {
    if (s_picking.exchange(true)) return;

    std::thread([callback = std::move(callback)]() {
        char szFile[MAX_PATH] = {0};

        OPENFILENAMEA ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "Macro Files (*.gdr;*.gdr2;*.json;*.mhr)\0*.gdr;*.gdr2;*.json;*.mhr\0All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        BOOL res = GetOpenFileNameA(&ofn);
        std::string chosenPath = (res == TRUE) ? std::string(szFile) : "";

        geode::Loader::get()->queueInMainThread([callback, res, chosenPath]() {
            s_picking.store(false);
            if (res == TRUE && !chosenPath.empty()) {
                callback(std::filesystem::path(chosenPath));
            } else {
                callback(std::nullopt);
            }
        });
    }).detach();
}

} // namespace cgv
