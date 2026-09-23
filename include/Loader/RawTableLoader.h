#pragma once

#include "Unreal/NameTypes.hpp"
#include "Loader/ModLoaderBase.h"
#include "nlohmann/json.hpp"
#include "safetyhook.hpp"

namespace RC::Unreal {
    class UDataTable;
}

namespace Schema {
	class RawTableLoader : public ModLoaderBase {
        struct LoadResult {
            int SuccessfulModifications = 0;
            int SuccessfulAdditions = 0;
            int SuccessfulDeletions = 0;
            int ErrorCount = 0;
        };
	public:
		RawTableLoader();

		~RawTableLoader();

		void Initialize();

        void Apply(RC::Unreal::UDataTable* DataTable);

        void Apply(const nlohmann::json& Data, RC::Unreal::UDataTable* Table, LoadResult& OutResult);
    protected:
        virtual void OnLoad(const std::filesystem::path& LoaderPath, const RC::StringType& ModName, const EEngineLifecyclePhase& EngineLifecyclePhase) override final;
        virtual void OnAutoReload(const RC::StringType& ModName, const std::filesystem::path& ModFilePath) override final;

        virtual bool CanInitialize(const EEngineLifecyclePhase& EngineLifecyclePhase) override final;
        virtual bool OnInitialize() override final;
    private:
        RC::Unreal::TMap<RC::Unreal::FName, std::vector<nlohmann::json>> TableDataMap;

        void AddRow(RC::Unreal::UDataTable* DataTable, const RC::Unreal::FName& RowName, const nlohmann::json& Data, LoadResult& OutResult);

        void EditRow(RC::Unreal::UDataTable* DataTable, const RC::Unreal::FName& RowName, RC::Unreal::uint8* Row, const nlohmann::json& Data, LoadResult& OutResult);

        void DeleteRow(RC::Unreal::UDataTable* DataTable, const RC::Unreal::FName& RowName, LoadResult& OutResult);

        bool ModifyRowProperties(RC::Unreal::UDataTable* DataTable, const RC::Unreal::FName& RowName, void* RowPtr, const nlohmann::json& Data, LoadResult& OutResult);

        void AddToTableDataMap(const RC::StringType& DataTableName, const nlohmann::json& Data);
    private:
        static inline SafetyHookInline SerializeDatatableHook;
        static inline std::function<void(RC::Unreal::UDataTable*, RC::Unreal::FArchive*)> SerializeDatatableCallback = nullptr;
        static void OnSerializeDatatable(RC::Unreal::UDataTable* Self, RC::Unreal::FArchive* Ar);
	};
}