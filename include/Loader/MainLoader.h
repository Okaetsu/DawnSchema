#pragma once

#include <vector>
#include <functional>
#include "ModLoaderBase.h"

namespace PS {
    class FileWatchWrapper;
}

namespace Schema {
	class MainLoader {
	public:
        MainLoader();

        ~MainLoader();

		void Initialize();
	private:
        std::vector<std::unique_ptr<ModLoaderBase>> Loaders;

        std::unique_ptr<PS::FileWatchWrapper> FileWatcher;

        void AutoReload(const std::filesystem::path& FilePath);

        void IterateModsFolder(const std::function<void(const std::filesystem::path&, const RC::StringType&)>& Callback);

        void CreateLoaders();

        void SetupAutoReload();

        void InitCore();

        void RegisterLoader(std::unique_ptr<ModLoaderBase> NewLoader);

        void InitializeMods(EEngineLifecyclePhase EngineLifecyclePhase);
        void LoadMods(EEngineLifecyclePhase EngineLifecyclePhase);
    private:
        static std::filesystem::path GetModsPath();
	};
}