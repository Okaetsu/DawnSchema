#pragma once

#include <memory>
#include "Unreal/FProperty.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "Helpers/String.hpp"
#include "SDK/Helper/PropertyHelper.h"

namespace UECustom {
    struct BaseReflectedStruct {
    public:
        BaseReflectedStruct(RC::Unreal::UScriptStruct* InScriptStruct);
        BaseReflectedStruct(RC::Unreal::UScriptStruct* InScriptStruct, void* InData);
        virtual ~BaseReflectedStruct();

        void DestroyStruct();

        void* GetData();

        virtual RC::Unreal::UScriptStruct* StaticStruct() = 0;
    protected:
        template <typename T>
        void SetPropertyValue(RC::Unreal::FProperty* Property, const T& Value)
        {
            auto ValuePtr = Property->ContainerPtrToValuePtr<T>(Data);
            *ValuePtr = Value;
        }

        template <typename T>
        T GetPropertyValue(RC::Unreal::FProperty* Property)
        {
            auto ValuePtr = Property->ContainerPtrToValuePtr<T>(Data);
            return *ValuePtr;
        }

        // This will return a nullptr if the Property doesn't exist.
        RC::Unreal::FProperty* GetProperty(const RC::StringType& PropertyName)
        {
            auto Property = ScriptStruct->GetPropertyByNameInChain(PropertyName.c_str());
            if (!Property)
            {
                return nullptr;
            }

            return Property;
        }

        // This will throw an error if the Property doesn't exist or if the type didn't match what was supplied.
        template <RC::Unreal::FFieldDerivative T>
        T* GetPropertyChecked(const RC::StringType& PropertyName)
        {
            auto Property = ScriptStruct->GetPropertyByNameInChain(PropertyName.c_str());
            if (!Property)
            {
                throw std::runtime_error(RC::fmt("Property '%S' does not exist in struct '%S'.", PropertyName.c_str(), 
                    ScriptStruct->GetNamePrivate().ToString().c_str()));
            }

            T* ReturnValue = RC::Unreal::CastField<T>(Property);
            if (!ReturnValue)
            {
                throw std::runtime_error(RC::fmt("Property '%S' has the wrong type, expected '%S'.", PropertyName.c_str(), *Property->GetCPPType()));
            }

            return ReturnValue;
        }
    private:
        RC::Unreal::UScriptStruct* ScriptStruct = nullptr;
        void* Data = nullptr;
    };
}