#include "Unreal/CoreUObject/UObject/Class.hpp"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "Unreal/FProperty.hpp"
#include "Unreal/NameTypes.hpp"
#include "Unreal/Engine/UDataTable.hpp"
#include "SDK/Structs/Custom/FManagedStruct.h"
#include "SDK/Helper/PropertyHelper.h"
#include "Utility/Logging.h"
#include "Utility/JsonHelpers.h"
#include "Loader/RawTableLoader.h"
#include "SDK/Helper/Memory.h"

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace Schema {
	RawTableLoader::RawTableLoader() : ModLoaderBase("raw") {
        SetDisplayName(TEXT("Raw Table Loader"));
    }

	RawTableLoader::~RawTableLoader() {}

    void RawTableLoader::Apply(RC::Unreal::UDataTable* DataTable)
    {
        std::vector<nlohmann::json>* It = TableDataMap.Find(DataTable->GetFName());
        if (It)
        {
            LoadResult Result{};

            for (auto& Data : *It)
            {
                Apply(Data, DataTable, Result);
            }

            PS::Log<LogLevel::Normal>(STR("{}: {} Rows updated, {} Rows added, {} Rows deleted, {} error{}.\n"),
                DataTable->GetName(), Result.SuccessfulModifications, Result.SuccessfulAdditions,
                Result.SuccessfulDeletions, Result.ErrorCount, Result.ErrorCount > 1 || Result.ErrorCount == 0 ? STR("s") : STR(""));
        }
    }

    void RawTableLoader::Apply(const nlohmann::json& Data, RC::Unreal::UDataTable* DataTable, LoadResult& OutResult)
    {
        for (auto& [DataKey, DataRow] : Data.items())
        {
            FName RowKeyName = FName(RC::to_generic_string(DataKey), FNAME_Add);
            if (DataRow.is_null())
            {
                DeleteRow(DataTable, RowKeyName, OutResult);
                continue;
            }

            uint8* Row = DataTable->FindRowUnchecked(RowKeyName);
            if (!Row)
            {
                AddRow(DataTable, RowKeyName, DataRow, OutResult);
                continue;
            }

            EditRow(DataTable, RowKeyName, Row, DataRow, OutResult);
        }
    }

    void RawTableLoader::OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& EngineLifecyclePhase)
    {
        if (EngineLifecyclePhase != EEngineLifecyclePhase::PostEngineInit)
        {
            return;
        }

        PS::JsonHelpers::ParseJsonFilesInPath(loaderPath, [&](const nlohmann::json& Data) {
            for (auto& [Key, Value] : Data.items())
            {
                auto KeyWide = RC::to_generic_string(Key);
                AddToTableDataMap(KeyWide, Value);
            }
        });

        std::vector<UObject*> OutDataTables;
        UObjectGlobals::FindAllOf(TEXT("DataTable"), OutDataTables);
        for (UObject* DataTable : OutDataTables)
        {
            Apply(static_cast<UDataTable*>(DataTable));
        }
    }

    void RawTableLoader::OnAutoReload(const RC::StringType& modName, const std::filesystem::path& modFilePath)
    {
        PS::JsonHelpers::ParseJsonFileInPath(modFilePath, [&](const nlohmann::json& Data) {
            for (auto& [Key, Value] : Data.items())
            {
                FName RowKeyName = FName(RC::to_generic_string(Key), FNAME_Add);
                LoadResult Result;

                std::vector<UObject*> OutDataTables;
                UObjectGlobals::FindAllOf(TEXT("DataTable"), OutDataTables);
                for (UObject* DataTable : OutDataTables)
                {
                    if (DataTable->GetFName().IsEqual(RowKeyName))
                    {
                        Apply(Value, static_cast<UDataTable*>(DataTable), Result);
                    }
                }
                
                PS::Log<LogLevel::Normal>(STR("{}: {} Rows updated, {} Rows added, {} Rows deleted, {} error{}.\n"),
                    RowKeyName.ToString(), Result.SuccessfulModifications, Result.SuccessfulAdditions,
                    Result.SuccessfulDeletions, Result.ErrorCount, Result.ErrorCount > 1 || Result.ErrorCount == 0 ? STR("s") : STR(""));
            }
        });
    }

    bool RawTableLoader::CanInitialize(const EEngineLifecyclePhase& EngineLifecyclePhase)
    {
        if (EngineLifecyclePhase == EEngineLifecyclePhase::PostEngineInit)
        {
            return true;
        }

        return false;
    }

    bool RawTableLoader::OnInitialize()
    {
        uintptr_t** VTable = SDK::GetVTablePtrByClassPath(TEXT("/Script/Engine.DataTable"));
        if (!VTable)
        {
            PS::Log<LogLevel::Error>(TEXT("Something went wrong with getting VTable pointer for UDataTable.\n"));
            return false;
        }

        void* VFuncPtr = SDK::GetVirtualFunctionFromVTable(VTable, 28);

        SerializeDatatableCallback = [this](UDataTable* Self, FArchive* Ar)
        {
            Apply(Self);
        };

        SerializeDatatableHook = safetyhook::create_inline(VFuncPtr,
            reinterpret_cast<void*>(OnSerializeDatatable));

        return true;
    }

    void RawTableLoader::AddRow(RC::Unreal::UDataTable* DataTable, const FName& RowName, const nlohmann::json& Data, LoadResult& OutResult)
    {
        UScriptStruct* RowStruct = DataTable->GetRowStruct().Get();
        SDK::FManagedStruct NewRowData(RowStruct);

        try
        {
            if (ModifyRowProperties(DataTable, RowName, NewRowData.GetData(), Data, OutResult))
            {
                DataTable->AddRow(RowName, *reinterpret_cast<RC::Unreal::FTableRowBase*>(NewRowData.GetData()));
                OutResult.SuccessfulAdditions++;
            }
        }
        catch (const std::exception& e)
        {
            RC::StringType TableName = DataTable->GetNamePrivate().ToString();
            OutResult.ErrorCount++;
            PS::Log<LogLevel::Error>(STR("Failed to add Row '{}' in {}: {}\n"), RowName.ToString(), TableName, RC::to_generic_string(e.what()));
        }
    }

    void RawTableLoader::EditRow(RC::Unreal::UDataTable* DataTable, const FName& RowName, uint8* Row, const nlohmann::json& Data, LoadResult& OutResult)
    {
        try
        {
            if (ModifyRowProperties(DataTable, RowName, Row, Data, OutResult))
            {
                OutResult.SuccessfulModifications++;
            }
        }
        catch (const std::exception& e)
        {
            RC::StringType TableName = DataTable->GetNamePrivate().ToString();
            OutResult.ErrorCount++;
            PS::Log<LogLevel::Error>(STR("Failed to edit Row '{}' in {}: {}\n"), RowName.ToString(), TableName, RC::to_generic_string(e.what()));
        }
    }

    void RawTableLoader::DeleteRow(RC::Unreal::UDataTable* DataTable, const RC::Unreal::FName& RowName, LoadResult& OutResult)
    {
        DataTable->RemoveRow(RowName);
        OutResult.SuccessfulDeletions++;
    }

    bool RawTableLoader::ModifyRowProperties(RC::Unreal::UDataTable* DataTable, const FName& RowName, void* RowPtr, const nlohmann::json& Data,
        LoadResult& OutResult)
    {
        if (!Data.is_object())
        {
            throw std::runtime_error(std::format("Value for {} must be an object", RC::to_string(RowName.ToString())));
        }

        UScriptStruct* RowStruct = DataTable->GetRowStruct().Get();
        bool WasRowModified = false;
        for (auto& [Key, Value] : Data.items())
        {
            RC::StringType KeyWide = RC::to_generic_string(Key);
            FProperty* Property = RowStruct->GetPropertyByNameInChain(KeyWide.c_str());
            if (Property)
            {
                void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowPtr);
                PropertyHelper::CopyJsonValueToContainer(ValuePtr, Property, Value);
                WasRowModified = true;
            }
            else
            {
                OutResult.ErrorCount++;
                PS::Log<LogLevel::Warning>(STR("Property '{}' not found in Row '{}' in {}.\n"), KeyWide, RowName.ToString(), DataTable->GetNamePrivate().ToString());
            }
        }

        return WasRowModified;
    }

    void RawTableLoader::AddToTableDataMap(const RC::StringType& DataTableName, const nlohmann::json& Data)
    {
        auto DataTableFName = FName(DataTableName, FNAME_Add);
        auto& It = TableDataMap.FindOrAdd(DataTableFName);
        It.push_back(Data);
    }

    void RawTableLoader::OnSerializeDatatable(RC::Unreal::UDataTable* Self, RC::Unreal::FArchive* Ar)
    {
        SerializeDatatableHook.call(Self, Ar);
        SerializeDatatableCallback(Self, Ar);
    }
}