#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "Helpers/String.hpp"
#include <fstream>
#include "glaze/glaze.hpp"
#include "UE4SSProgram.hpp"

namespace fs = std::filesystem;

namespace PS {
    std::unique_ptr<PSConfig> GConfig;

    PSConfig* PSConfig::Get()
    {
        if (!GConfig)
        {
            GConfig = std::make_unique<PSConfig>();
        }
        
        return GConfig.get();
    }

    std::string PSConfig::GetLanguageOverride()
    {
        auto Config = Get();
        return Config ? Config->ConfigSettings.languageOverride : "";
    }

    bool PSConfig::IsAutoReloadEnabled()
    {
        auto Config = Get();
        return Config ? Config->ConfigSettings.enableAutoReload : false;
    }

    bool PSConfig::IsDebugLoggingEnabled()
    {
        auto Config = Get();
        return Config ? Config->ConfigSettings.enableDebugLogging : false;
    }

    void PSConfig::Load()
    {
        auto FolderPath = GetConfigPath();
        if (!fs::exists(FolderPath))
        {
            fs::create_directory(FolderPath);
        }

        auto ConfigFile = FolderPath / "config.json";
        if (!fs::exists(ConfigFile))
        {
            this->Save();
            PS::Log<RC::LogLevel::Warning>(STR("Config file not found, a new one was generated. Default values will be used.\n"));
            return;
        }

        auto ErrorCode = glz::read_file_json < glz::opts{ .error_on_missing_keys = true } > (ConfigSettings, ConfigFile.string(), std::string{});
        if (ErrorCode) {
            std::string ErrorMessage = glz::format_error(ErrorCode, std::string{});
            PS::Log<RC::LogLevel::Error>(STR("Error parsing config: {}\n"), RC::to_generic_string(ErrorMessage));
            this->Save();
            PS::Log<RC::LogLevel::Normal>(STR("Config has been repaired.\n"));
        }

        PS::Log<RC::LogLevel::Normal>(STR("Config loaded.\n"));
    }

    fs::path PSConfig::GetConfigPath()
    {
        static auto Path = fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "DawnSchema" / "config";
        return Path;
    }

    void PSConfig::Save()
    {
        auto ConfigFile = GetConfigPath() / "config.json";
        auto ErrorCode = glz::write_file_json<glz::opts{ .prettify = true }>(ConfigSettings, ConfigFile.string(), std::string{});
        if (ErrorCode)
        {
            std::string ErrorMessage = glz::format_error(ErrorCode, std::string{});
            PS::Log<RC::LogLevel::Error>(STR("Failed to write to config: {}\n"), RC::to_generic_string(ErrorMessage));
        }
    }
}
