#include "SDK/Classes/KismetInternationalizationLibrary.h"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UFunction.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace SDK {
	FString UKismetInternationalizationLibrary::GetCurrentLanguage()
	{
		static auto Function = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, TEXT("/Script/Engine.KismetInternationalizationLibrary:GetCurrentLanguage"));

        if (!Function)
        {
            return FString{};
        }

		struct {
			FString ReturnValue;
		}params;

		GetDefaultObj()->ProcessEvent(Function, &params);

		return params.ReturnValue;
	}

	UKismetInternationalizationLibrary* UKismetInternationalizationLibrary::GetDefaultObj()
	{
		static auto Self = UObjectGlobals::StaticFindObject<UKismetInternationalizationLibrary*>(nullptr, nullptr, TEXT("/Script/Engine.Default__KismetInternationalizationLibrary"));
		return Self;
	}
}