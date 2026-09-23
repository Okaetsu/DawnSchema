#include "SDK/Classes/KismetSystemLibrary.h"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UFunction.hpp"
#include "Utility/Logging.h"

using namespace RC;
using namespace RC::Unreal;

namespace UECustom {
    void UKismetSystemLibrary::CollectGarbage()
    {
        static auto Function = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, TEXT("/Script/Engine.KismetSystemLibrary:CollectGarbage"));

        if (!Function)
        {
            PS::Log<LogLevel::Error>(STR("Function /Script/Engine.KismetSystemLibrary:CollectGarbage was invalid.\n"));
            return;
        }

        GetDefaultObj()->ProcessEvent(Function, nullptr);
    }

	UKismetSystemLibrary* UKismetSystemLibrary::GetDefaultObj()
	{
		static auto Self = UObjectGlobals::StaticFindObject<UKismetSystemLibrary*>(nullptr, nullptr, TEXT("/Script/Engine.Default__KismetSystemLibrary"));
		return Self;
	}
}