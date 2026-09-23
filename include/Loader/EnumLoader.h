#pragma once

#include "Unreal/NameTypes.hpp"
#include "Loader/ModLoaderBase.h"
#include "nlohmann/json.hpp"

namespace RC::Unreal {
    class UEnum;
}

namespace Schema {
    class EnumLoader : public ModLoaderBase {
    public:
        EnumLoader();
        ~EnumLoader();
    protected:
        virtual void OnLoad(const std::filesystem::path& LoaderPath, const RC::StringType& ModName, const EEngineLifecyclePhase& EngineLifecyclePhase) override final;
        
        virtual bool CanInitialize(const EEngineLifecyclePhase& EngineLifecyclePhase) override final;
        virtual bool OnInitialize() override final;
    private:
        std::unordered_map<RC::StringType, RC::Unreal::UEnum*> EnumNameToObjectMap;

        void LoadEnums(const nlohmann::json& Data);

        RC::Unreal::UEnum* GetEnumByName(const RC::StringType& Name);
    };
}