#include "Boats/FiskStarterBoat.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Boats/FiskBoatPresentation.h"
#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskFishPresentation.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

#include "FiskFishingGround.h"
#include "FiskRouteGeometry.h"
#include "Boats/FiskJourneyAudio.h"

void AFiskStarterBoat::CalcCamera(float DeltaTime,FMinimalViewInfo& OutResult)
{
    Super::CalcCamera(DeltaTime,OutResult);
    const AFiskBoardingController* PC=GetWorld()?Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController()):nullptr;
    if(!PC || !PC->IsLocalController()) { return; }
    const int32 View=PC->GetCameraView();
    const FVector Eye=View==2?FVector(-1700,-200,1000):FVector(-1250,View==1?1200:-1200,550);
    const FVector Focus=View==2?FVector(-200,0,130):View==1?FVector(-250,150,120):FVector(-100,-600,110);
    OutResult.Location=GetActorTransform().TransformPosition(Eye*(Kind==1?1.3:1.0));
    OutResult.Rotation=(GetActorTransform().TransformPosition(Focus)-OutResult.Location).Rotation();
    OutResult.FOV=View==2?70.0f:75.0f;
}

AFiskStarterBoat::AFiskStarterBoat()
{
    RodActivities.Init(0,6);FishVisuals.SetNum(6);
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
    CreateDefaultSubobject<UFiskJourneyAudio>(TEXT("JourneyAudio"));

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
    RodActivities.Init(0,6);FishVisuals.Init(FFiskFishVisualState(),6);
    SetActorLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
    OnRep_TripProgress(); OnRep_Route(); ForceNetUpdate();
}

void AFiskStarterBoat::Board(int32 PlayerId)
{
    if (!HasAuthority() || TripProgress != AtDock || PlayerId == INDEX_NONE || IsAboard(PlayerId) || Crew.Num() >= GetCapacity())
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
    DOREPLIFETIME(AFiskStarterBoat,Kind);DOREPLIFETIME(AFiskStarterBoat,RodActivities);DOREPLIFETIME(AFiskStarterBoat,FishVisuals);
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
    Route.GroundId=GroundId;Route.Destination=FiskGrounds::Find(GroundId)->Position;Route.BaseTravelMs=FiskGrounds::Find(GroundId)->TravelMs;
    ++Route.Revision;OnRep_Route();ForceNetUpdate();
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
        SetActorRotation((-Route.Destination).Rotation());

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
    SetActorRotation(Route.Destination.Rotation());
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
    const TArray<FVector> Path=FiskRouteGeometry::Path(Route.Destination);
    if(Path.IsEmpty()) { return; }
    const double Fraction = FMath::Clamp((NowMs - LegStartedMs) / static_cast<double>(GetTravelMs()), 0.0, 1.0);
    if (MarinaArrivalMs < 0)
    {
        MarinaArrivalMs = GetMarinaArrivalMs(NowMs);
        if (MarinaArrivalMs >= 0)
        {
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MARINA_CROSSED ms=%d"), MarinaArrivalMs);
        }
    }

    const bool bReturning = TripProgress > AtFishingSpot;
    const double Along=bReturning?1.0-Fraction:Fraction;
    const FVector Position=FiskRouteGeometry::Position(Path,Along);
    const FVector Next=FiskRouteGeometry::Position(Path,FMath::Clamp(Along+(bReturning?-0.001:0.001),0.0,1.0));
    if(!Next.Equals(Position,.01)) { SetActorRotation((Next-Position).Rotation()); }SetActorLocation(Position);
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
    const double Length=FiskRouteGeometry::Length(FiskRouteGeometry::Path(Route.Destination));
    const int32 CrossingMs = ReturnStartedMs+FMath::CeilToInt(GetTravelMs()*(1.0-500.0/FMath::Max(Length,500.0)));
    return NowMs >= CrossingMs ? CrossingMs : -1;
}


void AFiskStarterBoat::SetRodActivities(uint8 Port, uint8 Starboard)
{
    if(RodActivities.Num()==6) { RodActivities[0]=Port;RodActivities[1]=Starboard; }
    if (HasAuthority() && (PortRodActivity != Port || StarboardRodActivity != Starboard))
    {
        PortRodActivity = Port;
        StarboardRodActivity = Starboard;
        ForceNetUpdate();
    }
}


void AFiskStarterBoat::SetFishVisuals(const FFiskFishVisualState& Port, const FFiskFishVisualState& Starboard)
{
    if(FishVisuals.Num()==6) { FishVisuals[0]=Port;FishVisuals[1]=Starboard; }
    if (HasAuthority() && (PortFish != Port || StarboardFish != Starboard))
    {
        PortFish = Port;
        StarboardFish = Starboard;
        ForceNetUpdate();
    }
}

bool AFiskStarterBoat::ChooseKind(int32 Requester,int32 Revision,int32 NewKind)
{
    if(!HasAuthority() || Requester!=OwnerId || Revision!=Route.Revision || Route.Revision==MAX_int32
        || (NewKind!=0 && NewKind!=1) || (TripProgress!=AtDock && TripProgress!=AtMarina) || Crew.Num()>FiskBoatLayout::Seats(NewKind)) { return false; }
    Kind=NewKind;++Route.Revision;OnRep_Route();ForceNetUpdate();return true;
}
bool AFiskStarterBoat::SelectPoint(int32 Requester,int32 Revision,FVector Point,FString& Message)
{
    int32 Habitat=0,Duration=0;
    if(!FiskGrounds::MakeSeaPoint(Point,Habitat,Duration)) { Message=TEXT("Velg sjø innenfor kartet, utenfor øyer og marina.");return false; }
    if(!SelectGround(Requester,Revision,Habitat,Message)) { return false; }
    Route.Destination=Point;Route.BaseTravelMs=Duration;ForceNetUpdate();
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_SEA_POINT x=%.0f y=%.0f habitat=%d travel=%d"),Point.X,Point.Y,Habitat,GetTravelMs());return true;
}
int32 AFiskStarterBoat::ReturnMarginMs(int32 NowMs) const
{
    int64 Home=NowMs;const int64 Travel=GetTravelMs();
    if(TripProgress==-1) { Home=FMath::Max(NowMs,120000)+2*Travel; }
    else if(TripProgress<100) { Home+=FMath::DivideAndRoundUp<int64>((100LL-TripProgress)*Travel,100)+Travel; }
    else if(TripProgress==100) { Home+=Travel; }
    else if(TripProgress<200) { Home+=FMath::DivideAndRoundUp<int64>((TripProgress==101?100LL:200LL-TripProgress)*Travel,100); }
    return static_cast<int32>(FMath::Clamp<int64>(1020000-Home,MIN_int32,MAX_int32));
}
void AFiskStarterBoat::SetHolderVisuals(const TArray<uint8>& Activities,const TArray<FFiskFishVisualState>& Fish)
{
    if(!HasAuthority() || Activities.Num()!=6 || Fish.Num()!=6) { return; }
    bool Changed=RodActivities!=Activities;
    for(int32 I=0;I<6;++I) { Changed|=FishVisuals[I]!=Fish[I]; }
    if(Changed) { RodActivities=Activities;FishVisuals=Fish;ForceNetUpdate(); }
}
