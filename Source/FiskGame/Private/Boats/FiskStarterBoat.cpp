#include "Boats/FiskStarterBoat.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Boats/FiskBoatPresentation.h"
#include "Fishing/FiskFishPresentation.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

#include "FiskFishingGround.h"

AFiskStarterBoat::AFiskStarterBoat()
{
    SetActorHiddenInGame(false);
    bReplicates = true;
    bAlwaysRelevant = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("BoatRoot")));

    UCameraComponent* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("BoatCamera"));
    Camera->SetupAttachment(RootComponent);

    const FVector CameraPosition(-1250, -1200, 550);
    Camera->SetRelativeLocation(CameraPosition);
    Camera->SetRelativeRotation((FVector(-100, -600, 110) - CameraPosition).Rotation());
    Camera->FieldOfView = 75.0f;
    CreateDefaultSubobject<UFiskBoatPresentation>(TEXT("BoatPresentation"));
    CreateDefaultSubobject<UFiskFishPresentation>(TEXT("FishPresentation"));

    SetReplicateMovement(true);

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void AFiskStarterBoat::SetOwnerPlayer(int32 PlayerId)
{
    if (HasAuthority() && OwnerId == INDEX_NONE)
    {
        OwnerId = PlayerId;
        RefreshCaptain();
    }
}

void AFiskStarterBoat::ResetForDay()
{
    if (!HasAuthority()) { return; }
    SetActorTickEnabled(false); LegStartedMs = ReturnStartedMs = MarinaArrivalMs = -1;
    TripProgress = AtDock; ++Route.Revision;
    PortRodActivity = StarboardRodActivity = 0;
    PortFish = FFiskFishVisualState(); StarboardFish = FFiskFishVisualState();
    SetActorLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
    OnRep_TripProgress(); OnRep_Route(); ForceNetUpdate();
}

void AFiskStarterBoat::Board(int32 PlayerId)
{
    if (!HasAuthority() || TripProgress != AtDock || PlayerId == INDEX_NONE || IsAboard(PlayerId) || Crew.Num() >= Capacity)
    {
        return;
    }

    Crew.Add(PlayerId);
    RefreshCaptain();
    OnRep_Crew();
    ForceNetUpdate();
}

void AFiskStarterBoat::Leave(int32 PlayerId)
{
    if(!HasAuthority()) { return; }
    const bool bCaptainLeft=Route.CaptainId==PlayerId;
    if(Crew.Remove(PlayerId)>0)
    {
        if(bCaptainLeft) { bManualCaptain=false; }
        RefreshCaptain();OnRep_Crew();ForceNetUpdate();
    }
}

int32 AFiskStarterBoat::GetCaptainId() const
{
    return Crew.Contains(Route.CaptainId)?Route.CaptainId:INDEX_NONE;
}
void AFiskStarterBoat::RefreshCaptain()
{
    if(HasAuthority() && Route.RefreshCaptain(Crew,OwnerId,!bManualCaptain)) { OnRep_Route();ForceNetUpdate(); }
}
bool AFiskStarterBoat::TransferCaptain(int32 Requester,int32 ExpectedRevision,int32 Target)
{
    if(!HasAuthority() || !Route.Transfer(Requester,ExpectedRevision,Target,Crew)) { return false; }
    bManualCaptain=true;OnRep_Route();ForceNetUpdate();return true;
}

void AFiskStarterBoat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFiskStarterBoat, OwnerId);
    DOREPLIFETIME(AFiskStarterBoat, Route);
    DOREPLIFETIME(AFiskStarterBoat, Crew);
    DOREPLIFETIME(AFiskStarterBoat, TripProgress);
    DOREPLIFETIME(AFiskStarterBoat, MarinaArrivalMs);
    DOREPLIFETIME(AFiskStarterBoat, PortRodActivity);
    DOREPLIFETIME(AFiskStarterBoat, StarboardRodActivity);
    DOREPLIFETIME(AFiskStarterBoat, PortFish);
    DOREPLIFETIME(AFiskStarterBoat, StarboardFish);
}

void AFiskStarterBoat::OnRep_Crew()
{
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_STARTER_BOAT_CREW %d"), Crew.Num());
}

bool AFiskStarterBoat::SelectGround(int32 PlayerId,int32 ExpectedRevision,int32 GroundId,FString& Message)
{
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!HasAuthority() || PlayerId==INDEX_NONE || PlayerId!=GetCaptainId()) { Message=TEXT("Bare kapteinen kan velge plass.");return false; }
    if(TripProgress!=AtDock || !Day || (Day->HasStarted() && Day->GetElapsedSeconds()>=AFiskDayState::SettlementStart))
    { Message=TEXT("Velg fiskeplass ved brygga før avreise.");return false; }
    if(ExpectedRevision!=Route.Revision || Route.Revision==MAX_int32) { Message=TEXT("Kartet ble oppdatert. Velg på nytt.");return false; }
    if(!FiskGrounds::Find(GroundId)) { Message=TEXT("Ugyldig fiskeplass.");return false; }
    Route.GroundId=GroundId;++Route.Revision;OnRep_Route();ForceNetUpdate();
    Message=TEXT("Rute valgt. R starter turen fra 02:00.");return true;
}
void AFiskStarterBoat::OnRep_Route()
{
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_ROUTE id=%d revision=%d"),Route.GroundId,Route.Revision);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_STATE player=%d revision=%d"),Route.CaptainId,Route.Revision);
}
void AFiskStarterBoat::StartTrip(int32 PlayerId,int32 ExpectedRevision)
{
    if (!HasAuthority() || !CanCaptainCommand(PlayerId,ExpectedRevision))
    {
        return;
    }


    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!Day || !Day->HasStarted()) { return; }
    if (TripProgress == AtFishingSpot)
    {
        LegStartedMs = Day->GetElapsedMilliseconds();
        ReturnStartedMs = LegStartedMs;
        SetActorRotation((-FiskGrounds::Find(Route.GroundId)->Position).Rotation());

        TripProgress = AtFishingSpot + 1;
        OnRep_TripProgress();
        SetActorTickEnabled(true);
        ForceNetUpdate();
        return;
    }
    if (TripProgress != AtDock)
    {
        return;
    }

    if (Day->GetPhase(Day->GetElapsedSeconds()) != EFiskDayPhase::FishingAndReturn)
    {
        return;
    }

    LegStartedMs = Day->GetElapsedMilliseconds();
    SetActorRotation(FiskGrounds::Find(Route.GroundId)->Position.Rotation());
    TripProgress = 0;
    OnRep_TripProgress();
    SetActorTickEnabled(true);
    ForceNetUpdate();
}

void AFiskStarterBoat::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!HasAuthority() || !Day) { return; }
    const int32 NowMs = Day->GetElapsedMilliseconds();
    const FFiskFishingGround* Ground=FiskGrounds::Find(Route.GroundId);
    if(!Ground) { return; }
    const double Fraction = FMath::Clamp((NowMs - LegStartedMs) / static_cast<double>(Ground->TravelMs), 0.0, 1.0);
    if (MarinaArrivalMs < 0)
    {
        MarinaArrivalMs = GetMarinaArrivalMs(NowMs);
        if (MarinaArrivalMs >= 0)
        {
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MARINA_CROSSED ms=%d"), MarinaArrivalMs);
        }
    }

    const bool bReturning = TripProgress > AtFishingSpot;
    SetActorLocation(Ground->Position * (bReturning ? 1.0f - Fraction : Fraction));
    const int32 Percent = FMath::FloorToInt(Fraction * AtFishingSpot);
    TripProgress = bReturning ? AtFishingSpot + FMath::Max(1, Percent) : Percent;

    if (TripProgress == AtFishingSpot || TripProgress == AtMarina)
    {
        SetActorTickEnabled(false);
        OnRep_TripProgress();
        ForceNetUpdate();
    }
}

void AFiskStarterBoat::OnRep_TripProgress()
{
    if (TripProgress == AtDock) { bReportedDeparture = bReportedReturn = false; }
    if (TripProgress >= 0 && TripProgress < AtFishingSpot && !bReportedDeparture)
    {
        bReportedDeparture = true;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TRIP_UNDERWAY crew=%d"), Crew.Num());
    }
    else if (TripProgress > AtFishingSpot && TripProgress < AtMarina && !bReportedReturn)
    {
        bReportedReturn = true;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_RETURN_UNDERWAY crew=%d"), Crew.Num());
    }
    else if (TripProgress == AtMarina)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MARINA_REACHED crew=%d"), Crew.Num());
    }
    else if (TripProgress == AtFishingSpot)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TRIP_ARRIVED crew=%d"), Crew.Num());
    }
}


int32 AFiskStarterBoat::CalculateCrossingMs(int32 ReturnStartMs,int32 GroundId)
{
    return FiskGrounds::CrossingMs(ReturnStartMs,GroundId);
}

int32 AFiskStarterBoat::GetMarinaArrivalMs(int32 NowMs) const
{
    if (MarinaArrivalMs >= 0) { return MarinaArrivalMs; }
    if (ReturnStartedMs < 0) { return -1; }
    const int32 CrossingMs = CalculateCrossingMs(ReturnStartedMs, Route.GroundId);
    return NowMs >= CrossingMs ? CrossingMs : -1;
}


void AFiskStarterBoat::SetRodActivities(uint8 Port, uint8 Starboard)
{
    if (HasAuthority() && (PortRodActivity != Port || StarboardRodActivity != Starboard))
    {
        PortRodActivity = Port;
        StarboardRodActivity = Starboard;
        ForceNetUpdate();
    }
}


void AFiskStarterBoat::SetFishVisuals(const FFiskFishVisualState& Port, const FFiskFishVisualState& Starboard)
{
    if (HasAuthority() && (PortFish != Port || StarboardFish != Starboard))
    {
        PortFish = Port;
        StarboardFish = Starboard;
        ForceNetUpdate();
    }
}
