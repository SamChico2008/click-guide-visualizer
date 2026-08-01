#include "MacroPicker.hpp"
#include <windows.h>
#include <commdlg.h>
#include <filesystem>

namespace cgv {

void pickMacroFile(PickCallback callback) {
    char szFile[MAX_PATH] = {0};

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Macro Files (*.gdr;*.gdr2;*.json;*.mhr)\0*.gdr;*.gdr2;*.json;*.mhr\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        callback(std::filesystem::path(szFile));
    } else {
        callback(std::nullopt);
    }
}

} // namespace cgv
