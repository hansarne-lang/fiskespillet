#include "Boats/FiskStarterBoat.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Boats/FiskBoatPresentation.h"
#include "Boats/FiskCrewFigures.h"
#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskFishPresentation.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

#include "FiskFishingGround.h"
#include "FiskRouteGeometry.h"
#include "Boats/FiskJourneyAudio.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Expedition/FiskExpeditionState.h"
#include "FiskVoyageRules.h"
#include "Fishing/FiskPrototypeRod.h"
#include "GameFramework/PlayerState.h"
#include "UI/FiskSkipperPanel.h"
#include "Voyage/FiskVoyagePlan.h"

void AFiskStarterBoat::CalcCamera(float DeltaTime,FMinimalViewInfo& OutResult)
{
    Super::CalcCamera(DeltaTime,OutResult);
    const AFiskBoardingController* PC=GetWorld()?Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController()):nullptr;
    if(!PC || !PC->IsLocalController()) { return; }
    if(PC->GetSkipperPanel()->IsPhoto()) { PC->GetSkipperPanel()->PhotoCamera(GetActorTransform(),OutResult);return; }
    const int32 View=PC->GetCameraView();
    const bool Tow=CargoView.Mode==static_cast<uint8>(EFiskCargoMode::Towing);
    const FVector Eye=Tow?(View==2?FVector(-2000,-300,1350):FVector(-1750,View==1?1400:-1400,950)):View==2?FVector(-1700,-200,1000):FVector(-1250,View==1?1200:-1200,550);
    const FVector Focus=Tow?FVector(-450,0,100):View==2?FVector(-200,0,130):View==1?FVector(-250,150,120):FVector(-100,-600,110);
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
    CreateDefaultSubobject<UFiskCrewFigures>(TEXT("CrewFigures"));
    CreateDefaultSubobject<UFiskFishPresentation>(TEXT("FishPresentation"));
    CreateDefaultSubobject<UFiskJourneyAudio>(TEXT("JourneyAudio"));

    SetReplicateMovement(true);

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void AFiskStarterBoat::InitializeFleet(int32 Index,AFiskExpeditionState* E,AFiskVoyagePlan* V)
{
    if(!HasAuthority() || Index<0 || Index>=12) { return; }
    FleetIndex=Index;Expedition=E;Voyage=V;Route.Revision=Index*1000000;
    Voyage->InitializeRevision(Index*1000000);SetActorLocation(DockPosition());ForceNetUpdate();
}
void AFiskStarterBoat::SetOwnerPlayer(int32 PlayerId)
{
    if (HasAuthority() && PlayerId>=0)
    {
        OwnerId = PlayerId;
        RefreshCaptain();
    }
}

void AFiskStarterBoat::ResetForDay()
{
    if (!HasAuthority()) { return; }
    Cargo=FFiskCargoModel();CargoView=FFiskCargoView();OnRep_CargoView();bCargoReturnPending=false;
    SetActorTickEnabled(false); LegStartedMs = ReturnStartedMs = MarinaArrivalMs = -1;
    TripProgress = AtDock; ++Route.Revision;++LegSerial;TravelVotes.Reset();LegOrigin=FVector::ZeroVector;LegDestination=Route.Destination;LegTravelMs=GetTravelMs();
    PortRodActivity = StarboardRodActivity = 0;
    PortFish = FFiskFishVisualState(); StarboardFish = FFiskFishVisualState();
    RodActivities.Init(0,6);FishVisuals.Init(FFiskFishVisualState(),6);
    SetActorLocationAndRotation(DockPosition(), FRotator::ZeroRotator);
    OnRep_TripProgress(); OnRep_Route(); ForceNetUpdate();
}

void AFiskStarterBoat::Board(int32 PlayerId)
{
    if (!HasAuthority() || TripProgress != AtDock || PlayerId == INDEX_NONE || IsAboard(PlayerId) || Crew.Num() >= GetCapacity())
    {
        return;
    }

    Crew.Add(PlayerId);TravelVotes.Reset();
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
        TravelVotes.Reset();
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
    DOREPLIFETIME(AFiskStarterBoat,LegStartedMs);DOREPLIFETIME(AFiskStarterBoat,LegTravelMs);DOREPLIFETIME(AFiskStarterBoat,LegSerial);
    DOREPLIFETIME(AFiskStarterBoat,LegOrigin);DOREPLIFETIME(AFiskStarterBoat,LegDestination);DOREPLIFETIME(AFiskStarterBoat,TravelVotes);
    DOREPLIFETIME(AFiskStarterBoat,CargoView);
    DOREPLIFETIME(AFiskStarterBoat,Kind);DOREPLIFETIME(AFiskStarterBoat,RodActivities);DOREPLIFETIME(AFiskStarterBoat,FishVisuals);
    DOREPLIFETIME(AFiskStarterBoat, OwnerId);
    DOREPLIFETIME(AFiskStarterBoat,FleetIndex);DOREPLIFETIME(AFiskStarterBoat,Voyage);DOREPLIFETIME(AFiskStarterBoat,Expedition);
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
TArray<FVector> AFiskStarterBoat::GetActivePath() const
{ return IsTravelling()?FiskRouteGeometry::Between(LegOrigin,LegDestination):FiskRouteGeometry::Path(Route.Destination); }
FVector AFiskStarterBoat::PositionAt(int32 NowMs) const
{
    if(!IsTravelling()) { return GetActorLocation(); }
    return FiskRouteGeometry::Position(GetActivePath(),FMath::Clamp((NowMs-LegStartedMs)/static_cast<double>(FMath::Max(1,LegTravelMs)),0.0,1.0));
}
void AFiskStarterBoat::BeginLeg(FVector Origin,FVector Destination,int32 Duration,int32 NowMs,bool Returning)
{
    LegOrigin=Origin;LegDestination=Destination;LegStartedMs=NowMs;LegTravelMs=FMath::Max(1,Duration);++LegSerial;TravelVotes.Reset();
    ReturnStartedMs=Returning?NowMs:-1;TripProgress=Returning?101:0;bReportedDeparture=bReportedReturn=false;
    SetActorLocationAndRotation(Origin,(Destination-Origin).Rotation());OnRep_TripProgress();SetActorTickEnabled(true);ForceNetUpdate();
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_VOYAGE_LEG serial=%d returning=%d duration=%d fromX=%.0f fromY=%.0f toX=%.0f toY=%.0f"),LegSerial,Returning,LegTravelMs,Origin.X,Origin.Y,Destination.X,Destination.Y);
}
void AFiskStarterBoat::StartTrip(int32 PlayerId,int32 ExpectedRevision)
{
    if(!HasAuthority() || !CanCaptainCommand(PlayerId,ExpectedRevision)) { return; }
    const auto* Day=GetWorld()->GetGameState<AFiskDayState>();if(!Day || !Day->HasStarted()) { return; }
    const int32 Now=Day->GetElapsedMilliseconds();
    if(TripProgress==AtFishingSpot || (TripProgress>=0 && TripProgress<AtFishingSpot))
    {
        ReturnHome(Now);return;
    }
    if(TripProgress!=AtDock || Day->GetPhase(Day->GetElapsedSeconds())!=EFiskDayPhase::FishingAndReturn) { return; }
    auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if(!Mode || !Expedition || !Expedition->Depart()) { return; }
    Mode->LockBoatCrew(this);
    BeginLeg(FVector::ZeroVector,Route.Destination,GetTravelMs(),Now,false);
}
void AFiskStarterBoat::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);const auto* Day=GetWorld()->GetGameState<AFiskDayState>();if(!HasAuthority() || !Day) { return; }
    const int32 Now=Day->GetElapsedMilliseconds();if(bCargoReturnPending) { ForceCargoReturn(Now); }if(!IsTravelling()) { SetActorTickEnabled(false);return; }const auto Path=GetActivePath();if(Path.IsEmpty()) { return; }
    const double Fraction=FMath::Clamp((Now-LegStartedMs)/static_cast<double>(FMath::Max(1,LegTravelMs)),0.0,1.0);
    if(MarinaArrivalMs<0) { MarinaArrivalMs=GetMarinaArrivalMs(Now);if(MarinaArrivalMs>=0) { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MARINA_CROSSED ms=%d"),MarinaArrivalMs); } }
    const FVector Position=FiskRouteGeometry::Position(Path,Fraction),Next=FiskRouteGeometry::Position(Path,FMath::Min(1.0,Fraction+.001));
    if(!Next.Equals(Position,.01)) { SetActorRotation((Next-Position).Rotation()); }SetActorLocation(Position);
    const bool Returning=TripProgress>AtFishingSpot;const int32 Percent=FMath::FloorToInt(Fraction*100);
    TripProgress=Returning?100+FMath::Max(1,Percent):Percent;
    if(TripProgress==AtFishingSpot || TripProgress==AtMarina) { if(TripProgress==AtMarina) { SetActorLocation(DockPosition()); }SetActorTickEnabled(false);OnRep_TripProgress();ForceNetUpdate(); }
}
bool AFiskStarterBoat::UsesTravelCard() const { return IsTravelling() && FiskVoyage::Unanimous(Crew,TravelVotes); }
bool AFiskStarterBoat::VoteTravel(int32 Player,int32 ExpectedLeg,bool Vote)
{
    if(!HasAuthority() || !IsTravelling() || ExpectedLeg!=LegSerial || !Crew.Contains(Player)) { return false; }
    if(Vote) { TravelVotes.AddUnique(Player); }else { TravelVotes.Remove(Player); }ForceNetUpdate();
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_TRAVEL_VOTE player=%d votes=%d crew=%d unanimous=%d leg=%d"),Player,TravelVotes.Num(),Crew.Num(),UsesTravelCard(),LegSerial);return true;
}
bool AFiskStarterBoat::Relocate(int32 Player,int32 Revision,FVector Point,FString& Message)
{
    int32 Habitat=0,HomeMs=0;const bool Valid=FiskGrounds::MakeSeaPoint(Point,Habitat,HomeMs);
    const auto* Day=GetWorld()->GetGameState<AFiskDayState>();bool Safe=!Cargo.bMustReturn;
    // A rod with a fish on (bite, fight or ready to land) stops the move. Lines that are only out waiting
    // are pulled in for everyone when the move is accepted (owner decision 2026-11-11).
    TArray<UFiskPrototypeRod*> Waiting;
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
    {
        const auto* PC=Cast<AFiskBoardingController>(It->Get());if(!PC || !PC->PlayerState || !IsAboard(PC->PlayerState->GetPlayerId())) { continue; }
        for(int32 Slot=0;Slot<3;++Slot)
        {
            const auto State=PC->GetRodAt(Slot)->GetState();
            if(State==EFiskRodState::InitialRun || State==EFiskRodState::Fighting || State==EFiskRodState::ReadyToLand) { Safe=false; }
            else if(State==EFiskRodState::Fishing) { Waiting.Add(const_cast<UFiskPrototypeRod*>(PC->GetRodAt(Slot))); }
        }
    }
    if(!HasAuthority() || !Day || !FiskVoyage::CanRelocate(CanCaptainCommand(Player,Revision),Revision==Route.Revision,FiskVoyage::CanSteer(TripProgress),Day->GetElapsedMilliseconds(),Safe,Valid)
        || FVector::Dist2D(Point,Route.Destination)<500 || Route.Revision==MAX_int32)
    { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_RELOCATION_REJECTED player=%d safe=%d fresh=%d"),Player,Safe,Revision==Route.Revision);Message=TEXT("Flytting avvist. Kapteinen velger en ny plass før 17:00, på fiskeplassen eller på vei ut, når ingen har fisk på kroken.");return false; }
    const FVector Origin=TripProgress==AtFishingSpot?GetActorLocation():PositionAt(Day->GetElapsedMilliseconds());const auto Path=FiskRouteGeometry::Between(Origin,Point);if(Path.IsEmpty()) { Message=TEXT("Fant ingen farbar sjørute.");return false; }
    for(UFiskPrototypeRod* Rod:Waiting) { Rod->Lower();Rod->StopReel();Rod->Withdraw(); }
    const auto& G=*FiskGrounds::Find(Habitat);const int32 Base=FMath::Clamp(FMath::RoundToInt(G.TravelMs*FiskRouteGeometry::Length(Path)/G.Position.Size()),4000,150000);
    Route.GroundId=Habitat;Route.Destination=Point;Route.BaseTravelMs=HomeMs;++Route.Revision;OnRep_Route();
    BeginLeg(Origin,Point,GetKind()==1?FMath::Max(1,Base*4/5):Base,Day->GetElapsedMilliseconds(),false);
    Message=Waiting.IsEmpty()?TEXT("Flytting startet. Klokken går; R vender hjem underveis."):TEXT("Flytting startet. Ledige stenger er trukket inn; sett dem ut igjen på den nye plassen.");UE_LOG(LogTemp,Display,TEXT("FISKELAGET_RELOCATION_ACCEPTED player=%d withdrawn=%d"),Player,Waiting.Num());return true;
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
    const double Length=FiskRouteGeometry::Length(FiskRouteGeometry::Between(LegOrigin,FVector::ZeroVector));
    const int32 CrossingMs=FiskVoyage::Crossing(ReturnStartedMs,LegTravelMs,Length);
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
    if(FiskVoyage::CanSteer(TripProgress)) { return Relocate(Requester,Revision,Point,Message); }
    int32 Habitat=0,Duration=0;
    if(!FiskGrounds::MakeSeaPoint(Point,Habitat,Duration)) { Message=TEXT("Velg sjø innenfor kartet, utenfor øyer og marina.");return false; }
    if(!SelectGround(Requester,Revision,Habitat,Message)) { return false; }
    Route.Destination=Point;Route.BaseTravelMs=Duration;ForceNetUpdate();
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_SEA_POINT x=%.0f y=%.0f habitat=%d travel=%d"),Point.X,Point.Y,Habitat,GetTravelMs());return true;
}
int32 AFiskStarterBoat::ReturnMarginMs(int32 NowMs) const
{
    int64 Home=NowMs;const int64 Travel=GetReturnTravelMs();
    if(TripProgress==AtDock) { Home=FMath::Max(NowMs,120000)+2*Travel; }
    else if(TripProgress<100) { Home+=FMath::DivideAndRoundUp<int64>((100LL-TripProgress)*LegTravelMs,100)+Travel; }
    else if(TripProgress==AtFishingSpot) { Home+=Travel; }
    else if(TripProgress<200) { Home+=FMath::DivideAndRoundUp<int64>((TripProgress==101?100LL:200LL-TripProgress)*LegTravelMs,100); }
    return static_cast<int32>(FMath::Clamp<int64>(1020000-Home,MIN_int32,MAX_int32));
}
void AFiskStarterBoat::SetHolderVisuals(const TArray<uint8>& Activities,const TArray<FFiskFishVisualState>& Fish)
{
    if(!HasAuthority() || Activities.Num()!=6 || Fish.Num()!=6) { return; }
    bool Changed=RodActivities!=Activities;
    for(int32 I=0;I<6;++I) { Changed|=FishVisuals[I]!=Fish[I]; }
    if(Changed) { RodActivities=Activities;FishVisuals=Fish;ForceNetUpdate(); }
}

void AFiskStarterBoat::AcceptCargo(const FFiskCargoModel& Candidate)
{
    if(!HasAuthority()) { return; }Cargo=Candidate;CargoView=Cargo.View();OnRep_CargoView();bCargoReturnPending=Cargo.bMustReturn;ForceNetUpdate();if(bCargoReturnPending) { SetActorTickEnabled(true); }
}
void AFiskStarterBoat::RevealCargo()
{ if(HasAuthority()) { CargoView=Cargo.View(true);OnRep_CargoView();ForceNetUpdate(); } }
bool AFiskStarterBoat::ReturnHome(int32 Now)
{
    const FVector Origin=PositionAt(Now);const auto Path=FiskRouteGeometry::Between(Origin,FVector::ZeroVector);if(Path.IsEmpty()) { return false; }
    const double HomeLength=FiskRouteGeometry::Length(FiskRouteGeometry::Path(Route.Destination));
    const int32 Duration=TripProgress==AtFishingSpot?GetReturnTravelMs():FMath::Max(1,FMath::RoundToInt(GetReturnTravelMs()*FiskRouteGeometry::Length(Path)/FMath::Max(1.0,HomeLength)));
    BeginLeg(Origin,FVector::ZeroVector,Duration,Now,true);return true;
}
void AFiskStarterBoat::ForceCargoReturn(int32 Now)
{
    bCargoReturnPending=false;if(TripProgress!=AtFishingSpot || !ReturnHome(Now)) { return; }
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
    {
        auto* PC=Cast<AFiskBoardingController>(It->Get());if(!PC || !PC->PlayerState || !IsAboard(PC->PlayerState->GetPlayerId())) { continue; }
        for(int32 Slot=0;Slot<3;++Slot) { auto* Rod=const_cast<UFiskPrototypeRod*>(PC->GetRodAt(Slot));Rod->Lower();Rod->StopReel();Rod->Withdraw(); }
        PC->ClientActionFeedback(Cargo.TowedGrams>0?TEXT("Stor fangst er sikret på slep. Alle stenger er trukket inn; båten går hjem med 65 prosent fart."):TEXT("Båtens fangstlager er fullt eller utilstrekkelig. Alle stenger er trukket inn; automatisk hjemtur."));
    }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CARGO_RETURN mode=%d duration=%d"),CargoView.Mode,LegTravelMs);
}

void AFiskStarterBoat::OnRep_CargoView()
{ UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CARGO_VIEW mode=%d revealed=%d"),CargoView.Mode,CargoView.RevealedGrams>=0); }
