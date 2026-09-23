#pragma once

#include "Unreal/NameTypes.hpp"
#include "Loader/ModLoaderBase.h"
#include "nlohmann/json.hpp"

namespace Schema {
	class DataAssetLoader : public ModLoaderBase {
	public:
        DataAssetLoader();

        ~DataAssetLoader();
    protected:
        virtual void OnLoad(const std::filesystem::path& LoaderPath, const RC::StringType& ModName, const EEngineLifecyclePhase& EngineLifecyclePhase) override final;
        virtual void OnAutoReload(const RC::StringType& ModName, const std::filesystem::path& ModFilePath) override final;
        virtual bool CanInitialize(const EEngineLifecyclePhase& EngineLifecyclePhase) override final;
        virtual bool OnInitialize() override final;
        void ModifyAsset(const std::string& Key, const nlohmann::json& Data);
        void ModifyAsset(RC::Unreal::UObject* Asset, const nlohmann::json& Data);
        void CreateAsset(const RC::Unreal::FString& AssetPath, const nlohmann::json& Data);
        bool DoesAssetExist(const RC::Unreal::FString& AssetPath);
	};
}