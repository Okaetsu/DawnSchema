#pragma once

#include "nlohmann/json.hpp"

namespace RC::Unreal {
    class UScriptStruct;
    class FProperty;
    class FFieldClass;
    class FNumericProperty;
    class FEnumProperty;
    class FBoolProperty;
    class FNameProperty;
    class FStrProperty;
    class FTextProperty;
    class FClassProperty;
    class FObjectProperty;
    class FSoftClassProperty;
    class FSoftObjectProperty;
    class FStructProperty;
    class FArrayProperty;
    class FMapProperty;
}

namespace Schema::PropertyHelper {
    void CopyJsonValueToContainer(void* Container, RC::Unreal::FProperty* Property, const nlohmann::json& Value);

    RC::Unreal::int64 ParseEnumFromJsonValue(RC::Unreal::FEnumProperty* Property, const nlohmann::json& Value);

    RC::Unreal::int64 ParseByteFromJsonValue(RC::Unreal::FNumericProperty* Property, const nlohmann::json& Value);

    void SetEnumPropertyValueFromJsonValue(void* Data, RC::Unreal::FEnumProperty* Property, const nlohmann::json& Value);

    void SetNumericPropertyValueFromJsonValue(void* Data, RC::Unreal::FNumericProperty* Property, const nlohmann::json& Value);

    void SetBoolPropertyValueFromJsonValue(void* Data, RC::Unreal::FBoolProperty* Property, const nlohmann::json& Value);

    void SetNamePropertyValueFromJsonValue(void* Data, RC::Unreal::FNameProperty* Property, const nlohmann::json& Value);

    void SetStrPropertyValueFromJsonValue(void* Data, RC::Unreal::FStrProperty* Property, const nlohmann::json& Value);

    void SetTextPropertyValueFromJsonValue(void* Data, RC::Unreal::FTextProperty* Property, const nlohmann::json& Value);

    void SetClassPropertyValueFromJsonValue(void* Data, RC::Unreal::FClassProperty* Property, const nlohmann::json& Value);

    void SetObjectPropertyValueFromJsonValue(void* Data, RC::Unreal::FObjectProperty* Property, const nlohmann::json& Value);

    void SetSoftClassPropertyValueFromJsonValue(void* Data, RC::Unreal::FSoftClassProperty* Property, const nlohmann::json& Value);

    void SetSoftObjectPropertyValueFromJsonValue(void* Data, RC::Unreal::FSoftObjectProperty* Property, const nlohmann::json& Value);

    void SetStructPropertyValueFromJsonValue(void* Data, RC::Unreal::FStructProperty* Property, const nlohmann::json& Value);

    void SetArrayPropertyValueFromJsonValue(void* Data, RC::Unreal::FArrayProperty* Property, const nlohmann::json& Value);

    void SetMapPropertyValueFromJsonValue(void* Data, RC::Unreal::FMapProperty* Property, const nlohmann::json& Value);

    // Throws a runtime error if the validation failed
    void ValidateJsonValueType(RC::Unreal::FProperty* Property, const nlohmann::json& Value);

    std::string GetPropertyNameAsUTF8String(RC::Unreal::FProperty* Property);
}