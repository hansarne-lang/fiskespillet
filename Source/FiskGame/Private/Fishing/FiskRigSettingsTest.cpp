#include "Fishing/FiskRigSettings.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRigValidation,"Fiskelaget.Rig.ValidationAndRevision",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRigValidation::RunTest(const FString& Parameters)
{
    FFiskRigSettings R;
    TestTrue(TEXT("Default valid"),FFiskRigRules::IsValid(R.Bait,R.Depth,R.Program));
    for (int32 B=0;B<3;++B) { for (int32 P=0;P<3;++P) { for (int32 D=10;D<=60;D+=10) { TestTrue(TEXT("All choices valid"),FFiskRigRules::IsValid(B,D,P)); } } }
    TestFalse(TEXT("Invalid enum"),FFiskRigRules::TryApply(R,0,3,30,0));
    TestFalse(TEXT("Negative enum"),FFiskRigRules::TryApply(R,0,-1,30,0));
    TestFalse(TEXT("Below bottom"),FFiskRigRules::TryApply(R,0,0,70,0));
    TestFalse(TEXT("Fractional step"),FFiskRigRules::TryApply(R,0,0,35,0));
    TestFalse(TEXT("Invalid program"),FFiskRigRules::TryApply(R,0,0,30,3));
    TestEqual(TEXT("Rejections unchanged"),R.Revision,0);
    TestTrue(TEXT("Valid change"),FFiskRigRules::TryApply(R,0,2,40,2));
    TestFalse(TEXT("Duplicate request rejected"),FFiskRigRules::TryApply(R,0,0,10,0));
    TestEqual(TEXT("Accepted bait preserved"),R.Bait,2);
    TestEqual(TEXT("Accepted revision"),R.Revision,1);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRigDistribution,"Fiskelaget.Rig.QualityAndIndependentBites",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRigDistribution::RunTest(const FString& Parameters)
{
    double Means[3]={0,0,0};
    for (int32 Q=0;Q<3;++Q)
    {
        FFiskRigSettings R; R.Depth=Q==2?30:60;R.Program=Q==0?2:0;
        TestEqual(TEXT("Quality"),FFiskRigRules::Quality(R),Q);
        double First=-1;bool bVaries=false;
        FRandomStream One(5371),Two(5371),Other(6937);
        for(int32 I=0;I<10000;++I)
        {
            double T=FFiskRigRules::BiteDelay(R,One);
            const double Min[]={300,180,120},Max[]={480,300,180};
            if(T<Min[Q] || T>Max[Q]) { AddError(TEXT("Delay outside band"));return false; }
            Means[Q]+=T; if(First<0) { First=T; } else { bVaries|=T!=First; }
            FFiskRigRules::BiteDelay(R,Other); // Another rod must not consume this rod's random stream.
            if(T!=FFiskRigRules::BiteDelay(R,Two)) { AddError(TEXT("Other rod affected sequence"));return false; }
        }
        TestTrue(TEXT("Not a fixed bite interval"),bVaries);
        Means[Q]/=10000;
    }
    TestTrue(TEXT("Good mean 2-3 min"),Means[2]>120 && Means[2]<180);
    TestTrue(TEXT("Medium mean 3-5 min"),Means[1]>180 && Means[1]<300);
    TestTrue(TEXT("Poor mean 5-8 min"),Means[0]>300 && Means[0]<480);
    TestTrue(TEXT("Meaningful quality difference"),Means[0]>Means[1] && Means[1]>Means[2]);
    return true;
}
#endif
