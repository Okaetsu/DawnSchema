#include "Mod/CppUserModBase.hpp"
#include "UE4SSProgram.hpp"
#include "Unreal/Hooks.hpp"
#include "Loader/MainLoader.h"
#include "Generator/JsonSchema/JsonSchemaGenerator.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/SignatureManager.h"
#include "../version.h"

using namespace RC;
using namespace RC::Unreal;

class DawnSchema : public RC::CppUserModBase
{
public:
    DawnSchema() : CppUserModBase()
    {
        auto Version = std::format(STR("{}.{}.{}"), VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);

        ModName = STR("DawnSchema");
        ModVersion = Version;
        ModDescription = STR("Allows runtime modification of assets.");
        ModAuthors = STR("Okaetsu");

        auto config = PS::PSConfig::Get();
        config->Load();

        SDK::SignatureManager::Initialize();

        PS::Log<RC::LogLevel::Normal>(STR("{} v{} by {} loaded.\n"), ModName, ModVersion, ModAuthors);
    }

    ~DawnSchema() override
    {
    }

    auto has_member_variable_layout() -> bool
    {
        namespace fs = std::filesystem;
        auto MemberVariableLayoutFile = fs::path(UE4SSProgram::get_program().get_working_directory()) / "MemberVariableLayout.ini";
        return fs::exists(MemberVariableLayoutFile);
    }

    auto render_schema_generator()
    {
        static bool bGeneratingSchemas = false;
        if (ImGui::Button("Generate JSON Schema Files"))
        {
            if (!bGeneratingSchemas)
            {
                bGeneratingSchemas = true;
                PS::JsonSchemaGenerator::GenerateSchemaFiles();
                bGeneratingSchemas = false;
            }
        }

        if (bGeneratingSchemas)
        {
            ImGui::ProgressBar(-0.5f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Generating...");
        }
    }

    auto on_ui_init() -> void override
    {
        register_tab(STR("DawnSchema"), [](CppUserModBase* instance) {
            UE4SS_ENABLE_IMGUI()

            auto mod = dynamic_cast<DawnSchema*>(instance);
            if (!mod)
            {
                return;
            }

            // ImGui::SeparatorText("Generators");
            // mod->render_schema_generator();
        });

        PS::Log<LogLevel::Verbose>(STR("Finished registering DawnSchema tab for GUI Console.\n"));
    }

    auto on_update() -> void override
    {
    }

    auto on_program_start() -> void override
    {
    }

    auto on_unreal_init() -> void override
    {
        Unreal::Hook::RegisterEngineTickPreCallback([this](auto&, Unreal::UEngine*, float, bool) 
        {
            MainLoader.Initialize();
        },
        { true, true, ModName, STR("RunOnGameThread")});
    }
private:
    Schema::MainLoader MainLoader;
};


#define DAWNSCHEMA_API __declspec(dllexport)
extern "C"
{
    DAWNSCHEMA_API RC::CppUserModBase* start_mod()
    {
        return new DawnSchema();
    }

    DAWNSCHEMA_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
