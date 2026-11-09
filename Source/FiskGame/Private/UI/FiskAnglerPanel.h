#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputKeyEventArgs.h"
#include "Angler/FiskAnglerModel.h"
#include "FiskRodStick.h"
#include "FiskAnglerPanel.generated.h"
class AHUD;class UCanvas;
UCLASS()
class UFiskAnglerPanel final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskAnglerPanel();
    bool IsVisible() const { return Page>0; }
    int32 GetPage() const { return Page; }
    bool OpenSchool();
    bool HandleKey(const FInputKeyEventArgs& A);
    void Draw(AHUD& H,UCanvas& C);
    void DrawNotice(AHUD& H,UCanvas& C) const;
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* F) override;
    const FFiskPracticeSession& GetPractice() const { return Practice; }
    int32 GetLessonStep() const { return Lesson.Step; }
    const FFiskAnglerData& GetData() const { return Data; }
private:
    int32 Page=0,Species=0,Bait=0,Slot=0,Gear=0;
    bool bControls=true,bRenaming=false;
    FString Message,EditName,Notice;
    double NoticeUntil=0;
    TArray<int32> PreviousStates;
    FGuid LoadedProfile;
    FFiskAnglerData Data;
    FFiskControlLesson Lesson;
    FFiskPracticeSession Practice;
    // Mouse-steered rod in the fishing school (#132): the same stick as in a real fight.
    FFiskRodStick SchoolStick;
    FFiskStickEngage SchoolEngage;
    bool bSchoolRaised=false,bSchoolReeling=false,bSchoolButtons=false,bSchoolLiftHeld=false,bSchoolReelHeld=false;
    void ResetSchoolStick();
    void TickSchoolStick(float Dt);
    void LessonAct(EFiskLessonAction Action);
    FString File() const;
    void Load();
    void StartPractice();
    void DrawSchool(AHUD& H,UCanvas& C);
    void DrawWorkbench(AHUD& H,UCanvas& C);
};
