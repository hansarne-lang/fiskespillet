#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskPierWalk.generated.h"
class AHUD;class UCanvas;struct FInputKeyEventArgs;
UCLASS()
class UFiskPierWalk final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskPierWalk();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    bool HandleKey(const FInputKeyEventArgs& A);
    bool IsEnabled() const { return bEnabled; }
    FVector GetPosition() const { return Position; }
    float GetYaw() const { return Facing; }
    float GetSpeed() const { return Speed; }
    float GetCameraYaw() const { return CameraYaw; }
    float GetZoom() const { return Zoom; }
    int32 GetCameraMode() const { return CameraMode; }
    bool HasMenu() const;
    bool Draw(AHUD& H,UCanvas& C) const;
    void ServerMode(FGuid Profile,bool Enable);
    void ServerInput(FGuid Profile,int32 Generation,int32 Serial,FVector2D Axis,bool Sprint);
    void ReceiveMode(bool Enable,FVector P,int32 Generation);
#if WITH_DEV_AUTOMATION_TESTS
    int32 CoastTestView() const { return SmokeMode==TEXT("guide") && SmokeStage>=11 && SmokeStage<=13?SmokeStage-11:-1; }
#endif
private:
    UPROPERTY(Replicated) bool bEnabled=false;
    UPROPERTY(Replicated) FVector Position=FVector(-1300,0,130);
    float Facing=0,Speed=0,CameraYaw=0,Zoom=1;
    int32 CameraMode=0,GuideChoice=0,GuideIndex=0;
    bool bGuidePanel=false,bGuiding=false;
    TArray<FVector> GuideRoute;
    FString GuideStatus;
    FVector GuideLastPosition=FVector::ZeroVector;
    double GuideMovedAt=0;
    int32 GreetingSerial=0;
    double GreetingUntil=0;
    bool bPending=false,bDispatching=false,bUp=false,bDown=false,bLeft=false,bRight=false,bSprint=false,bServerSprint=false;
    FVector2D ServerAxis=FVector2D::ZeroVector;
    int32 Generation=0,LastSerial=0,Serial=0;
    double LastInputAt=-1,NextSend=0;
    void ClearKeys();
    void Interact();
    FString DestinationName(int32 I) const;
    void StartGuide();
    void StopGuide(const FString& Reason);
    FVector2D GuideAxis(double Now);
#if WITH_DEV_AUTOMATION_TESTS
    FString SmokeMode,SmokeDir;FTimerHandle SmokeTimer;int32 SmokeStage=0,SmokeWaypoint=0;
    double SmokeAt=0;FVector SmokeOrigin=FVector::ZeroVector;int64 SmokeCash=0;
    void RunSmoke();
    void RunGuideSmoke();
#endif
};
