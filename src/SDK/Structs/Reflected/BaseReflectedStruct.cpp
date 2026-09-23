#include "SDK/Structs/Reflected/BaseReflectedStruct.h"

using namespace RC;
using namespace RC::Unreal;

namespace UECustom {
    BaseReflectedStruct::BaseReflectedStruct(RC::Unreal::UScriptStruct* scriptStruct) : ScriptStruct(scriptStruct)
    {
        Data = FMemory::Malloc(ScriptStruct->GetStructureSize());
        ScriptStruct->InitializeStruct(Data);
    }

    BaseReflectedStruct::BaseReflectedStruct(RC::Unreal::UScriptStruct* scriptStruct, void* data) : ScriptStruct(scriptStruct), Data(data)
    {
    }

    BaseReflectedStruct::~BaseReflectedStruct()
    {
    }

    void BaseReflectedStruct::DestroyStruct()
    {
        ScriptStruct->DestroyStruct(Data);
        FMemory::Free(Data);
    }

    void* BaseReflectedStruct::GetData()
    {
        return Data;
    }
}