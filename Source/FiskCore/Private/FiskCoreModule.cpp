#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFiskCore, Log, All);

class FFiskCoreModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UE_LOG(LogFiskCore, Display, TEXT("FISKCORE_MODULE_LOADED"));
    }
};

IMPLEMENT_MODULE(FFiskCoreModule, FiskCore);
