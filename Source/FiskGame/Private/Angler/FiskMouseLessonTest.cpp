#include "Angler/FiskAnglerModel.h"
#include "FiskRodStick.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMouseLessonTest,"Fiskelaget.Angler.MouseLessonSteps",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMouseLessonTest::RunTest(const FString& Parameters)
{
    // The fishing school maps rod stick changes to lesson actions exactly like UFiskAnglerPanel::TickSchoolStick.
    FFiskControlLesson Lesson;FFiskRodStick Stick;Stick.Reset();bool Raised=false,Reeling=false;
    auto Frames=[&](float Speed,float Seconds)
    {
        for(float T=0;T<Seconds;T+=1.0f/60)
        {
            Stick.Move(Speed/60);Stick.Advance(1.0f/60,false);
            if(Stick.WantsRaised()!=Raised) { Raised=Stick.WantsRaised();Lesson.Act(Raised?EFiskLessonAction::Lift:EFiskLessonAction::Lower); }
            if(Stick.WantsReeling()!=Reeling) { Reeling=Stick.WantsReeling();Lesson.Act(Reeling?EFiskLessonAction::Reel:EFiskLessonAction::StopReel); }
        }
    };
    Frames(0.8f,0.6f);TestEqual(TEXT("Mouse up completes lift"),Lesson.Step,1);
    Frames(0.0f,0.4f);TestEqual(TEXT("Holding high keeps the rod lifted"),Lesson.Step,1);
    Frames(-0.8f,0.6f);TestEqual(TEXT("Mouse down lowers and reels"),Lesson.Step,3);
    Frames(0.0f,0.5f);TestEqual(TEXT("Resting stops the reel"),Lesson.Step,4);
    TestTrue(TEXT("Wheel and space finish the lesson"),Lesson.Act(EFiskLessonAction::Tighten) && Lesson.Act(EFiskLessonAction::Loosen) && Lesson.Act(EFiskLessonAction::Land) && Lesson.Complete());
    return true;
}
#endif
