#include "MacroPicker.hpp"

#include "core/Compat.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/file.hpp>
#include <utility>

#if CGV_GEODE_V5
#include <Geode/utils/async.hpp>
#endif

using namespace geode::prelude;

namespace cgv {

namespace {

file::FilePickOptions macroPickOptions() {
    file::FilePickOptions options;
    options.filters.push_back(file::FilePickOptions::Filter{
        "Macro files",
        {"*.gdr", "*.gdr2", "*.json", "*.mhr"},
    });
    return options;
}

} // namespace

void pickMacroFile(PickCallback callback) {
    auto options = macroPickOptions();

#if CGV_GEODE_V5
    async::spawn(file::pick(file::PickMode::OpenFile, options),
                 [callback = std::move(callback)](file::PickResult result) {
                     if (!result) {
                         callback(std::nullopt);
                         return;
                     }
                     auto picked = std::move(result).unwrap();
                     if (!picked.has_value()) {
                         callback(std::nullopt);
                         return;
                     }
                     callback(std::move(picked.value()));
                 });
#else
    file::pick(file::PickMode::OpenFile, options).listen([callback = std::move(callback)](Result<std::filesystem::path>* res) {
        if (!res || !res->isOk()) {
            callback(std::nullopt);
            return;
        }
        callback(res->unwrap());
    });
#endif
}

} // namespace cgv
