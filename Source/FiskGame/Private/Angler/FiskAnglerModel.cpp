#include "Angler/FiskAnglerModel.h"
#include "FiskFishingGround.h"
#include "Fishing/FiskCatchCatalog.h"
#include "FiskCareerProfile.h"
FFiskRigSettings FiskAngler::Advice(int32 Ground,int32 Bait)
{
    FFiskRigSettings R;const auto* G=FiskGrounds::Find(Ground);if(!G || Bait<0 || Bait>2) { return R; }
    R.Bait=R.Program=Bait;R.Depth=FMath::Clamp(((G->FishDepthMin+G->FishDepthMax)/20)*10,10,FMath::Min(120,G->BottomDepth));return R;
}
int32 FiskAngler::BestHabitat(int32 Species)
{ if(!FiskCatches::Find(Species)) { return -1; }int32 Best=0;for(int32 G=1;G<FiskGrounds::Count;++G) { if(FiskCatches::HabitatChance(G,Species)>FiskCatches::HabitatChance(Best,Species)) { Best=G; } }return Best; }
bool FiskAngler::Prepare(const FFiskRigBundle& R,int32 Day,int32 Count,int32 Bottom,const TArray<FGuid>& Casts,const TArray<FFiskRigSettings>& Current,const TArray<bool>& Safe,TArray<FFiskRigSettings>& Out)
{
    if(Day<1 || R.Day!=Day || Count<1 || Count>3 || R.Rigs.Num()!=Count || R.Casts.Num()!=Count || R.Revisions.Num()!=Count || Casts.Num()!=Count || Current.Num()!=Count || Safe.Num()!=Count) { return false; }
    TArray<FFiskRigSettings> Candidate=Current;
    for(int32 I=0;I<Count;++I)
    { if(!Safe[I] || R.Casts[I]!=Casts[I] || !FFiskRigRules::TryApply(Candidate[I],R.Revisions[I],R.Rigs[I].Bait,R.Rigs[I].Depth,R.Rigs[I].Program,Bottom)) { return false; } }
    Out=MoveTemp(Candidate);return true;
}
const TCHAR* FiskAngler::RodAlert(int32 S)
{ return S==2?TEXT("NAPP – første utras"):S==4?TEXT("Klar til landing – mellomrom"):S==6?TEXT("Fisken slapp unna"):nullptr; }
bool FFiskControlLesson::Act(EFiskLessonAction A)
{ if(Step<7 && static_cast<int32>(A)==Step) { ++Step;return true; }return false; }
bool FFiskPracticeSession::Start(int32 S,int32 Gear,int32 Seed)
{
    if(!FiskCatches::Find(S) || (Gear!=0 && Gear!=7)) { return false; }
    *this=FFiskPracticeSession();Species=S;Equipment=Gear;Attempt=FGuid::NewGuid();bStarted=true;Fight.Reset(Seed,30,S,Gear);return true;
}
void FFiskPracticeSession::Advance(double Dt)
{
    if(!IsRunning() || !FMath::IsFinite(Dt) || Dt<=0) { return; }Dt=FMath::Min(Dt,.25);Seconds+=Dt;Fight.Advance(Dt);
    if(Fight.GetLoadPercent()>static_cast<int32>(FFiskFightModel::OverloadLoad(Equipment,FFiskCareerProfile::ItemLine)) || Fight.GetLoadPercent()<20) { BadLoadSeconds+=Dt; }
    if(Fight.IsSurging() && Fight.IsReeling()) { SurgeReelSeconds+=Dt; }
}
bool FFiskPracticeSession::Land()
{ if(!bStarted || bLanded || Fight.GetOutcome()!=EFiskFightOutcome::ReadyToLand) { return false; }bLanded=true;return true; }
bool FFiskPracticeScore::Valid() const
{ return Attempts>=0 && Attempts<=1000000 && Completed>=0 && Completed<=Attempts && ((Completed==0 && BestMs==0 && !LastCompleted.IsValid()) || (Completed>0 && BestMs>=1 && BestMs<=3600000 && LastCompleted.IsValid())); }
bool FFiskPracticeScore::Begin(const FGuid& Id) { if(!Valid() || Attempts==1000000 || !Id.IsValid() || Id==LastCompleted || Id==ActiveAttempt) { return false; }++Attempts;ActiveAttempt=Id;return true; }
bool FFiskPracticeScore::Complete(const FFiskPracticeSession& S)
{
    const int32 Ms=FMath::CeilToInt(S.Seconds*1000);if(!Valid() || !S.IsLanded() || !S.Attempt.IsValid() || S.Attempt!=ActiveAttempt || S.Attempt==LastCompleted || Completed>=Attempts || Ms<1 || Ms>3600000) { return false; }
    ++Completed;BestMs=BestMs==0?Ms:FMath::Min(BestMs,Ms);LastCompleted=S.Attempt;ActiveAttempt.Invalidate();return true;
}
FFiskAnglerData::FFiskAnglerData()
{ Loadouts={{TEXT("STANGSETT 1"),{}},{TEXT("STANGSETT 2"),{}},{TEXT("STANGSETT 3"),{}}};Scores.SetNum(FiskAngler::ScoreCount); }
bool FFiskAnglerData::Valid() const
{
    if(Loadouts.Num()!=3 || Scores.Num()!=FiskAngler::ScoreCount) { return false; }
    for(const auto& L:Loadouts) { if(!FFiskCareerProfile::ValidName(L.Name) || L.Rigs.Num()>3) { return false; }for(const auto& R:L.Rigs) { if(!FFiskRigRules::IsValid(R.Bait,R.Depth,R.Program,250)) { return false; } } }
    for(const auto& S:Scores) { if(!S.Valid()) { return false; } }return true;
}
