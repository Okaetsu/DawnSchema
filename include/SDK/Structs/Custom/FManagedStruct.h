#pragma once

namespace RC::Unreal {
    class UScriptStruct;
}

namespace SDK {
    // A wrapper for UScriptStruct that automatically allocates and deallocates memory for the struct data in constructor/destructor.
    // Should only be passed to functions that make a copy of the internal data, like UDataTable::AddRow.
    class FManagedStruct {
    public:
        FManagedStruct(RC::Unreal::UScriptStruct* InStruct);

        ~FManagedStruct();

        void* GetData();
    private:
        void* Data;
        RC::Unreal::UScriptStruct* Struct;
    };
}