#pragma once
#include "CoreMinimal.h"
#include "FiskRigSettings.generated.h"
struct FFiskFishingGround;

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
    static bool IsValid(int32 Bait, int32 Depth, int32 Program, int32 BottomDepth = 60);
    static bool TryApply(FFiskRigSettings& Rig, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program, int32 BottomDepth = 60);
    static int32 Quality(const FFiskRigSettings& Rig, const FFiskFishingGround* Ground = nullptr);
    static double BiteDelay(const FFiskRigSettings& Rig, FRandomStream& Random, const FFiskFishingGround* Ground = nullptr);
    static const TCHAR* BaitName(int32 Bait);
    static const TCHAR* ProgramName(int32 Program);
    static const TCHAR* QualityName(const FFiskRigSettings& Rig, const FFiskFishingGround* Ground = nullptr);
};
