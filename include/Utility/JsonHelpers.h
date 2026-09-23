#pragma once

#include <string>
#include <functional>
#include "nlohmann/json_fwd.hpp"
#include "Unreal/Core/HAL/Platform.hpp"

namespace RC::Unreal {
    struct FRotator;
    struct FVector;
    class FName;
    class FString;
}

namespace PS::JsonHelpers {
    void ParseJsonFileInPath(const std::filesystem::path& Path, const std::function<void(const nlohmann::json&)>& Callback);
    void ParseJsonFilesInPath(const std::filesystem::path& Path, const std::function<void(const nlohmann::json&)>& Callback);
}