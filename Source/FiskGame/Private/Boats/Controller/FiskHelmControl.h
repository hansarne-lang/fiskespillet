#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskHelmControl.generated.h"
class AHUD;class UCanvas;class AFiskStarterBoat;struct FInputKeyEventArgs;

// The captain's helm on the client (#176 step 3). W/S move a throttle lever in quarter steps (S below
// zero is reverse), A/D held turn the rudder. While the captain steers, the lever and rudder go to the
// server about ten times a second (UFiskBoatCommands::ServerHelm); the server moves the boat. When the
// chart's autopilot or R takes over, the lever returns to zero. Lives on the player's controller.
UCLASS()
class UFiskHelmControl final : public UActorComponent
{
    GENERATED_BODY()
public:
    static constexpr float LeverStep=0.25f;
    UFiskHelmControl();
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    bool HandleKey(const FInputKeyEventArgs& A);   // Client: true when W/A/S/D steered the boat
    void Draw(AHUD& H,UCanvas& C) const;           // Client: helm gauge for everyone aboard at sea
    float GetLever() const { return Lever; }
    float GetRudder() const { return (bRight?1.f:0.f)-(bLeft?1.f:0.f); }
    void ReleaseLever() { Lever=0;bLeft=bRight=false; }   // R hands the boat to the autopilot
private:
    const AFiskStarterBoat* OwnBoat(bool& bCaptain) const;
    float Lever=0;
    bool bLeft=false,bRight=false,bWasUnderHelm=false;
    int32 Serial=0;
    double NextSend=0;
};
