#pragma once
#include "CoreMinimal.h"
#include "InputCoreTypes.h"

struct FFiskAssistIntent
{
    bool bRaise=false, bReel=false;
    int32 DragStep=0;
};

// Uses only the same visible surge/load/drag readings as a human player.
class FFiskTestPilot
{
public:
    FFiskAssistIntent Step(double Delta, bool bSurging, int32 Load, int32 Drag)
    {
        FFiskAssistIntent Intent;
        const bool Rest=bSurging || Load>85;
        const int32 Target=Rest?40:70;
        Intent.DragStep=Drag<Target?1:Drag>Target?-1:0;
        if(Rest) { Phase=0; return Intent; }
        Phase=FMath::Fmod(Phase+FMath::Clamp(Delta,0.0,.25),3.0);
        Intent.bRaise=Phase<1.5;
        Intent.bReel=!Intent.bRaise;
        return Intent;
    }
private:
    double Phase=0;
};

namespace FiskQuickControls
{
    inline bool IsAssistKey(const FKey& Key)
    {
        const FString Name=Key.ToString();
        if(Name==TEXT("Å") || Name==TEXT("å")) { return true; }
        const uint32 *Virtual=nullptr,*Character=nullptr;
        FInputKeyManager::Get().GetCodesFromKey(Key,Virtual,Character);
        return Character && (*Character==0x00c5 || *Character==0x00e5);
    }
    inline FBox2D ReturnBox(float Scale) { return FBox2D(FVector2D(900,166)*Scale,FVector2D(1260,228)*Scale); }
    inline FBox2D AssistBox(float Scale) { return FBox2D(FVector2D(900,242)*Scale,FVector2D(1260,308)*Scale); }
    inline bool CanReturn(bool Ready,bool Aboard,bool Captain,bool Day,bool Revision,bool Leg,int32 Progress)
    { return Ready && Aboard && Captain && Day && Revision && Leg && Progress>=0 && Progress<=100; }
}
