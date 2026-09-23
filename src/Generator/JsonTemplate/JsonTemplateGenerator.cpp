#include <fstream>
#include "Generator/JsonTemplate/JsonTemplateGenerator.h"
#include "SDK/Classes/Custom/UBlueprintGeneratedClass.h"
#include "SDK/Structs/Custom/FManagedStruct.h"
#include "UE4SSProgram.hpp"
#include "Unreal/CoreUObject/UObject/UnrealType.hpp"
#include "Unreal/Engine/UDataTable.hpp"
#include "Unreal/Property/FEnumProperty.hpp"
#include "Unreal/Property/FStrProperty.hpp"
#include "Unreal/Property/FTextProperty.hpp"
#include "Utility/Logging.h"

using namespace RC;
using namespace RC::Unreal;

struct FPropertyJsonInfo
{
    std::string Name;
    nlohmann::json Value;
};

namespace PS::JsonTemplateGenerator {
    FPropertyJsonInfo PropertyToJson(FProperty* Property, void* Container)
    {
        FPropertyJsonInfo PropertyInfo;
        RC::StringType PropertyName = Property->GetName();
        void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Container);

        PropertyInfo.Name = RC::to_string(PropertyName);
        
        if (auto NumericProperty = CastField<FNumericProperty>(Property))
        {
            if (NumericProperty->IsEnum())
            {
                FString PropertyValue;
                Property->ExportTextItem(PropertyValue, ValuePtr, nullptr, nullptr, 0);
                PropertyInfo.Value = RC::to_string(*PropertyValue);
            }
            else if (NumericProperty->IsFloatingPoint())
            {
                PropertyInfo.Value = NumericProperty->GetFloatingPointPropertyValue(ValuePtr);
            }
            else if (NumericProperty->IsInteger())
            {
                PropertyInfo.Value = NumericProperty->GetSignedIntPropertyValue(ValuePtr);
            }
        }
        else if (CastField<FTextProperty>(Property) || 
                 CastField<FStrProperty>(Property)  || 
                 CastField<FNameProperty>(Property) || 
                 CastField<FEnumProperty>(Property))
        {
            FString PropertyValue;
            Property->ExportTextItem(PropertyValue, ValuePtr, nullptr, nullptr, 0);
            PropertyInfo.Value = RC::to_string(*PropertyValue);
        }
        else if (auto BoolProperty = CastField<FBoolProperty>(Property))
        {
            PropertyInfo.Value = BoolProperty->GetPropertyValue(ValuePtr);
        }

        return PropertyInfo;
    }

    void GenerateDataTableTemplate(nlohmann::ordered_json& Json, UDataTable* DataTable)
    {
        RC::StringType DataTableName = DataTable->GetName();
        std::string DataTableNameNarrow = RC::to_string(DataTableName);
        UScriptStruct* RowStruct = DataTable->GetRowStruct().Get();
        auto RowData = Palworld::FManagedStruct(RowStruct);
        auto FirstRow = DataTable->FindRowUnchecked(FName(TEXT("PinkCat")));

        Json[DataTableNameNarrow] = {};
        Json[DataTableNameNarrow]["PinkCat"] = {};

        for (FProperty* Property : TFieldRange<FProperty>(RowStruct, EFieldIterationFlags::IncludeSuper))
        {
            FPropertyJsonInfo PropertyInfo = PropertyToJson(Property, FirstRow);
            Json[DataTableNameNarrow]["PinkCat"][PropertyInfo.Name] = PropertyInfo.Value;
        }
    }

    void GenerateBPTemplate(nlohmann::ordered_json& Json, UClass* BPClass)
    {
        RC::StringType ClassName = BPClass->GetName();
        std::string ClassNameNarrow = RC::to_string(ClassName);
        UObject* DefaultObject = BPClass->GetClassDefaultObject().Get();

        Json[ClassNameNarrow] = {};

        for (FProperty* Property : TFieldRange<FProperty>(BPClass, EFieldIterationFlags::IncludeSuper))
        {
            FPropertyJsonInfo PropertyInfo = PropertyToJson(Property, DefaultObject);
            Json[ClassNameNarrow][PropertyInfo.Name] = PropertyInfo.Value;
        }
    }

    void GenerateTemplate(nlohmann::ordered_json& Json, UObject* Object)
    {
        namespace fs = std::filesystem;

        if (Object->IsA(UBlueprintGeneratedClass::StaticClass()))
        {
            UClass* BPClass = static_cast<UClass*>(Object);
            GenerateBPTemplate(Json, BPClass);
        }
        else if (Object->IsA(UDataTable::StaticClass()))
        {
            UDataTable* DataTable = static_cast<UDataTable*>(Object);
            GenerateDataTableTemplate(Json, DataTable);
        }
        else
        {
            return;
        }

        static auto TemplatesPath = fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "PalSchema" / "templates";

        if (!std::filesystem::exists(TemplatesPath))
        {
            std::filesystem::create_directories(TemplatesPath);
        }

        std::ofstream OutputFile(TemplatesPath / "Template.json");
        OutputFile << Json.dump(4);

        PS::Log<LogLevel::Verbose>(STR("done\n"));
    }
}