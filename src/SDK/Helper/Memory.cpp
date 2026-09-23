#include "SDK/Helper/Memory.h"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/CoreUObject/UObject/UnrealType.hpp"
#include "Utility/Logging.h"

using namespace RC;
using namespace RC::Unreal;

namespace SDK {
    uintptr_t** GetVTablePtrByClassPath(const RC::StringType& ClassPath)
    {
        auto ClassObject = UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, ClassPath.c_str(), false);
        if (!ClassObject)
        {
            PS::Log<LogLevel::Error>(STR("Unable to get VTable pointer for {}, class not found.\n"), ClassPath);
            return nullptr;
        }

        auto& CDO = ClassObject->GetClassDefaultObject();
        if (!CDO)
        {
            PS::Log<LogLevel::Error>(STR("Unable to get VTable pointer for {}, class default object not found.\n"), ClassPath);
            return nullptr;
        }

        uintptr_t** VTablePtr = *(uintptr_t***)CDO;
        return VTablePtr;
    }

    void* GetVirtualFunctionFromClass(RC::Unreal::UClass* TargetClass, size_t Index)
    {
        auto& CDO = TargetClass->GetClassDefaultObject();
        if (!CDO)
        {
            PS::Log<LogLevel::Error>(STR("Unable to get VTable pointer for {}, class default object not found.\n"), TargetClass->GetFullName());
            return nullptr;
        }

        uintptr_t** VTablePtr = *(uintptr_t***)CDO;
        void* VFuncPtr = (void*)VTablePtr[Index];

        return VFuncPtr;
    }

    void* GetVirtualFunctionFromVTable(uintptr_t** VTable, size_t Index)
    {
        void* VFuncPtr = (void*)VTable[Index];
        return VFuncPtr;
    }
}