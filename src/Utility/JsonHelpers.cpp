#include <Filesystem>
#include "Utility/JsonHelpers.h"
#include "Unreal/Core/HAL/Platform.hpp"
#include "Unreal/NameTypes.hpp"
#include "Unreal/UnrealCoreStructs.hpp"
#include "Unreal/Rotator.hpp"
#include "nlohmann/json.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace PS::JsonHelpers {
    void ParseJsonFileInPath(const fs::path& Path, const std::function<void(const nlohmann::json&)>& Callback)
    {
        if (!fs::exists(Path))
        {
            return;
        }

        if (Path.extension() != ".json" && Path.extension() != ".jsonc")
        {
            return;
        }

        auto IgnoreComments = Path.extension() == ".jsonc";
        std::ifstream FileHandle(Path);

        nlohmann::json Data = nlohmann::json::parse(FileHandle, nullptr, true, IgnoreComments);
        Callback(Data);
    }

    void ParseJsonFilesInPath(const fs::path& Path, const std::function<void(const nlohmann::json&)>& Callback)
    {
        if (!fs::is_directory(Path))
        {
            return;
        }

        for (const auto& File : fs::directory_iterator(Path))
        {
            try
            {
                auto FilePath = File.path();
                if (FilePath.has_extension())
                {
                    ParseJsonFileInPath(FilePath, Callback);
                }
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error(std::format("Failed parsing mod file {} - {}.\n", File.path().string(), e.what()));
            }
        }
    }
}