#pragma once
#include "CoreMinimal.h"
#include "FiskRigSettings.generated.h"

// Only the rod owner's selected equipment. No catch or hidden fish attributes.
USTRUCT()
struct FFiskRigSettings
{
    GENERATED_BODY()
    UPROPERTY() int32 Bait = 0;
    UPROPERTY() int32 Depth = 30;
    UPROPERTY() int32 Program = 0;
    UPROPERTY() int32 Revision = 0;
};

class FFiskRigRules
{
public:
    static bool IsValid(int32 Bait, int32 Depth, int32 Program);
    static bool TryApply(FFiskRigSettings& Rig, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program);
    static int32 Quality(const FFiskRigSettings& Rig);
    static double BiteDelay(const FFiskRigSettings& Rig, FRandomStream& Random);
    static const TCHAR* BaitName(int32 Bait);
    static const TCHAR* ProgramName(int32 Program);
    static const TCHAR* QualityName(const FFiskRigSettings& Rig);
};
