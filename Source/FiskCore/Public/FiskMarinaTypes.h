#pragma once
#include "CoreMinimal.h"
#include "FiskCareerProfile.h"
#include "FiskMarinaTypes.generated.h"

namespace FiskMarina
{
    inline constexpr int32 SpeciesCount=15;
    inline const TCHAR* SpeciesName(int32 Species)
    { static const TCHAR* Names[]={TEXT("Snapper"),TEXT("Amberjack"),TEXT("Gulfinnet tunfisk"),TEXT("Grouper"),TEXT("Mahi-mahi"),TEXT("Wahoo"),TEXT("Blå marlin"),TEXT("Barrakuda"),TEXT("Tarpon"),TEXT("Permit"),TEXT("Snook"),TEXT("Storøyet tunfisk"),TEXT("Seilfisk"),TEXT("Sverdfisk"),TEXT("Escolar")};return Species>=0 && Species<SpeciesCount?Names[Species]:TEXT("Ukjent"); }
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
    UPROPERTY() int32 Lighting=0;
    UPROPERTY() int32 Pier=0;
    UPROPERTY() int32 Display=0;
    static int32 PremiumItem(int32 Category,int32 Level) { return Category>=0 && Category<3 && Level>=1 && Level<=3?4+Category*3+Level-1:-1; }
    bool IsOwnedBy(const FFiskCareerProfile& P) const { return IsValid() && (Lighting==0 || P.Has(PremiumItem(0,Lighting))) && (Pier==0 || P.Has(PremiumItem(1,Pier))) && (Display==0 || P.Has(PremiumItem(2,Display))); }
    bool IsValid() const { return FFiskCareerProfile::ValidName(DisplayName) && FFiskCareerProfile::ValidName(HouseName) && Color>=0 && Color<6 && Style>=0 && Style<3 && Decor>=0 && Decor<3 && Lighting>=0 && Lighting<=3 && Pier>=0 && Pier<=3 && Display>=0 && Display<=3; }
    bool operator==(const FFiskMarinaIdentity& O) const { return DisplayName==O.DisplayName && HouseName==O.HouseName && Color==O.Color && Style==O.Style && Decor==O.Decor && Lighting==O.Lighting && Pier==O.Pier && Display==O.Display; }
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
    UPROPERTY() int32 Place=0; // 0 marina, 1 naust, 2 bar, 3 at sea (server-derived)
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
