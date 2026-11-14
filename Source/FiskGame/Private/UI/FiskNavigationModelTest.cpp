#include "UI/FiskNavigationModel.h"
#include "FiskFishingGround.h"
#include "Fishing/FiskPrototypeRod.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskSonarTest,"Fiskelaget.Navigation.SonarLocationAndPublicDepths",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskSonarTest::RunTest(const FString& Parameters)
{
    TestFalse(TEXT("No dock reading"),FiskNavigation::Sonar(0,-1,true,30).bAvailable);
    TestFalse(TEXT("No travelling reading"),FiskNavigation::Sonar(1,50,true,80).bAvailable);
    TestFalse(TEXT("No remote reading"),FiskNavigation::Sonar(1,100,false,80).bAvailable);
    TestFalse(TEXT("No invalid ground"),FiskNavigation::Sonar(FiskGrounds::Count,100,true,80).bAvailable);
    TestFalse(TEXT("No return reading"),FiskNavigation::Sonar(1,150,true,80).bAvailable);
    const FFiskSonarView V=FiskNavigation::Sonar(1,100,true,80);
    TestTrue(TEXT("Current ground available"),V.bAvailable);TestEqual(TEXT("Bottom matches chart"),V.Bottom,120);
    TestEqual(TEXT("Shallow band"),V.Near,60);TestEqual(TEXT("Deep band"),V.Far,90);TestEqual(TEXT("Own rig"),V.BaitDepth,80);
    TestEqual(TEXT("Offshore bottom"),FiskNavigation::Sonar(2,100,true,40).Bottom,250);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBoardPrivacyTest,"Fiskelaget.Navigation.BoardPrivacyAndTies",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskBoardPrivacyTest::RunTest(const FString& Parameters)
{
    FFiskSettlementView View;View.DayNumber=2;View.bRevealed=true;
    for(int32 I=0;I<3;++I) { FFiskDayResult R;R.PlayerId=10-I;R.SessionBalance=I==2?20:80;View.Results.Add(R); }
    TestEqual(TEXT("Standings after the weigh-in, also before 17:00 (#183)"),FiskNavigation::Standings(View,2,1019999).Num(),3);
    TestTrue(TEXT("Wrong day hidden"),FiskNavigation::Standings(View,3,1020000).IsEmpty());
    View.bRevealed=false;TestTrue(TEXT("Unconfirmed hidden"),FiskNavigation::Standings(View,2,1020000).IsEmpty());View.bRevealed=true;
    const auto Rows=FiskNavigation::Standings(View,2,1020000);
    TestEqual(TEXT("All rows"),Rows.Num(),3);if(Rows.Num()!=3) { return false; }
    TestEqual(TEXT("Stable player tie ordering"),Rows[0].Player,9);
    TestEqual(TEXT("First place"),Rows[0].Rank,1);TestEqual(TEXT("Equal income shares first"),Rows[1].Rank,1);
    TestEqual(TEXT("Competition rank"),Rows[2].Rank,3);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBoardRecordTest,"Fiskelaget.Navigation.RecordsAndDayTransition",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskBoardRecordTest::RunTest(const FString& Parameters)
{
    FFiskSettlementView V;V.DayNumber=2;V.bRevealed=false;
    FFiskHistoryDay Old;Old.Number=1;FFiskHistoryRow R;R.PlayerId=7;R.BestGrams=10000;R.BestSpecies=1;R.Lost=1;Old.Rows.Add(R);
    FFiskHistoryDay Current=Old;Current.Number=2;Current.Rows[0].BestGrams=20000;
    FFiskHistoryDay Future=Old;Future.Number=3;Future.Rows[0].BestGrams=50000;
    const TArray<FFiskHistoryDay> History={Old,Current,Future};
    auto Rows=FiskNavigation::Records(History,V,2,1019999);
    TestEqual(TEXT("Old lost record retained"),Rows.Num(),1);if(Rows.IsEmpty()) { return false; }
    TestEqual(TEXT("Current and future cannot leak via history"),Rows[0].BestGrams,10000);
    V.bRevealed=true;Rows=FiskNavigation::Records(History,V,2,1020000);
    TestEqual(TEXT("Revealed new record"),Rows[0].BestGrams,20000);TestEqual(TEXT("Correct day"),Rows[0].Day,2);
    V.DayNumber=3;V.bRevealed=false;Rows=FiskNavigation::Records(History,V,3,0);
    TestEqual(TEXT("Day 2 retained on next morning"),Rows[0].BestGrams,20000);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskNextActionTest,"Fiskelaget.Navigation.NextActionPhases",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskNextActionTest::RunTest(const FString& Parameters)
{
    auto Hint=[](int32 Ms,int32 Progress,bool Aboard,int32 Rod=0,bool Rig=true,bool Busy=false){return FiskNavigation::NextAction(true,Ms,Progress,Aboard,true,true,Rod,Rig,Busy);};
    TestTrue(TEXT("Board first"),Hint(0,-1,false).StartsWith(TEXT("E:")));
    TestTrue(TEXT("Prep until real 02"),Hint(119999,-1,true).Contains(TEXT("02:00")));
    TestTrue(TEXT("Departure at 02"),Hint(120000,-1,true).StartsWith(TEXT("R:")));
    TestTrue(TEXT("Bad rig"),Hint(160000,100,true,0,false).StartsWith(TEXT("G:")));
    TestTrue(TEXT("Cast when ready"),Hint(160000,100,true).StartsWith(TEXT("F:")));
    TestTrue(TEXT("Landing cue"),Hint(300000,100,true,static_cast<int32>(EFiskRodState::ReadyToLand)).StartsWith(TEXT("Mellomrom:")));
    TestTrue(TEXT("Landing busy"),Hint(300000,100,true,static_cast<int32>(EFiskRodState::Landed),true,true).Contains(TEXT("pågår")));
    TestTrue(TEXT("Returned cue"),Hint(900000,200,true).Contains(TEXT("17:00")));
    TestTrue(TEXT("Settlement cue"),Hint(1020000,200,true).StartsWith(TEXT("F2:")));
    TestTrue(TEXT("Next day"),Hint(1200000,200,true).StartsWith(TEXT("N:")));
    return true;
}
#endif
