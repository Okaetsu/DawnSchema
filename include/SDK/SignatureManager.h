#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>

namespace SDK {
    class SignatureManager {
    public:
        static void Initialize();
        
        // Expected parameter format: [CLASS]::[FUNCTION] or [FUNCTION], for example AGameModeBase::InitGameState or AsyncTask
        static void* GetSignature(const std::string& ClassAndFunction);
    private:
        static inline std::unordered_map<std::string, void*> SignatureMap;

        static inline std::unordered_map<std::string, std::string> Signatures {
        };
        static inline std::unordered_map<std::string, std::string> SignaturesCallResolve {
        };
    };
}