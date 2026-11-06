#include "Angler/FiskAnglerModel.h"
#include "FiskFishingGround.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
namespace
{
bool AnglerTestFinish(FFiskPracticeSession& S,double Dt=.1)
{
    for(int32 I=0;I<10000 && S.IsRunning();++I) { const bool Lift=!S.Fight.IsSurging() && static_cast<int32>(S.Seconds/2)%2==0;S.Fight.SetRaised(Lift);S.Fight.SetReeling(!S.Fight.IsSurging() && !Lift);S.Advance(Dt); }
    return S.Land();
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskAdviceTest,"Fiskelaget.Angler.CatalogAdviceWithoutCatchAccess",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskAdviceTest::RunTest(const FString& Parameters)
{
    for(int32 G=0;G<FiskGrounds::Count;++G) { for(int32 B=0;B<3;++B) { const auto R=FiskAngler::Advice(G,B);TestTrue(TEXT("Valid local recommendation"),FFiskRigRules::IsValid(R.Bait,R.Depth,R.Program,FiskGrounds::Find(G)->BottomDepth));TestEqual(TEXT("Uses existing good-quality rule"),FFiskRigRules::Quality(R,FiskGrounds::Find(G)),2); } }
    for(int32 S=0;S<FiskCatches::Count;++S) { const int32 Best=FiskAngler::BestHabitat(S);TestTrue(TEXT("Known habitat only"),Best>=0 && Best<FiskGrounds::Count);for(int32 G=0;G<FiskGrounds::Count;++G) { TestTrue(TEXT("Highest existing catalog weight"),FiskCatches::HabitatChance(Best,S)>=FiskCatches::HabitatChance(G,S)); } }
    TestEqual(TEXT("No invented species"),FiskAngler::BestHabitat(FiskCatches::Count),-1);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBundleTest,"Fiskelaget.Angler.AtomicRigBundleFreshnessAndBounds",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskBundleTest::RunTest(const FString& Parameters)
{
    TArray<FFiskRigSettings> Current;Current.SetNum(3);TArray<FGuid> Casts={FGuid::NewGuid(),FGuid::NewGuid(),FGuid::NewGuid()};TArray<bool> Safe={true,true,true};FFiskRigBundle R;R.Day=2;R.Casts=Casts;R.Revisions={0,0,0};R.Rigs=Current;R.Rigs[0].Bait=R.Rigs[0].Program=1;
    TArray<FFiskRigSettings> Out;TestTrue(TEXT("Valid complete bundle"),FiskAngler::Prepare(R,2,3,120,Casts,Current,Safe,Out));TestEqual(TEXT("Revision increment"),Out[0].Revision,1);TestEqual(TEXT("Own first rig"),Out[0].Bait,1);
    const auto Original=R;R.Rigs[2].Depth=125;TestFalse(TEXT("Invalid last rod rejects whole bundle"),FiskAngler::Prepare(R,2,3,120,Casts,Current,Safe,Out));TestEqual(TEXT("Output not partially overwritten"),Out[0].Revision,1);TestEqual(TEXT("Current first rig unchanged"),Current[0].Bait,0);
    R=Original;TestFalse(TEXT("Old day rejected"),FiskAngler::Prepare(R,3,3,120,Casts,Current,Safe,Out));R.Casts[0]=FGuid::NewGuid();TestFalse(TEXT("Foreign/stale cast"),FiskAngler::Prepare(R,2,3,120,Casts,Current,Safe,Out));R=Original;R.Revisions[2]=4;TestFalse(TEXT("Stale revision"),FiskAngler::Prepare(R,2,3,120,Casts,Current,Safe,Out));R=Original;Safe[1]=false;TestFalse(TEXT("Any active rod blocks all"),FiskAngler::Prepare(R,2,3,120,Casts,Current,Safe,Out));TestFalse(TEXT("Changed holder count"),FiskAngler::Prepare(R,2,2,120,Casts,Current,Safe,Out));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskLessonTest,"Fiskelaget.Angler.OrderedControlLesson",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskLessonTest::RunTest(const FString& Parameters)
{
    FFiskControlLesson L;TestFalse(TEXT("Cannot skip to landing"),L.Act(EFiskLessonAction::Land));for(int32 I=0;I<7;++I) { TestTrue(TEXT("Expected action advances"),L.Act(static_cast<EFiskLessonAction>(I))); }TestTrue(TEXT("Seven controls complete"),L.Complete());TestFalse(TEXT("Repeat does not add progress"),L.Act(EFiskLessonAction::Land));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPracticeTest,"Fiskelaget.Angler.SameFightModelAllSpecies",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPracticeTest::RunTest(const FString& Parameters)
{
    for(int32 I=0;I<FiskCatches::Count;++I) { FFiskPracticeSession P;TestTrue(TEXT("Start known species"),P.Start(I,7,2048+I));TestFalse(TEXT("Cannot land at start"),P.Land());TestTrue(TEXT("Actual model reaches landing"),AnglerTestFinish(P));TestTrue(TEXT("Finite elapsed practice time"),P.Seconds>1 && P.Seconds<1000);TestFalse(TEXT("Landing counted only once"),P.Land()); }
    FFiskPracticeSession P;TestFalse(TEXT("Invalid species rejected"),P.Start(FiskCatches::Count,0,1));TestFalse(TEXT("Only known virtual gear options"),P.Start(0,99,1));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPracticeClockTest,"Fiskelaget.Angler.PracticeClockAndFailure",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPracticeClockTest::RunTest(const FString& Parameters)
{
    FFiskPracticeSession A,B;A.Start(0,0,1);B.Start(0,0,1);A.Advance(-1);TestEqual(TEXT("Negative delta ignored"),A.Seconds,0.0);A.Advance(10);TestEqual(TEXT("A long frame cannot accelerate ten seconds"),A.Seconds,.25);
    A=FFiskPracticeSession();A.Start(0,0,1);for(int32 I=0;I<1000 && A.IsRunning();++I) { A.Advance(.1); }TestFalse(TEXT("Idle practice can fail"),A.IsLanded());TestTrue(TEXT("Failure uses normal outcomes"),A.Fight.GetOutcome()==EFiskFightOutcome::Slack || A.Fight.GetOutcome()==EFiskFightOutcome::LineBreak);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPracticeScoreTest,"Fiskelaget.Angler.PracticeCompletionBinding",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPracticeScoreTest::RunTest(const FString& Parameters)
{
    FFiskPracticeScore Score;FFiskPracticeSession A;A.Start(0,7,11);TestTrue(TEXT("Begin binds attempt"),Score.Begin(A.Attempt));TestFalse(TEXT("Cannot count before landing"),Score.Complete(A));TestTrue(TEXT("Finish actual fight"),AnglerTestFinish(A));TestTrue(TEXT("Record first completion"),Score.Complete(A));TestFalse(TEXT("No duplicate completion"),Score.Complete(A));FFiskPracticeSession B;B.Start(0,7,12);TestTrue(TEXT("New attempt"),Score.Begin(B.Attempt));TestFalse(TEXT("Old completion cannot consume new attempt"),Score.Complete(A));TestTrue(TEXT("Scores remain bounded"),Score.Valid());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskAnglerDiskTest,"Fiskelaget.Angler.ProfileStoreBackupAndPreservation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskAnglerDiskTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits)),File=FPaths::Combine(Dir,TEXT("angler.ini"));const FGuid Id=FGuid::NewGuid();FFiskAnglerData D;
    TestTrue(TEXT("Empty old career gets defaults"),D.Load(File,Id));D.Loadouts[0].Rigs={FiskAngler::Advice(0,0),FiskAngler::Advice(0,1)};D.Loadouts[0].Name=TEXT("FISKER 12");TestTrue(TEXT("Save initial data"),D.Save(File,Id));D.Loadouts[0].Name=TEXT("SECOND");TestTrue(TEXT("Save new generation"),D.Save(File,Id));FFiskAnglerData E;TestTrue(TEXT("Reload"),E.Load(File,Id));TestEqual(TEXT("Full rig count"),E.Loadouts[0].Rigs.Num(),2);
    TestFalse(TEXT("Wrong profile cannot load"),E.Load(File,FGuid::NewGuid()));TestFalse(TEXT("Wrong profile cannot replace"),D.Save(File,FGuid::NewGuid()));FFileHelper::SaveStringToFile(TEXT("bad-primary"),*File);TestTrue(TEXT("Prior backup used"),E.Load(File,Id));TestTrue(TEXT("Recovery flag"),E.bRecovered);TestEqual(TEXT("Previous name preserved"),E.Loadouts[0].Name,FString(TEXT("FISKER 12")));TestTrue(TEXT("Explicit repair"),E.Save(File,Id));
    FFileHelper::SaveStringToFile(TEXT("preserve-primary"),*File);FFileHelper::SaveStringToFile(TEXT("preserve-backup"),*(File+TEXT(".bak")));TestFalse(TEXT("Two damaged files block"),E.Load(File,Id));TestFalse(TEXT("Save cannot discard damaged files"),D.Save(File,Id));FString Text;FFileHelper::LoadFileToString(Text,*File);TestEqual(TEXT("Primary unchanged"),Text,FString(TEXT("preserve-primary")));
    FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.Angler]\nVersion=3\n"),*File);TestFalse(TEXT("Future file preserved"),D.Load(File,Id));TestFalse(TEXT("Future file not replaced"),E.Save(File,Id));
    for(const FString& T:{FString(),FString(TEXT(".bak")),FString(TEXT(".tmp")),FString(TEXT(".bak.tmp"))}) { IFileManager::Get().Delete(*(File+T)); }IFileManager::Get().DeleteDirectory(*Dir);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskAlertTest,"Fiskelaget.Angler.RodAlertPrivacy",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskAlertTest::RunTest(const FString& Parameters)
{
    for(int32 S:{2,4,6}) { TestNotNull(TEXT("Owner action alert"),FiskAngler::RodAlert(S));const FString V=FiskAngler::RodAlert(S);TestFalse(TEXT("No exact catch values"),V.Contains(TEXT("kg")) || V.Contains(TEXT("fiskepenger"))); }
    for(int32 S:{0,1,3,5,7,99}) { TestNull(TEXT("No spurious private result notice"),FiskAngler::RodAlert(S)); }return true;
}
#endif
