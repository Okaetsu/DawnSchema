#pragma once

#include "nlohmann/json.hpp"

namespace RC::Unreal {
    class UObject;
    class UClass;
}

namespace PS::JsonTemplateGenerator {
    void GenerateTemplate(nlohmann::ordered_json& Json, RC::Unreal::UObject* Object);
}