#include "SDK/Structs/Custom/FManagedStruct.h"
#include "Unreal/Core/HAL/UnrealMemory.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "DynamicOutput/DynamicOutput.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace SDK {
    FManagedStruct::FManagedStruct(UScriptStruct* InStruct)
    {
        Struct = InStruct;
        Data = FMemory::Malloc(Struct->GetStructureSize());
        Struct->InitializeStruct(Data);
    }

    FManagedStruct::~FManagedStruct()
    {
        Struct->DestroyStruct(Data);
        FMemory::Free(Data);
    }

    void* FManagedStruct::GetData()
    {
        return Data;
    }
}
