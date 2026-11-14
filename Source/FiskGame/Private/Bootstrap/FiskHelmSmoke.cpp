#include "Bootstrap/FiskHelmSmoke.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Boats/Controller/FiskHelmControl.h"
#include "DayCycle/FiskDayState.h"
#include "FiskRouteGeometry.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

void UFiskHelmSmoke::BeginPlay()
{
    Super::BeginPlay();
    FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),SmokeDir);
    GetWorld()->GetTimerManager().SetTimer(SmokeTimer,this,&UFiskHelmSmoke::RunSmoke,.25f,true,3.f);
}

void UFiskHelmSmoke::RunSmoke()
{
    auto* P=CastChecked<AFiskBoardingController>(GetOwner());const auto* Helm=P->FindComponentByClass<UFiskHelmControl>();const auto* D=GetWorld()->GetGameState<AFiskDayState>();
    if(!Helm || !P->IsCareerReady() || !P->PlayerState || !D || !D->HasStarted()) { return; }
    const auto* B=P->FindStarterBoat();if(!B) { return; }
    const double Real=GetWorld()->GetRealTimeSeconds();const int32 Self=P->PlayerState->GetPlayerId();
    auto Key=[&](FKey K,EInputEvent E=IE_Pressed){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=E;P->InputKey(A);if(E==IE_Pressed && K!=EKeys::A && K!=EKeys::D) { A.Event=IE_Released;P->InputKey(A); }};
    auto Shot=[&](const TCHAR* Name){if(!SmokeDir.IsEmpty() && !FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(FPaths::Combine(SmokeDir,FString(Name)+TEXT(".png")),false,false); }};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_HELM_SMOKE_FAILED stage=%d %s"),SmokeStage,Why);GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);};
    auto Next=[&](int32 Stage){SmokeStage=Stage;SmokeAt=Real;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HELM_SMOKE_STAGE %d ms=%d"),Stage,D->GetElapsedMilliseconds());};
    if(Real-SmokeAt>120 && SmokeStage>0) { Fail(TEXT("stage_timeout"));return; }
    switch(SmokeStage)
    {
    case 0:
        if(!B->IsAboard(Self)) { Key(EKeys::E);return; }
        if(B->GetCaptainId()!=Self) { return; }
        if(D->GetElapsedSeconds()<AFiskDayState::PreparationEnd) { Key(EKeys::F12);return; }   // Test-only clock skip to 02:00
        Key(EKeys::W);Key(EKeys::W);Next(1);return;
    case 1:   // Leaves the dock under the helm at half throttle
        if(!B->IsUnderHelm()) { if(Real-SmokeAt>10) { Fail(TEXT("no_departure_under_helm")); }return; }
        if(FMath::IsNearlyEqual(Helm->GetLever(),.5f)==false) { Fail(TEXT("lever_not_half"));return; }
        if(B->GetActorLocation().X<4000) { return; }
        SmokeYaw=B->GetActorRotation().Yaw;Key(EKeys::D);Next(2);return;
    case 2:   // Starboard rudder turns the boat (half a second: still heading out to sea)
        if(Real-SmokeAt<.5) { return; }
        Key(EKeys::D,IE_Released);
        if(FMath::FindDeltaAngleDegrees(SmokeYaw,B->GetActorRotation().Yaw)<3) { Fail(TEXT("rudder_did_not_turn"));return; }
        Shot(TEXT("helm-underway"));Next(3);return;
    case 3:   // Out to open sea, then the lever back to zero
        if(FiskRouteGeometry::Sea(B->GetActorLocation()) && B->GetActorLocation().Size2D()>12000) { Key(EKeys::S);Key(EKeys::S);Next(4); }
        else if(Real-SmokeAt>60) { Fail(TEXT("never_reached_open_sea")); }
        return;
    case 4:   // Stopped with the throttle released: a fishing spot
        if(Helm->GetLever()!=0) { Fail(TEXT("lever_not_zero"));return; }
        if(B->GetTripProgress()!=AFiskStarterBoat::AtFishingSpot) { return; }
        if(B->IsUnderHelm()) { Fail(TEXT("stopped_but_still_under_helm"));return; }
        SmokeStop=B->GetActorLocation();
        if(FVector::Dist2D(SmokeStop,B->GetDestination())>1) { Fail(TEXT("spot_not_where_boat_stopped"));return; }
        Shot(TEXT("helm-stopped"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HELM_SMOKE_SPOT x=%.0f y=%.0f ground=%d"),SmokeStop.X,SmokeStop.Y,B->GetGroundId());Next(5);return;
    case 5:   // Two seconds still, then on again
        if(FVector::Dist2D(B->GetActorLocation(),SmokeStop)>1) { Fail(TEXT("stopped_boat_drifted"));return; }
        if(Real-SmokeAt<2) { return; }
        Key(EKeys::W);Next(6);return;
    case 6:
        if(!B->IsUnderHelm()) { if(Real-SmokeAt>10) { Fail(TEXT("could_not_move_on")); }return; }
        if(FVector::Dist2D(B->GetActorLocation(),SmokeStop)<1500) { return; }
        Key(EKeys::R);Next(7);return;   // R from under the helm: autopilot home
    case 7:
        if(B->IsUnderHelm() && Real-SmokeAt>5) { Fail(TEXT("R_did_not_take_over"));return; }
        if(Helm->GetLever()!=0 && !B->IsUnderHelm()) { Fail(TEXT("lever_not_reset_by_autopilot"));return; }
        if(B->GetTripProgress()!=AFiskStarterBoat::AtMarina) { return; }
        if(B->GetMarinaArrivalMs(D->GetElapsedMilliseconds())<0) { Fail(TEXT("missing_marina_arrival"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HELM_SMOKE_PASSED arrival=%d"),B->GetMarinaArrivalMs(D->GetElapsedMilliseconds()));
        GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);return;
    default: return;
    }
}
