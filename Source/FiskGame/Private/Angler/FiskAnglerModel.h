#pragma once
#include "CoreMinimal.h"
#include "Fishing/FiskRigSettings.h"
#include "Fishing/FiskFightModel.h"
#include "Fishing/FiskCatchCatalog.h"
#include "FiskAnglerModel.generated.h"
USTRUCT()
struct FFiskRigBundle
{
    GENERATED_BODY()
    UPROPERTY() int32 Day=0;
    UPROPERTY() TArray<FGuid> Casts;
    UPROPERTY() TArray<int32> Revisions;
    UPROPERTY() TArray<FFiskRigSettings> Rigs;
};
namespace FiskAngler
{
    inline constexpr int32 ScoreCount=FiskCatches::Count*2;
    FFiskRigSettings Advice(int32 Ground,int32 Bait);
    int32 BestHabitat(int32 Species);
    bool Prepare(const FFiskRigBundle& Request,int32 Day,int32 Capacity,int32 Bottom,const TArray<FGuid>& Casts,const TArray<FFiskRigSettings>& Current,const TArray<bool>& Safe,TArray<FFiskRigSettings>& Result);
    const TCHAR* RodAlert(int32 State);
}
enum class EFiskLessonAction : uint8 { Lift,Lower,Reel,StopReel,Tighten,Loosen,Land };
struct FFiskControlLesson
{
    int32 Step=0;
    bool Act(EFiskLessonAction Action);
    bool Complete() const { return Step==7; }
};
class FFiskPracticeSession
{
public:
    bool Start(int32 Species,int32 Equipment,int32 Seed);
    void Advance(double Seconds);
    bool Land();
    bool IsRunning() const { return bStarted && !bLanded && Fight.GetOutcome()==EFiskFightOutcome::Fighting; }
    bool HasStarted() const { return bStarted; }
    bool IsLanded() const { return bLanded; }
    FFiskFightModel Fight;
    double Seconds=0,BadLoadSeconds=0,SurgeReelSeconds=0;
    int32 Species=0,Equipment=0;
    FGuid Attempt;
private:
    bool bStarted=false,bLanded=false;
};
struct FFiskPracticeScore
{
    int32 Attempts=0,Completed=0,BestMs=0;
    FGuid LastCompleted,ActiveAttempt;
    bool Valid() const;
    bool Begin(const FGuid& Attempt);
    bool Complete(const FFiskPracticeSession& Session);
};
struct FFiskRodLoadout { FString Name;TArray<FFiskRigSettings> Rigs; };
struct FFiskAnglerData
{
    TArray<FFiskRodLoadout> Loadouts;
    TArray<FFiskPracticeScore> Scores;
    bool bBlocked=false,bRecovered=false;
    FFiskAnglerData();
    bool Valid() const;
    bool Load(const FString& File,const FGuid& Profile);
    bool Save(const FString& File,const FGuid& Profile) const;
};
