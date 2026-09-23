#pragma once

#include "Helpers/String.hpp"

namespace RC::Unreal {
    class UClass;
}

namespace SDK {
    uintptr_t** GetVTablePtrByClassPath(const RC::StringType& ClassPath);
    void* GetVirtualFunctionFromClass(RC::Unreal::UClass* TargetClass, size_t Index);
    void* GetVirtualFunctionFromVTable(uintptr_t** VTable, size_t Index);
}