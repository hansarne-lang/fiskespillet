#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Bootstrap/FiskBootstrapHUD.h"
#include "DayCycle/FiskDayState.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Engine/World.h"
#include "Economy/FiskDaySettlement.h"
#include "Boats/FiskMarinaZone.h"
#include "World/FiskOceanScene.h"
#include "World/FiskMarinaHub.h"
#include "GameFramework/PlayerState.h"
#include "DayCycle/FiskSessionRules.h"
#include "GameFramework/GameSession.h"

AFiskBootstrapGameMode::AFiskBootstrapGameMode()
{
    HUDClass = AFiskBootstrapHUD::StaticClass();
    DefaultPawnClass = nullptr;
    GameStateClass = AFiskDayState::StaticClass();
    PlayerControllerClass = AFiskBoardingController::StaticClass();
}

void AFiskBootstrapGameMode::StartPlay()
{
    Super::StartPlay();
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_BOOTSTRAP_READY"));
}

void AFiskBootstrapGameMode::InitGameState()
{
    Super::InitGameState();
    if(GameSession) { GameSession->MaxPlayers=4; }
    StarterBoat = GetWorld()->SpawnActor<AFiskStarterBoat>();
    Settlement = GetWorld()->SpawnActor<AFiskDaySettlement>();
    GetWorld()->SpawnActor<AFiskMarinaZone>();
    GetWorld()->SpawnActor<AFiskOceanScene>();
    GetWorld()->SpawnActor<AFiskMarinaHub>();
}

void AFiskBootstrapGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (StarterBoat && NewPlayer->IsLocalController() && NewPlayer->PlayerState)
    {
        StarterBoat->SetOwnerPlayer(NewPlayer->PlayerState->GetPlayerId());
    }

    // The main menu is standalone. A playable listen-server session starts
    // with its first player; StartDay is idempotent when friends join later.
    if (GetNetMode() != NM_Standalone && GetNumPlayers() >= 1)
    {
        if (AFiskDayState* Day = GetGameState<AFiskDayState>())
        {
            Day->StartDay();
        }
    }
}

void AFiskBootstrapGameMode::Logout(AController* Exiting)
{
    if (StarterBoat && Exiting->PlayerState)
    {
        StarterBoat->Leave(Exiting->PlayerState->GetPlayerId());
    }

    Super::Logout(Exiting);
}

bool AFiskBootstrapGameMode::StartNextDay(APlayerController* Requester, int32 ExpectedDay)
{
    AFiskDayState* Day = GetGameState<AFiskDayState>();
    const bool Host = Requester && Requester->IsLocalController() && GetNetMode() == NM_ListenServer;
    if (!Day || !StarterBoat || !Settlement || !FiskSession::CanAdvance(Host, Day->GetDayNumber(), ExpectedDay,
        Day->GetElapsedMilliseconds(), Settlement->GetView().bRevealed)) { return false; }
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
    { const AFiskBoardingController* P=Cast<AFiskBoardingController>(It->Get());if(P && (!P->IsCareerSynced() || !P->GetServerCareer().Receipts.ContainsByPredicate([&](const FFiskCareerReceipt& R){return R.Id==Settlement->GetView().DayId && R.Kind==2;}))) { return false; } }
    // All resets run in this one server frame before the fresh clock starts.
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    { if (AFiskBoardingController* PC = Cast<AFiskBoardingController>(It->Get())) { PC->ResetRodForDay(ExpectedDay+1); } }
    DayProfiles.Reset();
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
    { if(const AFiskBoardingController* P=Cast<AFiskBoardingController>(It->Get());P && P->PlayerState && P->IsCareerReady()) { BindCareer(P->PlayerState->GetPlayerId(),P->GetServerCareer().Id); } }
    StarterBoat->ResetForDay(); Settlement->ResetForDay(ExpectedDay + 1);
    return Day->AdvanceDay();
}
int64 AFiskBootstrapGameMode::CareerIncome(const FGuid& Profile) const
{
    int64 Sum=0;if(!Settlement || !Settlement->GetView().bRevealed) { return -1; }
    for(const FFiskDayResult& Row:Settlement->GetView().Results)
    { const FGuid* BoundProfile=DayProfiles.Find(Row.PlayerId);if(BoundProfile && *BoundProfile==Profile) { Sum+=static_cast<int64>(Row.SessionBalance)*100; } }
    return Sum;
}
