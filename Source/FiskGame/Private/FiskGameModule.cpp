#include "Modules/ModuleManager.h"

class FFiskGameModule final : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        FModuleManager::LoadModuleChecked<IModuleInterface>(TEXT("FiskCore"));
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FFiskGameModule, FiskGame, "FISKELAGET");
