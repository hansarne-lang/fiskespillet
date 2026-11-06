#include "UI/FiskAnglerPanel.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskUserSettings.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "FiskCareerProfile.h"
#include "DayCycle/FiskDayState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/HUD.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Misc/Paths.h"
UFiskAnglerPanel::UFiskAnglerPanel() { PrimaryComponentTick.bCanEverTick=true;PreviousStates.Init(0,3); }
FString UFiskAnglerPanel::File() const
{ return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Angler-")+LoadedProfile.ToString(EGuidFormats::Digits)+TEXT(".ini")); }
void UFiskAnglerPanel::Load()
{
    const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());if(LoadedProfile==GI->Career.Id) { return; }LoadedProfile=GI->Career.Id;Data.Load(File(),LoadedProfile);
    Message=Data.bBlocked?TEXT("Lagringsfilen er bevart og blokkert. Du kan fortsatt øve uten å lagre."):Data.bRecovered?TEXT("Stangsett og øving hentet fra backup. Neste uttrykkelige lagring reparerer hovedfilen."):TEXT("");
}
bool UFiskAnglerPanel::OpenSchool()
{
    const auto* D=GetWorld()->GetGameState<AFiskDayState>();if(GetNetMode()!=NM_Standalone || (D && D->HasStarted())) { return false; }
    Load();Page=2;bControls=true;Lesson=FFiskControlLesson();Practice=FFiskPracticeSession();CastChecked<AFiskBoardingController>(GetOwner())->ReleaseRodInputsForMenu();return true;
}
void UFiskAnglerPanel::StartPractice()
{
    if(!Practice.Start(Species,Gear,1000+Species*31+Gear+Data.Scores[Species*2+(Gear==7?1:0)].Attempts)) { return; }
    auto Candidate=Data;const int32 Index=Species*2+(Gear==7?1:0);
    if(Candidate.Scores[Index].Begin(Practice.Attempt))
    { const bool Saved=Candidate.Save(File(),LoadedProfile);Data=Candidate;Message=Saved?TEXT("Trening startet. Ingen penger, fangster eller karriererekorder endres."):TEXT("Trening startet uten lagringsbekreftelse. Tidligere filer er bevart."); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PRACTICE_STARTED species=%d equipment=%d"),Species,Gear);
}
void UFiskAnglerPanel::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* F)
{
    Super::TickComponent(Dt,Type,F);const auto* PC=Cast<AFiskBoardingController>(GetOwner());if(!PC || !PC->IsLocalController()) { return; }
    if(Page==2 && !bControls) { Practice.Advance(Dt); }
    const auto* D=GetWorld()->GetGameState<AFiskDayState>();const auto* B=PC->FindStarterBoat();if(!D || !D->HasStarted() || !B || !PC->PlayerState || !B->IsAboard(PC->PlayerState->GetPlayerId())) { return; }
    for(int32 I=0;I<PC->GetRodCapacity();++I)
    {
        const int32 State=static_cast<int32>(PC->GetRodAt(I)->GetState());if(State==PreviousStates[I]) { continue; }PreviousStates[I]=State;
        if(const TCHAR* Text=FiskAngler::RodAlert(State)) { Notice=FString::Printf(TEXT("STANG %d: %s | %d velger stangen"),I+1,Text,I+1);NoticeUntil=GetWorld()->GetRealTimeSeconds()+6;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_ROD_NOTICE slot=%d state=%d"),I,State); }
    }
}
bool UFiskAnglerPanel::HandleKey(const FInputKeyEventArgs& A)
{
    const bool Press=A.Event==IE_Pressed,Repeat=A.Event==IE_Repeat;const FKey K=A.Key;auto* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if(Page==0)
    { if(Press && K==EKeys::Q) { Load();Page=1;bRenaming=false;PC->ReleaseRodInputsForMenu();return true; }return false; }
    if(bRenaming)
    {
        if(!Press && !Repeat) { return true; }if(K==EKeys::Escape) { bRenaming=false;return true; }
        if(K==EKeys::Enter)
        {
            auto Candidate=Data;if(FFiskCareerProfile::ValidName(EditName)) { Candidate.Loadouts[Slot].Name=EditName;if(Candidate.Save(File(),LoadedProfile)) { Data=Candidate;bRenaming=false;Message=TEXT("Navn lagret."); }else { Message=TEXT("Lagring avvist; filene er bevart."); } }else { Message=TEXT("Bruk 1–24 bokstaver, tall eller mellomrom, uten mellomrom først/sist."); }return true;
        }
        if(K==EKeys::BackSpace) { EditName.LeftChopInline(1); }else if(K==EKeys::SpaceBar && EditName.Len()<24) { EditName+=TEXT(" "); }
        else { FString V=K.ToString();const FKey Digits[]={EKeys::Zero,EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};for(int32 I=0;I<10;++I) { if(K==Digits[I]) { V=FString::FromInt(I); } }if(V.Len()==1 && FChar::IsAlnum(V[0]) && EditName.Len()<24) { EditName+=V.ToUpper(); } }return true;
    }
    if(Press && (K==EKeys::Escape || (Page==1 && K==EKeys::Q) || (Page==2 && K==EKeys::F6)))
    { Page=0;Practice=FFiskPracticeSession();return true; }
    if(Page==2)
    {
        if(Press && K==EKeys::Tab) { bControls=!bControls;Practice=FFiskPracticeSession();return true; }
        if((Press || Repeat) && !Practice.IsRunning())
        {
            if(K==EKeys::Left) { Species=(Species+6)%7;Practice=FFiskPracticeSession(); }if(K==EKeys::Right) { Species=(Species+1)%7;Practice=FFiskPracticeSession(); }
            if(K==EKeys::G) { Gear=Gear==0?7:0;Practice=FFiskPracticeSession(); }
        }
        if(Press && K==EKeys::Enter) { if(bControls) { Lesson=FFiskControlLesson(); }else { StartPractice(); } }
        auto Act=[&](EFiskLessonAction V){if(bControls && Lesson.Act(V) && Lesson.Complete()) { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_LESSON_PASSED")); }};
        if(K==EKeys::LeftMouseButton) { if(Press) { Act(EFiskLessonAction::Lift);Practice.Fight.SetRaised(true); }else if(A.Event==IE_Released) { Act(EFiskLessonAction::Lower);Practice.Fight.SetRaised(false); } }
        if(K==EKeys::RightMouseButton) { if(Press) { Act(EFiskLessonAction::Reel);Practice.Fight.SetReeling(true); }else if(A.Event==IE_Released) { Act(EFiskLessonAction::StopReel);Practice.Fight.SetReeling(false); } }
        if(Press && K==EKeys::MouseScrollUp) { Act(EFiskLessonAction::Tighten);Practice.Fight.AdjustDrag(1); }if(Press && K==EKeys::MouseScrollDown) { Act(EFiskLessonAction::Loosen);Practice.Fight.AdjustDrag(-1); }
        if(Press && K==EKeys::SpaceBar)
        {
            Act(EFiskLessonAction::Land);
            if(!bControls && Practice.Land())
            {
                auto Candidate=Data;const int32 Index=Practice.Species*2+(Practice.Equipment==7?1:0);const bool Counted=Candidate.Scores[Index].Complete(Practice);const bool Saved=Counted && Candidate.Save(File(),LoadedProfile);if(Counted) { Data=Candidate; }
                Message=Saved?TEXT("Øvingsresultat lagret separat. Ingen karrierebelønning."):TEXT("Øving fullført. Resultatlagring mangler; eksisterende filer er bevart.");
                UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PRACTICE_COMPLETED species=%d equipment=%d ms=%d saved=%d"),Practice.Species,Practice.Equipment,FMath::CeilToInt(Practice.Seconds*1000),Saved);
            }
        }return true;
    }
    if(!Press && !Repeat) { return true; }
    if(K==EKeys::Left) { Species=(Species+6)%7; }if(K==EKeys::Right) { Species=(Species+1)%7; }
    if(Press && K==EKeys::G) { Bait=(Bait+1)%3; }if(Press && K==EKeys::Tab) { Slot=(Slot+1)%3; }
    if(Press && (K==EKeys::One || K==EKeys::Two || K==EKeys::Three)) { PC->RequestRodSelection(K==EKeys::One?0:K==EKeys::Two?1:2); }
    const auto* B=PC->FindStarterBoat();const auto Advice=FiskAngler::Advice(B?B->GetGroundId():0,Bait);
    if(Press && K==EKeys::Enter) { PC->RequestRig(Advice.Bait,Advice.Depth,Advice.Program); }
    if(Press && K==EKeys::F4) { TArray<FFiskRigSettings> Rigs;Rigs.Init(Advice,PC->GetRodCapacity());PC->ServerApplyRigBundle(PC->MakeRigBundle(Rigs)); }
    if(Press && K==EKeys::F2) { bRenaming=true;EditName=Data.Loadouts[Slot].Name; }
    if(Press && K==EKeys::F5)
    {
        auto Candidate=Data;auto& Rigs=Candidate.Loadouts[Slot].Rigs;Rigs.Reset();for(int32 I=0;I<PC->GetRodCapacity();++I) { auto R=PC->GetRodAt(I)->GetRig();R.Revision=0;Rigs.Add(R); }
        if(!Rigs.IsEmpty() && Candidate.Save(File(),LoadedProfile)) { Data=Candidate;Message=TEXT("Hele ditt stangsett er lagret."); }else { Message=TEXT("Stangsettet kunne ikke lagres. Kontroller egne holdere og lagringsfil."); }
    }
    if(Press && K==EKeys::F8) { PC->ServerApplyRigBundle(PC->MakeRigBundle(Data.Loadouts[Slot].Rigs)); }return true;
}
void UFiskAnglerPanel::Draw(AHUD& H,UCanvas& C) { if(Page==2) { DrawSchool(H,C); }else if(Page==1) { DrawWorkbench(H,C); } }
void UFiskAnglerPanel::DrawNotice(AHUD& H,UCanvas& C) const
{
    if(Notice.IsEmpty() || GetWorld()->GetRealTimeSeconds()>=NoticeUntil) { return; }const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);
    H.DrawRect(FLinearColor(.025f,.1f,.13f,.98f),385*S,88*S,490*S,62*S);H.DrawText(Notice,FLinearColor(1,.86f,.45f),400*S,109*S,nullptr,.95f*S);
}
