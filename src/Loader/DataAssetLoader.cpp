#include "Unreal/CoreUObject/UObject/UnrealType.hpp"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UPackage.hpp"
#include "Unreal/NameTypes.hpp"
#include "SDK/Helper/PropertyHelper.h"
#include "Utility/Logging.h"
#include "Helpers/String.hpp"
#include "Utility/JsonHelpers.h"
#include "Loader/DataAssetLoader.h"

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace Schema {
	DataAssetLoader::DataAssetLoader() : ModLoaderBase("dataasset") {}

	DataAssetLoader::~DataAssetLoader() {}

    void DataAssetLoader::OnLoad(const std::filesystem::path& LoaderPath, const RC::StringType& ModName, const EEngineLifecyclePhase& EngineLifecyclePhase)
    {
        if (EngineLifecyclePhase != EEngineLifecyclePhase::PostEngineInit)
        {
            return;
        }

        PS::JsonHelpers::ParseJsonFilesInPath(LoaderPath, [&](const nlohmann::json& Data) {
            for (auto& [Key, Value] : Data.items())
            {
                ModifyAsset(Key, Value);
            }
        });
    }

    void DataAssetLoader::OnAutoReload(const RC::StringType& ModName, const std::filesystem::path& ModFilePath)
    {
        PS::JsonHelpers::ParseJsonFileInPath(ModFilePath, [&](const nlohmann::json& Data) {
            for (auto& [Key, Value] : Data.items())
            {
                ModifyAsset(Key, Value);
            }
        });
    }

    bool DataAssetLoader::CanInitialize(const EEngineLifecyclePhase& EngineLifecyclePhase)
    {
        if (EngineLifecyclePhase == EEngineLifecyclePhase::PostEngineInit)
        {
            return true;
        }

        return false;
    }

    bool DataAssetLoader::OnInitialize()
    {
        return true;
    }

    void DataAssetLoader::ModifyAsset(const std::string& AssetName, const nlohmann::json& Data)
    {
        RC::StringType AssetNameWide = RC::to_generic_string(AssetName);

        if (Data.contains("$Template"))
        {
            CreateAsset(FString(AssetNameWide), Data);
            return;
        }

        FSoftObjectPtr SoftObjectPtr = FSoftObjectPtr(FSoftObjectPath(FString(AssetNameWide)));
        UObject* Asset = SoftObjectPtr.LoadSynchronous();
        if (!Asset)
        {
            PS::Log<LogLevel::Error>(STR("Failed to load asset: {}\n"), AssetNameWide);
            return;
        }

        ModifyAsset(Asset, Data);
    }

    void DataAssetLoader::ModifyAsset(UObject* Asset, const nlohmann::json& Data)
    {
        UClass* AssetClass = Asset->GetClassPrivate();
        for (auto& [PropertyName, PropertyValue] : Data.items())
        {
            if (PropertyName.starts_with("$"))
            {
                continue;
            }

            RC::StringType PropertyNameWide = RC::to_generic_string(PropertyName);
            FProperty* Property = Asset->GetPropertyByNameInChain(PropertyNameWide.c_str());

            if (!Property)
            {
                PS::Log<RC::LogLevel::Warning>(TEXT("Property '{}' does not exist in {}\n"), PropertyNameWide, AssetClass->GetNamePrivate().ToString());
                continue;
            }

            void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Asset);
            PropertyHelper::CopyJsonValueToContainer(ValuePtr, Property, PropertyValue);
        }
    }

    void DataAssetLoader::CreateAsset(const FString& AssetPath, const nlohmann::json& Data)
    {
        if (DoesAssetExist(AssetPath))
        {
            PS::Log<LogLevel::Error>(STR("Asset {} already exists, unable to create.\n"), *AssetPath);
            return;
        }

        if (!Data.at("$Template").is_string())
        {
            PS::Log<LogLevel::Error>(STR("Failed to create {}, $Template was not a string.\n"), *AssetPath);
            return;
        }

        std::string TemplatePath = Data.at("$Template").get<std::string>();
        FSoftObjectPtr TemplateSoftObjectPtr = FSoftObjectPtr(FSoftObjectPath(FString(RC::to_generic_string(TemplatePath))));
        UObject* TemplateAsset = TemplateSoftObjectPtr.LoadSynchronous();
        if (!TemplateAsset)
        {
            PS::Log<LogLevel::Error>(STR("Failed to create {}, asset referenced by $Template was not valid.\n"), *AssetPath);
            return;
        }

        UClass* TemplateAssetClass = TemplateAsset->GetClassPrivate();

        TArray<FString> Parts;
        AssetPath.Split(TEXT("."), Parts);

        if (Parts.Num() < 2)
        {
            PS::Log<LogLevel::Error>(STR("Failed to create {}, invalid asset path supplied to $Template.\n"), *AssetPath);
            return;
        }

        FString PackagePath = Parts[0];
        FString AssetName = Parts[1];

        EObjectFlags ObjectFlags = static_cast<EObjectFlags>(RF_Public);
        UPackage* NewPackage = UObjectGlobals::NewObject<UPackage>(nullptr, FName(*PackagePath, FNAME_Add), ObjectFlags);
        NewPackage->SetRootSet();

        UObject* NewAsset = UObjectGlobals::NewObject<UObject>(NewPackage, TemplateAssetClass, FName(*AssetName, FNAME_Add), ObjectFlags);
        NewAsset->SetRootSet();

        for (FProperty* Property : TFieldRange<FProperty>(TemplateAssetClass, EFieldIterationFlags::IncludeSuper))
        {
            void* TemplateValuePtr = Property->ContainerPtrToValuePtr<void>(TemplateAsset);
            void* ValuePtr = Property->ContainerPtrToValuePtr<void>(NewAsset);
            Property->CopyCompleteValue(ValuePtr, TemplateValuePtr);
        }

        ModifyAsset(NewAsset, Data);
    }

    bool DataAssetLoader::DoesAssetExist(const FString& AssetPath)
    {
        FSoftObjectPtr SoftObjectPtr = FSoftObjectPtr(FSoftObjectPath(FString(AssetPath)));
        UObject* Asset = SoftObjectPtr.LoadSynchronous();
        if (Asset)
        {
            return true;
        }

        return false;
    }
}