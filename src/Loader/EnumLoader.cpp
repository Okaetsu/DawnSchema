#include "Loader/EnumLoader.h"
#include "Helpers/String.hpp"
#include "Utility/Logging.h"
#include "Utility/JsonHelpers.h"
#include "Unreal/CoreUObject/UObject/Class.hpp"
#include "Unreal/UEnum.hpp"
#include "Unreal/UObjectGlobals.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace Schema {
    EnumLoader::EnumLoader() : ModLoaderBase("enums") {
        SetDisplayName(TEXT("Enum Loader"));
    }

    EnumLoader::~EnumLoader() {}

    void EnumLoader::OnLoad(const std::filesystem::path& LoaderPath, const RC::StringType& ModName, const EEngineLifecyclePhase& EngineLifecyclePhase)
    {
        if (EngineLifecyclePhase != EEngineLifecyclePhase::PostEngineInit)
        {
            return;
        }

        PS::JsonHelpers::ParseJsonFilesInPath(LoaderPath, [&](const nlohmann::json& Data) {
            LoadEnums(Data);
        });
    }

    bool EnumLoader::CanInitialize(const EEngineLifecyclePhase& EngineLifecyclePhase)
    {
        if (EngineLifecyclePhase == EEngineLifecyclePhase::PostEngineInit)
        {
            return true;
        }

        return false;
    }

    bool EnumLoader::OnInitialize()
    {
        std::vector<UObject*> Results;

        PS::Log<LogLevel::Verbose>(STR("UClass for UEnum found, fetching UEnums...\n"));
        UObjectGlobals::FindAllOf(TEXT("Enum"), Results);

        int AddedUEnums = 0;
        for (auto& Result : Results)
        {
            EnumNameToObjectMap.emplace(Result->GetName(), static_cast<UEnum*>(Result));
            AddedUEnums++;
        }

        PS::Log<LogLevel::Verbose>(STR("Finished mapping {} UEnums.\n"), AddedUEnums);

        return true;
    }

    void EnumLoader::LoadEnums(const nlohmann::json& Data)
    {
        for (auto& [EnumNamespace, EnumValues] : Data.items())
        {
            auto EnumNamespaceWide = RC::to_generic_string(EnumNamespace);
            if (!EnumValues.is_array())
            {
                throw std::runtime_error(std::format("Values in {} must be arrays of strings", EnumNamespace));
            }

            auto EnumObject = GetEnumByName(EnumNamespaceWide);
            if (!EnumObject) {
                throw std::runtime_error(std::format("Enum object {} was invalid.", EnumNamespace));
            }

            for (auto& EnumValue : EnumValues)
            {
                if (!EnumValue.is_string()) throw std::runtime_error(std::format("Array must only contain strings"));

                auto EnumValueString = EnumValue.get<std::string>();
                if (EnumValueString.find(':') != std::string::npos) {
                    throw std::runtime_error(
                        std::format("Enum value '{}' must not contain the namespace. Example: Write ExampleEnum instead of {}::ExampleEnum",
                            EnumValueString, EnumNamespace));
                }

                auto EnumValueStringWide = std::format(STR("{}::{}"), EnumNamespaceWide, RC::to_generic_string(EnumValueString));

                auto EnumName = FName(EnumValueStringWide, FNAME_Add);
                int32 IndexToInsertAt = EnumObject->NumEnums() - 1;

                FEnumNamePair EnumNamePair;
                EnumNamePair.Key = EnumName;
                EnumNamePair.Value = IndexToInsertAt;

                auto ResultIndex = EnumObject->InsertIntoNames(EnumNamePair, IndexToInsertAt, true);
                if (ResultIndex < 0)
                {
                    throw std::runtime_error(std::format("Something went wrong adding the enum {}", EnumValueString));
                }

                PS::Log<LogLevel::Normal>(STR("Enum value {} has been added to {}.\n"), EnumValueStringWide, EnumNamespaceWide);
            }
        }
    }

    RC::Unreal::UEnum* EnumLoader::GetEnumByName(const RC::StringType& Name)
    {
        auto EnumMapIterator = EnumNameToObjectMap.find(Name);
        if (EnumMapIterator != EnumNameToObjectMap.end())
        {
            return EnumMapIterator->second;
        }

        return nullptr;
    }
}