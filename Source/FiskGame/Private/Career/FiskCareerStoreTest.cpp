#include "Career/FiskCareerStore.h"
#include "Fishing/FiskFightModel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCareerStorageTest,"Fiskelaget.Career.AtomicSaveRecoveryAndConflict",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCareerStorageTest::RunTest(const FString& Parameters)
{
    const FString File=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(),TEXT("Career.ini"));
    FFiskCareerProfile P=FFiskCareerProfile::Create(TEXT("TEST")),Reload;
    TestTrue(TEXT("First save"),FFiskCareerStore::Save(File,P));const FFiskCareerProfile Initial=P;
    P.Buy(FGuid::NewGuid(),1,0);TestTrue(TEXT("Purchase persisted"),FFiskCareerStore::Save(File,P));
    TestEqual(TEXT("New process loads"),FFiskCareerStore::Load(File,Reload),EFiskCareerLoad::Loaded);
    TestEqual(TEXT("Equipment restored"),Reload.Owned,1);TestEqual(TEXT("Cash retained"),Reload.Cash,P.Cash);
    FFiskCareerProfile Branch=Initial;Branch.Buy(FGuid::NewGuid(),1,1);TestFalse(TEXT("Concurrent divergent purchase not overwritten"),FFiskCareerStore::Save(File,Branch));
    FFileHelper::SaveStringToFile(TEXT("damaged-primary"),*File);
    TestEqual(TEXT("Backup selected"),FFiskCareerStore::Load(File,Reload),EFiskCareerLoad::Recovered);
    TestTrue(TEXT("Repair from confirmed chain"),FFiskCareerStore::Save(File,P));
    TestEqual(TEXT("Repaired latest money"),FFiskCareerStore::Load(File,Reload),EFiskCareerLoad::Loaded);TestEqual(TEXT("No repeat grant"),Reload.Cash,P.Cash);
    FFileHelper::SaveStringToFile(TEXT("future-main"),*File);FFileHelper::SaveStringToFile(TEXT("future-backup"),*(File+TEXT(".bak")));
    TestEqual(TEXT("Two bad copies blocked"),FFiskCareerStore::Load(File,Reload),EFiskCareerLoad::Blocked);
    TestFalse(TEXT("Bad copies preserved"),FFiskCareerStore::Save(File,P));FString After;FFileHelper::LoadFileToString(After,*File);TestEqual(TEXT("Bytes preserved"),After,FString(TEXT("future-main")));
    for(const TCHAR* S:{TEXT(""),TEXT(".bak"),TEXT(".tmp"),TEXT(".bak.tmp")}) { IFileManager::Get().Delete(*(File+S)); }
    IFileManager::Get().DeleteDirectory(*FPaths::GetPath(File));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskEquipmentFightTest,"Fiskelaget.Career.EquipmentChangesActualFight",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskEquipmentFightTest::RunTest(const FString& Parameters)
{
    FFiskFightModel Base,Line;Base.Reset(7,100,1);Line.Reset(7,100,1,1);
    for(int32 I=0;I<2;++I) { Base.AdjustDrag(1);Line.AdjustDrag(1); }Base.SetRaised(true);Line.SetRaised(true);Base.Advance(12);Line.Advance(12);
    TestEqual(TEXT("Original line breaks when lifting into run"),Base.GetOutcome(),EFiskFightOutcome::LineBreak);
    TestEqual(TEXT("Better line survives the same load"),Line.GetOutcome(),EFiskFightOutcome::Fighting);
    Base.Reset(7,100,1);Line.Reset(7,100,1,2);Base.SetRaised(true);Line.SetRaised(true);Base.Advance(1.5);Line.Advance(1.5);
    Base.SetRaised(false);Line.SetRaised(false);Base.SetReeling(true);Line.SetReeling(true);Base.Advance(.25);Line.Advance(.25);
    TestTrue(TEXT("Reel improves actual gain"),Line.GetLine()<Base.GetLine());
    for(int32 Gear=0;Gear<8;++Gear) { for(int32 Species=0;Species<3;++Species)
    {
        FFiskFightModel F;F.Reset(43,60,Species,Gear);
        for(int32 Tick=0;Tick<12000 && F.GetOutcome()==EFiskFightOutcome::Fighting;++Tick)
        { const bool Lift=!F.IsSurging() && (Tick/40)%2==0;F.SetRaised(Lift);F.SetReeling(!F.IsSurging() && !Lift);F.Advance(.05); }
        TestEqual(TEXT("Equipment remains playable for every species"),F.GetOutcome(),EFiskFightOutcome::ReadyToLand);
    } }return true;
}
#endif
