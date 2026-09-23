#include "Unreal/CoreUObject/UObject/FStrProperty.hpp"
#include "Unreal/CoreUObject/UObject/UnrealType.hpp"
#include "Unreal/Property/FEnumProperty.hpp"
#include "Unreal/Property/FTextProperty.hpp"
#include "Helpers/Casting.hpp"
#include "SDK/Structs/Custom/FManagedValue.h"
#include "SDK/Structs/Custom/FScriptMapHelper.h"
#include "SDK/Helper/PropertyHelper.h"
#include "Utility/Logging.h"

using namespace RC;
using namespace RC::Unreal;

namespace Schema {
    void PropertyHelper::CopyJsonValueToContainer(void* Container, FProperty* Property, const nlohmann::json& Value)
    {
        if (!Property)
        {
            throw std::runtime_error("A null Property was supplied to PropertyHelper::CopyJsonValueToContainer.");
        }

        ValidateJsonValueType(Property, Value);

        auto PropertyName = Property->GetName();
        auto Type = Property->GetCPPType();
        auto Class = Property->GetClass();
        auto ClassName = Class.GetName();

        if (auto EnumProperty = CastField<FEnumProperty>(Property))
        {
            SetEnumPropertyValueFromJsonValue(Container, EnumProperty, Value);
        }
        else if (auto NumProperty = CastField<FNumericProperty>(Property))
        {
            SetNumericPropertyValueFromJsonValue(Container, NumProperty, Value);
        }
        else if (auto BoolProperty = CastField<FBoolProperty>(Property))
        {
            SetBoolPropertyValueFromJsonValue(Container, BoolProperty, Value);
        }
        else if (auto NameProperty = CastField<FNameProperty>(Property))
        {
            SetNamePropertyValueFromJsonValue(Container, NameProperty, Value);
        }
        else if (auto StrProperty = CastField<FStrProperty>(Property))
        {
            SetStrPropertyValueFromJsonValue(Container, StrProperty, Value);
        }
        else if (auto TextProperty = CastField<FTextProperty>(Property))
        {
            SetTextPropertyValueFromJsonValue(Container, TextProperty, Value);
        }
        else if (auto ClassProperty = CastField<FClassProperty>(Property))
        {
            SetClassPropertyValueFromJsonValue(Container, ClassProperty, Value);
        }
        else if (CastField<FObjectProperty>(Property) && ClassName == STR("ObjectProperty"))
        {
            auto ObjectProperty = CastField<FObjectProperty>(Property);
            SetObjectPropertyValueFromJsonValue(Container, ObjectProperty, Value);
        }
        else if (CastField<FSoftObjectProperty>(Property) && ClassName == STR("SoftObjectProperty"))
        {
            auto SoftObjectProperty = CastField<FSoftObjectProperty>(Property);
            SetSoftObjectPropertyValueFromJsonValue(Container, SoftObjectProperty, Value);
        }
        else if (CastField<FSoftClassProperty>(Property) && ClassName == STR("SoftClassProperty"))
        {
            auto SoftClassProperty = CastField<FSoftClassProperty>(Property);
            SetSoftClassPropertyValueFromJsonValue(Container, SoftClassProperty, Value);
        }
        else if (auto StructProperty = CastField<FStructProperty>(Property))
        {
            SetStructPropertyValueFromJsonValue(Container, StructProperty, Value);
        }
        else if (auto ArrayProperty = CastField<FArrayProperty>(Property))
        {
            SetArrayPropertyValueFromJsonValue(Container, ArrayProperty, Value);
        }
        else if (auto MapProperty = CastField<FMapProperty>(Property))
        {
            SetMapPropertyValueFromJsonValue(Container, MapProperty, Value);
        }
        else
        {
            PS::Log<RC::LogLevel::Warning>(STR("Unhandled property '{}' with class of {} and type of {}\n"), PropertyName, ClassName, Type.GetCharArray());
        }
    }

    int64 PropertyHelper::ParseEnumFromJsonValue(FEnumProperty* Property, const nlohmann::json& Value)
    {
        UEnum* Enum = Property->GetEnum();
        if (!Enum)
        {
            throw std::runtime_error(RC::fmt("EnumProperty %s had an invalid Enum value", Property->GetName().c_str()));
        }

        std::string ParsedValue = Value.get<std::string>();
        if (!ParsedValue.contains("::"))
        {
            ParsedValue = RC::fmt("%S::%S", *Property->GetCPPType(), ParsedValue.c_str());
        }

        FName EnumName = FName(RC::to_generic_string(ParsedValue));

        bool WasEnumFound = false;
        int64_t EnumValue = 0;

        for (const FEnumNamePair& EnumNamePair : Enum->GetEnumNames())
        {
            if (EnumNamePair.Key == EnumName)
            {
                WasEnumFound = true;
                EnumValue = EnumNamePair.Value;
            }
        }

        if (!WasEnumFound)
        {
            throw std::runtime_error(std::format("Enum '{}' doesn't exist", ParsedValue));
        }

        return EnumValue;
    }

    int64 PropertyHelper::ParseByteFromJsonValue(FNumericProperty* Property, const nlohmann::json& Value)
    {
        auto Enum = Property->GetIntPropertyEnum();
        if (!Enum)
        {
            throw std::runtime_error(RC::fmt("EnumProperty %s had an invalid Enum value", Property->GetName().c_str()));
        }

        std::string ParsedValue = Value.get<std::string>();
        if (!ParsedValue.contains("::"))
        {
            ParsedValue = RC::fmt("%S::%S", *Property->GetCPPType(), ParsedValue.c_str());
        }

        FName EnumName = FName(RC::to_generic_string(ParsedValue));

        bool WasEnumFound = false;
        int64_t EnumValue = 0;

        for (const FEnumNamePair& EnumNamePair : Enum->GetEnumNames())
        {
            if (EnumNamePair.Key == EnumName)
            {
                WasEnumFound = true;
                EnumValue = EnumNamePair.Value;
            }
        }

        if (!WasEnumFound)
        {
            throw std::runtime_error(std::format("Enum '{}' doesn't exist", ParsedValue));
        }

        return EnumValue;
    }

    void PropertyHelper::SetEnumPropertyValueFromJsonValue(void* Data, FEnumProperty* Property, const nlohmann::json& Value)
    {
        auto EnumValue = ParseEnumFromJsonValue(Property, Value);
        FMemory::Memcpy(Data, &EnumValue, Property->GetElementSize());
    }

    void PropertyHelper::SetNumericPropertyValueFromJsonValue(void* Data, FNumericProperty* Property, const nlohmann::json& Value)
    {
        auto PropertyName = GetPropertyNameAsUTF8String(Property);

        if (Property->IsEnum())
        {
            auto EnumValue = ParseByteFromJsonValue(Property, Value);
            Property->SetIntPropertyValue(Data, EnumValue);
        }
        else
        {
            if (Property->IsInteger())
            {
                Property->SetIntPropertyValue(Data, Value.get<int64>());
            }
            else if (Property->IsFloatingPoint())
            {
                Property->SetFloatingPointPropertyValue(Data, Value.get<double>());
            }
            else
            {
                PS::Log<RC::LogLevel::Warning>(STR("Unhandled Numeric Type: {}\n"), Property->GetName());
            }
        }
    }

    void PropertyHelper::SetBoolPropertyValueFromJsonValue(void* Data, FBoolProperty* Property, const nlohmann::json& Value)
    {
        Property->SetPropertyValue(Data, Value.get<bool>());
    }

    void PropertyHelper::SetNamePropertyValueFromJsonValue(void* Data, FNameProperty* Property, const nlohmann::json& Value)
    {
        auto ParsedValue = Value.get<std::string>();
        auto Name = FName(RC::to_generic_string(ParsedValue), FNAME_Add);
        Property->SetPropertyValue(Data, Name);
    }

    void PropertyHelper::SetStrPropertyValueFromJsonValue(void* Data, FStrProperty* Property, const nlohmann::json& Value)
    {
        auto ParsedValue = Value.get<std::string>();
        auto String = FString(RC::to_generic_string(ParsedValue).c_str());
        Property->SetPropertyValue(Data, String);
    }

    void PropertyHelper::SetTextPropertyValueFromJsonValue(void* Data, FTextProperty* Property, const nlohmann::json& Value)
    {
        auto StringValue = Value.get<std::string>();
        auto Text = FText(RC::to_generic_string(StringValue).c_str());
        Property->SetPropertyValue(Data, Text);
    }

    void PropertyHelper::SetClassPropertyValueFromJsonValue(void* Data, FClassProperty* Property, const nlohmann::json& Value)
    {
        auto PropertyName = GetPropertyNameAsUTF8String(Property);

        std::string StringValue = Value.get<std::string>();
        if (!StringValue.ends_with("_C"))
        {
            throw std::runtime_error(std::format("ClassProperty path for {} must end with a _C", PropertyName.c_str()));
        }

        RC::StringType StringValueWide = RC::to_generic_string(StringValue);
        FSoftObjectPtr SoftObjectPtr = FSoftObjectPtr(FSoftObjectPath(FString(StringValueWide)));
        UObject* Asset = SoftObjectPtr.LoadSynchronous();

        if (!Asset)
        {
            throw std::runtime_error(std::format("Property {} was supplied an invalid class of {}", PropertyName, StringValue));
        }

        Asset->SetRootSet();

        Property->SetPropertyValue(Data, Asset);
    }

    void PropertyHelper::SetObjectPropertyValueFromJsonValue(void* Data, FObjectProperty* Property, const nlohmann::json& Value)
    {
        if (Value.is_string())
        {
            std::string StringValue = Value.get<std::string>();
            RC::StringType WideStringValue = RC::to_generic_string(StringValue);
            FSoftObjectPtr SoftObjectPtr = FSoftObjectPtr(FSoftObjectPath(FString(WideStringValue)));
            auto LoadedObject = SoftObjectPtr.LoadSynchronous();

            if (!LoadedObject)
            {
                throw std::runtime_error(RC::fmt("Unable to apply changes to %S. Asset was invalid.", Property->GetName().c_str()));
            }

            auto& ExpectedClass = Property->GetPropertyClass();
            if (!LoadedObject->IsA(ExpectedClass))
            {
                throw std::runtime_error(RC::fmt(
                    "Unable to apply changes to %S. Asset didn't match the expected class for this property. Expected '%S', got '%S'", 
                    Property->GetName().c_str(),
                    ExpectedClass->GetName().c_str(),
                    LoadedObject->GetClassPrivate()->GetName().c_str())
                );
            }

            Property->SetPropertyValue(Data, LoadedObject);
        }
        else if (Value.is_object())
        {
            auto ObjectValue = *Property->ContainerPtrToValuePtr<UObject*>(Data);
            auto ParsedValue = Value.get<nlohmann::json>();
            if (ObjectValue)
            {
                for (auto& [InnerKey, InnerValue] : Value.items())
                {
                    auto ObjectValue_PropertyName = RC::to_generic_string(InnerKey);
                    auto ObjectValue_Property = ObjectValue->GetPropertyByNameInChain(ObjectValue_PropertyName.c_str());

                    if (!ObjectValue_Property)
                    {
                        ObjectValue_Property = ObjectValue->GetClassPrivate()->GetPropertyByNameInChain(ObjectValue_PropertyName.c_str());
                    }

                    if (ObjectValue_Property)
                    {
                        CopyJsonValueToContainer(ObjectValue, ObjectValue_Property, InnerValue);
                    }
                }
            }
        }
    }

    void PropertyHelper::SetSoftClassPropertyValueFromJsonValue(void* Data, FSoftClassProperty* Property, const nlohmann::json& Value)
    {
        std::string ParsedValue = Value.get<std::string>();
        if (!ParsedValue.ends_with("_C"))
        {
            throw std::runtime_error(RC::fmt("SoftClassProperty path for %S must end with a _C", Property->GetName().c_str()));
        }

        RC::StringType String = RC::to_generic_string(ParsedValue);
        FSoftObjectPtr SoftClassPtr = FSoftObjectPtr(FSoftObjectPath(FString(String)));
        Property->SetPropertyValue(Data, SoftClassPtr);
    }

    void PropertyHelper::SetSoftObjectPropertyValueFromJsonValue(void* Data, FSoftObjectProperty* Property, const nlohmann::json& Value)
    {
        const std::string resourcePrefix = "$resource/";

        auto ParsedValue = Value.get<std::string>();

        RC::StringType SoftObjectPath = RC::to_generic_string(ParsedValue);

        if (ParsedValue.starts_with(resourcePrefix))
        {
            // Before: "$resource/modname/resourcename"
            // After:  "modname/resourcename"
            SoftObjectPath = SoftObjectPath.erase(0, resourcePrefix.length());

            // "/Engine/Transient.DawnSchema/Resources/modname/resourcename"
            SoftObjectPath = std::format(TEXT("/Engine/Transient.DawnSchema/Resources/{}"), SoftObjectPath);
        }

        auto SoftObjectPtr = FSoftObjectPtr(FSoftObjectPath(FString(SoftObjectPath)));
        Property->SetPropertyValue(Data, SoftObjectPtr);
    }

    void PropertyHelper::SetStructPropertyValueFromJsonValue(void* Data, FStructProperty* Property, const nlohmann::json& Value)
    {
        auto ParsedObject = Value.get<nlohmann::json>();

        auto Struct = Property->GetStruct();
        if (!Struct)
        {
            throw std::runtime_error(std::format("Failed to get Struct"));
        }

        for (FProperty* FieldProperty : TFieldRange<FProperty>(Struct, EFieldIterationFlags::None))
        {
            auto FieldName = GetPropertyNameAsUTF8String(static_cast<FProperty*>(FieldProperty));
            if (Value.contains(FieldName))
            {
                void* FieldValuePtr = FieldProperty->ContainerPtrToValuePtr<void>(Data);
                CopyJsonValueToContainer(FieldValuePtr, FieldProperty, Value.at(FieldName));
            }
        }
    }

    void PropertyHelper::SetArrayPropertyValueFromJsonValue(void* Data, FArrayProperty* Property, const nlohmann::json& Value)
    {
        FScriptArrayHelper ArrayHelper(Property, Data);
        FProperty* InnerProperty = Property->GetInner();

        if (Value.is_object())
        {
            if (Value.contains("Action"))
            {
                std::string Action = Value.at("Action").get<std::string>();
                if (Action == "Clear")
                {
                    ArrayHelper.EmptyValues();
                }
            }

            if (Value.contains("Items"))
            {
                if (!Value.at("Items").is_array())
                {
                    throw std::runtime_error(std::format("Field Items must be an array"));
                }

                auto Items = Value.at("Items").get<nlohmann::json::array_t>();
                for (auto& Item : Items)
                {
                    int32 NewIndex = ArrayHelper.AddValue();
                    uint8* RawPtr = ArrayHelper.GetRawPtr(NewIndex);
                    CopyJsonValueToContainer(RawPtr, InnerProperty, Item);
                }
            }
        }
        else if (Value.is_array())
        {
            ArrayHelper.EmptyValues();

            auto Items = Value.get<nlohmann::json::array_t>();
            for (auto& Item : Items)
            {
                int32 NewIndex = ArrayHelper.AddValue();
                uint8* RawPtr = ArrayHelper.GetRawPtr(NewIndex);
                CopyJsonValueToContainer(RawPtr, InnerProperty, Item);
            }
        }
    }

    void PropertyHelper::SetMapPropertyValueFromJsonValue(void* Data, FMapProperty* Property, const nlohmann::json& Value)
    {
        auto ArrayItems = Value.get<std::vector<nlohmann::json>>();

        FProperty* KeyProperty = Property->GetKeyProp();
        FProperty* ValueProperty = Property->GetValueProp();

        FScriptMapLayout MapLayout = Property->GetMapLayout();
        FScriptMap* ScriptMap = static_cast<FScriptMap*>(Data);
        auto ScriptMapHelper = UECustom::FScriptMapHelper(ScriptMap, MapLayout, KeyProperty, ValueProperty);

        for (const auto& Entry : ArrayItems)
        {
            if (!Entry.contains("Key"))
            {
                throw std::runtime_error("Missing 'Key' property.");
            }

            UECustom::FManagedValue ScopedPair;

            ScriptMapHelper.InitializePair(ScopedPair);

            CopyJsonValueToContainer(ScopedPair.GetData(), KeyProperty, Entry.at("Key"));

            if (Entry.contains("Action") && Entry.at("Action").is_string())
            {
                std::string Action = Entry.at("Action").get<std::string>();
                if (Action == "Remove")
                {
                    ScriptMapHelper.Remove(ScopedPair.GetData());
                    continue;
                }
            }

            if (!Entry.contains("Value"))
            {
                throw std::runtime_error("Missing 'Value' property.");
            }

            CopyJsonValueToContainer(static_cast<uint8*>(ScopedPair.GetData()) + MapLayout.ValueOffset, ValueProperty, Entry.at("Value"));
            ScriptMapHelper.Add(ScopedPair);
        }

        ScriptMap->Rehash(MapLayout,
        [&](const void* Src) -> uint32 {
            return KeyProperty->GetValueTypeHash(Src);
        });
    }

    void PropertyHelper::ValidateJsonValueType(FProperty* Property, const nlohmann::json& Value)
    {
        auto PropertyName = GetPropertyNameAsUTF8String(Property);
        auto PropertyClass = Property->GetClass();
        auto PropertyClassName = PropertyClass.GetName();

        if (auto EnumProperty = CastField<FEnumProperty>(Property))
        {
            if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", PropertyName));
        }
        else if (auto NumProperty = CastField<FNumericProperty>(Property))
        {
            if (!Value.is_number()) throw std::runtime_error(std::format("Property {} must be a number", PropertyName));
        }
        else if (auto BoolProperty = CastField<FBoolProperty>(Property))
        {
            if (!Value.is_boolean()) throw std::runtime_error(std::format("Property {} must be a boolean", PropertyName));
        }
        else if (auto NameProperty = CastField<FNameProperty>(Property))
        {
            if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", PropertyName));
        }
        else if (auto StrProperty = CastField<FStrProperty>(Property))
        {
            if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", PropertyName));
        }
        else if (auto TextProperty = CastField<FTextProperty>(Property))
        {
            if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", PropertyName));
        }
        else if (auto ClassProperty = CastField<FClassProperty>(Property))
        {
            if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", PropertyName));
        }
        else if (auto ObjectProperty = CastField<FObjectProperty>(Property) && PropertyClassName == STR("ObjectProperty"))
        {
            if (!Value.is_object() && !Value.is_string()) throw std::runtime_error(std::format("Property {} must be an object or string", PropertyName));
        }
        else if (auto SoftObjectProperty = CastField<FSoftObjectProperty>(Property) && PropertyClassName == STR("SoftObjectProperty"))
        {
            if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", PropertyName));
        }
        else if (auto SoftClassProperty = CastField<FSoftClassProperty>(Property) && PropertyClassName == STR("SoftClassProperty"))
        {
            if (!Value.is_string()) throw std::runtime_error(std::format("Property {} must be a string", PropertyName));
        }
        else if (auto StructProperty = CastField<FStructProperty>(Property))
        {
            if (!Value.is_object()) throw std::runtime_error(std::format("Property {} must be an object", PropertyName));
        }
        else if (auto ArrayProperty = CastField<FArrayProperty>(Property))
        {
            if (!Value.is_object() && !Value.is_array()) throw std::runtime_error(std::format("Property {} must be an object or array", PropertyName));
        }
        else if (auto MapProperty = CastField<FMapProperty>(Property))
        {
            if (!Value.is_array()) throw std::runtime_error(std::format("Property {} must be an array of objects", PropertyName));
        }
    }

    std::string PropertyHelper::GetPropertyNameAsUTF8String(FProperty* Property)
    {
        auto PropertyName = Property->GetName();
        auto PropertyNameUTF8 = RC::to_string(PropertyName);
        return PropertyNameUTF8;
    }
}