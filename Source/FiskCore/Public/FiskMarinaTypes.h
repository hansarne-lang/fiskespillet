#pragma once
#include "CoreMinimal.h"
#include "FiskCareerProfile.h"
#include "FiskMarinaTypes.generated.h"

namespace FiskMarina
{
    inline constexpr int32 SpeciesCount=7;
    inline const TCHAR* SpeciesName(int32 Species)
    { static const TCHAR* Names[]={TEXT("Snapper"),TEXT("Amberjack"),TEXT("Gulfinnet tunfisk"),TEXT("Grouper"),TEXT("Mahi-mahi"),TEXT("Wahoo"),TEXT("Blå marlin")};return Species>=0 && Species<SpeciesCount?Names[Species]:TEXT("Ukjent"); }
}

USTRUCT()
struct FISKCORE_API FFiskMarinaIdentity
{
    GENERATED_BODY()
    UPROPERTY() FString DisplayName=TEXT("FISKER");
    UPROPERTY() FString HouseName=TEXT("MITT NAUST");
    UPROPERTY() int32 Color=0;
    UPROPERTY() int32 Style=0;
    UPROPERTY() int32 Decor=0;
    bool IsValid() const { return FFiskCareerProfile::ValidName(DisplayName) && FFiskCareerProfile::ValidName(HouseName) && Color>=0 && Color<6 && Style>=0 && Style<3 && Decor>=0 && Decor<3; }
    bool operator==(const FFiskMarinaIdentity& O) const { return DisplayName==O.DisplayName && HouseName==O.HouseName && Color==O.Color && Style==O.Style && Decor==O.Decor; }
};
USTRUCT()
struct FISKCORE_API FFiskMarinaTrophy
{
    GENERATED_BODY()
    UPROPERTY() int32 Species=0;
    UPROPERTY() int32 Grams=0;
    UPROPERTY() int32 LengthMm=0;
    UPROPERTY() FGuid DayId;
    bool operator==(const FFiskMarinaTrophy& O) const { return Species==O.Species && Grams==O.Grams && LengthMm==O.LengthMm && DayId==O.DayId; }
};
USTRUCT()
struct FISKCORE_API FFiskMarinaResident
{
    GENERATED_BODY()
    UPROPERTY() FGuid Profile;
    UPROPERTY() int32 PlayerId=-1;
    UPROPERTY() FFiskMarinaIdentity Identity;
    UPROPERTY() TArray<FFiskMarinaTrophy> Trophies;
};
USTRUCT()
struct FISKCORE_API FFiskMarinaMessage
{
    GENERATED_BODY()
    UPROPERTY() int32 Serial=0;
    UPROPERTY() FString Name;
    UPROPERTY() int32 Code=0;
    UPROPERTY() int32 AtMs=0;
};
