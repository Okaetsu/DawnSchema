#include <UE4SS/mod_api.h>
#include "Mod/CppUserModBase.hpp"
#include "UE4SSProgram.hpp"
#include "Loader/PalMainLoader.h"
#include "Generator/JsonSchema/JsonSchemaGenerator.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/PalSignatures.h"
#include "SDK/Classes/Async.h"
#include "SDK/UnrealOffsets.h"
#include "../version.h"

using namespace RC;
using namespace RC::Unreal;

class PalSchema : public RC::CppUserModBase
{
public:
    PalSchema() : CppUserModBase()
    {
        RC::StringType Version = STR("0.6.71");

        ModName = STR("PalSchema");

        ModVersion = Version;

        ModDescription = STR("Allows modifying of Palworld's assets dynamically.");

        ModAuthors = STR("Okaetsu");


        if (!has_member_variable_layout())
        {
            PS::Log<LogLevel::Error>(STR("MemberVariableLayout.ini is missing, unable to start PalSchema. Please ensure you are using UE4SS from https://github.com/Okaetsu/RE-UE4SS/releases/tag/experimental-palworld which comes with MemberVariableLayout.ini\n"));
            return;
        }

        auto config = PS::PSConfig::Get();
        config->Load();

        PS::Log<LogLevel::Verbose>(STR("Initializing SignatureManager...\n"));
        Palworld::SignatureManager::Initialize();

        PS::Log<LogLevel::Verbose>(STR("Initializing UnrealOffsets...\n"));
        Palworld::UnrealOffsets::Initialize();

        PS::Log<LogLevel::Verbose>(STR("Preparing to pre-initialize PalSchema...\n"));
        MainLoader.PreInitialize();

        PS::Log<RC::LogLevel::Normal>(STR("{} v{} by {} loaded.\n"), ModName, ModVersion, ModAuthors);
    }

    ~PalSchema() override
    {
    }

    auto has_member_variable_layout() -> bool
    {
        namespace fs = std::filesystem;
        auto MemberVariableLayoutFile = fs::path(RC::UE4SSProgram::get_program().get_working_directory()) / "MemberVariableLayout.ini";
        return fs::exists(MemberVariableLayoutFile);
    }

#ifdef _WIN32
    auto render_schema_generator()
    {
        static bool bGeneratingSchemas = false;
        if (ImGui::Button("Generate JSON Schema Files"))
        {
            if (!bGeneratingSchemas)
            {
                bGeneratingSchemas = true;
                UECustom::AsyncTask(UECustom::ENamedThreads::GameThread, [&]() {
                    PS::JsonSchemaGenerator::GenerateSchemaFiles();
                    bGeneratingSchemas = false;
                });
            }
        }

        if (bGeneratingSchemas)
        {
            ImGui::ProgressBar(-0.5f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Generating...");
        }
    }

    auto on_ui_init() -> void override
    {
        register_tab(STR("Pal Schema"), [](CppUserModBase* instance) {
            UE4SS_ENABLE_IMGUI()

            auto mod = dynamic_cast<PalSchema*>(instance);
            if (!mod)
            {
                return;
            }

            ImGui::SeparatorText("Generators");
            mod->render_schema_generator();
        });

        PS::Log<LogLevel::Verbose>(STR("Finished registering Pal Schema tab for GUI Console.\n"));
    }

#endif

    auto on_update() -> void override
    {
    }

    auto on_program_start() -> void override
    {
    }

    auto on_unreal_init() -> void override
    {
        MainLoader.Initialize();
    }
private:
    Palworld::PalMainLoader MainLoader;
};


#define PALSCHEMA_API UE4SS_MOD_API
extern "C"
{
    PALSCHEMA_API RC::CppUserModBase* start_mod()
    {
        auto* mod = new PalSchema();
        return mod;
    }

    PALSCHEMA_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
