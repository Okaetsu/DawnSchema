#include <fstream>
#include <filesystem>
#include "Unreal/CoreUObject/UObject/Class.hpp"
#include "Unreal/UFunction.hpp"
#include "Unreal/Hooks.hpp"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "UE4SSProgram.hpp"
#include "Loader/DataAssetLoader.h"
#include "Loader/RawTableLoader.h"
#include "Loader/EnumLoader.h"
#include "Loader/MainLoader.h"
#include "Loader/ResourceLoader.h"
#include "Misc/FileWatchWrapper.h"

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace Schema {
    MainLoader::MainLoader()
    {
        CreateLoaders();
    }

    MainLoader::~MainLoader()
    {
    }

    void MainLoader::Initialize()
	{
        SetupAutoReload();
        InitializeMods(EEngineLifecyclePhase::PostEngineInit);
        LoadMods(EEngineLifecyclePhase::PostEngineInit);
	}

    void MainLoader::AutoReload(const fs::path& FilePath)
    {
        // Skip to the DawnSchema folder and start our iterator from there
        auto It = std::find_if(FilePath.begin(), FilePath.end(),
            [](const auto& Path) { return Path == "DawnSchema"; });

        if (It == FilePath.end() || std::distance(It, FilePath.end()) < 4)
        {
            return;
        }

        // Skip DawnSchema and mods folder
        std::advance(It, 2);
        auto ModName = It->native();

        // Move to folder type, e.g. dataasset
        std::advance(It, 1);
        auto FolderType = It->string();

        std::ifstream FileHandle(FilePath);
        if (FileHandle.peek() == std::ifstream::traits_type::eof()) {
            return;
        }
        FileHandle.close();

        Unreal::Hook::RegisterEngineTickPreCallback([this, FilePath, FolderType, ModName](auto&, Unreal::UEngine*, float, bool)
        {
            try
            {
                for (auto& Loader : Loaders)
                {
                    if (Loader->GetModFolderType() == FolderType)
                    {
                        Loader->AutoReload(ModName, FilePath);
                        PS::Log<LogLevel::Normal>(STR("Auto-reloaded mod {}\n"), ModName);
                        break;
                    }
                }
            }
            catch (const std::exception& e)
            {
                PS::Log<LogLevel::Error>(STR("Failed to auto-reload mod {} - {}\n"), ModName, RC::to_generic_string(e.what()));
            }
        },
        { true, true, ModName, STR("AutoReload") });
    }

    void MainLoader::IterateModsFolder(const std::function<void(const fs::path&, const RC::StringType&)>& Callback)
    {
        auto ModsPath = GetModsPath();
        if (fs::exists(ModsPath))
        {
            for (const auto& Entry : fs::directory_iterator(ModsPath)) {
                if (Entry.is_directory())
                {
                    auto& Path = Entry.path();
                    auto FolderName = Path.stem().native();
                    Callback(Entry.path(), FolderName);
                }
            }
        }
    }

    void MainLoader::CreateLoaders()
    {
        auto ResourceModLoader = std::make_unique<ResourceLoader>();
        RegisterLoader(std::move(ResourceModLoader));

        auto DataAssetModLoader = std::make_unique<DataAssetLoader>();
        RegisterLoader(std::move(DataAssetModLoader));

        auto RawTableModLoader = std::make_unique<RawTableLoader>();
        RegisterLoader(std::move(RawTableModLoader));
    }

    void MainLoader::SetupAutoReload()
    {
        auto Config = PS::PSConfig::Get();
        if (!Config->IsAutoReloadEnabled()) return;

        PS::Log<LogLevel::Normal>(STR("Auto-reload is enabled.\n"));

        auto ModsPath = GetModsPath();

        FileWatcher = std::make_unique<PS::FileWatchWrapper>(ModsPath, [this](efsw::WatchID WatchId, const std::string& Dir,
            const std::string& Filename, efsw::Action Action, std::string OldFilename) {
                if (Action == efsw::Actions::Add || Action == efsw::Actions::Modified)
                {
                    auto Path = fs::path(Dir) / Filename;
                    AutoReload(Path);
                }
            }
        );
        FileWatcher->Watch();
    }

    void MainLoader::RegisterLoader(std::unique_ptr<ModLoaderBase> NewLoader)
    {
        NewLoader->Setup();
        Loaders.push_back(std::move(NewLoader));
    }

    void MainLoader::InitializeMods(EEngineLifecyclePhase EngineLifecyclePhase)
    {
        for (auto& Loader : Loaders)
        {
            Loader->Initialize(EngineLifecyclePhase);
        }
    }

    void MainLoader::LoadMods(EEngineLifecyclePhase EngineLifecyclePhase)
    {
        IterateModsFolder([&](const fs::path& ModPath, const fs::path::string_type& ModName)
        {
            try
            {
                PS::Log<RC::LogLevel::Normal>(STR("Loading mod: {}\n"), ModName);

                for (auto& Loader : Loaders)
                {
                    Loader->Load(ModPath, ModName, EngineLifecyclePhase);
                }
            }
            catch (const std::exception& e)
            {
                PS::Log<LogLevel::Error>(STR("Failed to load mod {} - {}\n"), ModName, RC::to_generic_string(e.what()));
            }
        });
    }

    fs::path MainLoader::GetModsPath()
    {
        static auto ModsPath = fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "DawnSchema" / "mods";
        return ModsPath;
    }
}
