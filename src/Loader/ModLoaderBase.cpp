#include "Loader/ModLoaderBase.h"
#include "Unreal/Engine/UDataTable.hpp"
#include "Utility/JsonHelpers.h"
#include "Utility/Logging.h"
#include "UE4SSProgram.hpp"

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace Schema {
	ModLoaderBase::ModLoaderBase(const std::string& modFolderType) : m_modFolderType(modFolderType) {}

	ModLoaderBase::~ModLoaderBase() {
    }

    const RC::StringType& ModLoaderBase::GetDisplayName() const
    {
        return m_displayName;
    }

    void ModLoaderBase::Setup()
    {
        OnSetup();
    }

    void ModLoaderBase::AutoReload(const std::filesystem::path::string_type& modName, const std::filesystem::path& modFilePath)
    {
        OnAutoReload(modName, modFilePath);
    }

    void ModLoaderBase::Load(const fs::path& modPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase)
    {
        // Loaders should be initialized by this point. Load was called before Initialize or something went wrong during initialization.
        if (!HasInitialized())
        {
            return;
        }

        auto loaderPath = modPath / m_modFolderType;
        if (!fs::is_directory(loaderPath))
        {
            return;
        }

        OnLoad(loaderPath, modName, engineLifecyclePhase);
    }

	void ModLoaderBase::Initialize(const EEngineLifecyclePhase& engineLifecyclePhase) {
        if (!CanInitialize(engineLifecyclePhase))
        {
            return;
        }

        Initialize_Internal();

        if (!HasInitialized())
        {
            return;
        }

        PostInitialize();
    }

    const bool& ModLoaderBase::HasInitialized() const
    {
        return m_hasInitialized;
    }

    const std::string& ModLoaderBase::GetModFolderType()
    {
        return m_modFolderType;
    }

    void ModLoaderBase::SetDisplayName(const RC::StringType& displayName)
    {
        m_displayName = displayName;
    }

    void ModLoaderBase::IterateModsFolder(const std::function<void(const std::filesystem::path&, const RC::StringType&)>& callback)
    {
        static auto modsPath = fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "DawnSchema" / "mods";
        if (fs::exists(modsPath))
        {
            for (const auto& entry : fs::directory_iterator(modsPath)) {
                if (entry.is_directory())
                {
                    auto& path = entry.path();
                    auto folderName = path.stem().native();
                    callback(entry.path(), folderName);
                }
            }
        }
    }

    void ModLoaderBase::OnSetup() {}

    void ModLoaderBase::OnLoad(const std::filesystem::path& loaderPath, const RC::StringType& modName, const EEngineLifecyclePhase& engineLifecyclePhase) {}

    void ModLoaderBase::OnAutoReload(const std::filesystem::path::string_type& modName, const std::filesystem::path& modFilePath) {}

    void ModLoaderBase::PostInitialize() {}

    void ModLoaderBase::Initialize_Internal()
    {
        if (HasInitialized())
        {
            // TODO: Debugging purposes only, remove this print when done testing.
            PS::Log<LogLevel::Warning>(STR("Loader '{}' attempted to initialize more than once. Skipping.\n"), RC::to_generic_string(m_modFolderType));
            return;
        }

        if (!OnInitialize())
        {
            PS::Log<LogLevel::Error>(STR("Failed to initialize '{}' loader.\n"), RC::to_generic_string(m_modFolderType));
            return;
        }

        m_hasInitialized = true;

        PS::Log<LogLevel::Normal>(STR("Loader '{}' initialized.\n"), RC::to_generic_string(m_modFolderType));
    }
}