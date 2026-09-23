#pragma once

#include "Unreal/UObject.hpp"
#include "Unreal/SoftObjectPtr.hpp"


namespace UECustom {
	class UKismetSystemLibrary : public RC::Unreal::UObject {
	public:
        static void CollectGarbage();
	private:
		static UKismetSystemLibrary* GetDefaultObj();
	};
}