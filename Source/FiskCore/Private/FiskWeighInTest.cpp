#include "FiskWeighIn.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include <limits>
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskWeighInOrderTest,"Fiskelaget.WeighIn.SmallestFirstLargestLast",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskWeighInOrderTest::RunTest(const FString&)
{
    const TArray<int32> Grams={12000,3000,150000,3000,8000};
    const TArray<int32> Order=FiskWeighIn::Order(Grams);
    TestEqual(TEXT("Every fish once"),Order.Num(),5);
    TestEqual(TEXT("Smallest first"),Order[0],1);
    TestEqual(TEXT("Equal weights keep landing order"),Order[1],3);
    TestEqual(TEXT("Then 8 kg"),Order[2],4);
    TestEqual(TEXT("Then 12 kg"),Order[3],0);
    TestEqual(TEXT("Largest last"),Order[4],2);
    TestTrue(TEXT("No fish, no order"),FiskWeighIn::Order({}).IsEmpty());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskWeighInPaceTest,"Fiskelaget.WeighIn.PaceAndLongDays",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskWeighInPaceTest::RunTest(const FString&)
{
    using namespace FiskWeighIn;
    TestEqual(TEXT("One fish is the largest"),StepSeconds(0,1),LargestSeconds);
    TestEqual(TEXT("Small fish"),StepSeconds(0,6),SmallSeconds);
    TestEqual(TEXT("Second largest"),StepSeconds(4,6),LargeSeconds);
    TestEqual(TEXT("Fourth largest"),StepSeconds(2,6),LargeSeconds);
    TestEqual(TEXT("Largest last"),StepSeconds(5,6),LargestSeconds);
    TestEqual(TEXT("Six fish"),TotalSeconds(6),2*SmallSeconds+3*LargeSeconds+LargestSeconds);
    TestTrue(TEXT("A very long day is weighed at about the minimum pace"),TotalSeconds(200)<=200*MinSeconds+MaxTotalSeconds*.05);
    TestTrue(TEXT("Never above the cap unless the minimum forces it"),TotalSeconds(60)<=MaxTotalSeconds+.001);
    TestTrue(TEXT("The largest still gets the longest time"),StepSeconds(59,60)>StepSeconds(0,60));
    TestEqual(TEXT("Outside the range"),StepSeconds(6,6),0.0);
    TestEqual(TEXT("Nothing to weigh"),TotalSeconds(0),0.0);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskWeighInProgressTest,"Fiskelaget.WeighIn.OneFishAtATime",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskWeighInProgressTest::RunTest(const FString&)
{
    using namespace FiskWeighIn;
    TestEqual(TEXT("Before the start"),OnScale(-1,3),0);
    TestEqual(TEXT("The first goes on at once"),OnScale(0,3),1);
    TestEqual(TEXT("Still the first"),OnScale(SmallSeconds-.01,3),1);
    TestEqual(TEXT("Then the second"),OnScale(LargeSeconds,3),2);
    TestEqual(TEXT("The largest is on the scale"),OnScale(2*LargeSeconds,3),3);
    TestFalse(TEXT("Not finished while the largest is on"),Finished(2*LargeSeconds,3));
    TestTrue(TEXT("Finished after the largest's time"),Finished(2*LargeSeconds+LargestSeconds,3));
    TestEqual(TEXT("Never more than all"),OnScale(1000,3),3);
    TestTrue(TEXT("Nothing to weigh is finished at once"),Finished(0,0));
    TestEqual(TEXT("Bad time"),OnScale(std::numeric_limits<double>::quiet_NaN(),3),0);
    TestFalse(TEXT("Bad time never finishes"),Finished(std::numeric_limits<double>::quiet_NaN(),3));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskWeighInStartTest,"Fiskelaget.WeighIn.StartsWhenEveryoneIsAshore",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskWeighInStartTest::RunTest(const FString&)
{
    using namespace FiskWeighIn;
    const int32 Deadline=1020000;
    TestTrue(TEXT("Everyone home before 17:00"),ShouldStart(400000,Deadline,false,true));
    TestFalse(TEXT("A boat still out"),ShouldStart(400000,Deadline,true,true));
    TestFalse(TEXT("Nobody has been out yet"),ShouldStart(400000,Deadline,false,false));
    TestFalse(TEXT("Not before 02:00"),ShouldStart(EarliestStartMs-1,Deadline,false,true));
    TestTrue(TEXT("At 17:00 even with a boat out"),ShouldStart(Deadline,Deadline,true,true));
    TestTrue(TEXT("At 17:00 also when nobody went out"),ShouldStart(Deadline,Deadline,false,false));
    return true;
}
#endif
