#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FiskFleetMembership.h"
#include "Economy/Betting/FiskBetting.h"
#include "FiskBootstrapGameMode.generated.h"

class AFiskStarterBoat;
class AFiskDaySettlement;
class AFiskExpeditionState;
class AFiskBoardingController;

UCLASS()
class AFiskBootstrapGameMode final : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFiskBootstrapGameMode();
    virtual void StartPlay() override;
    virtual void InitGameState() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    AFiskStarterBoat* GetStarterBoat() const { return StarterBoat; }
    AFiskStarterBoat* GetBoatForPlayer(int32 Player) const;
    AFiskStarterBoat* CreateOwnedBoat(AFiskBoardingController* PC);
    void RegisterFleetCareer(int32 Player,const FGuid& Profile);
    bool CanJoinFleet(const FGuid& Profile) const { return FleetMembership.CanJoin(Profile); }
    void LockBoatCrew(const AFiskStarterBoat* Boat);
    const TArray<TObjectPtr<AFiskStarterBoat>>& GetFleet() const { return Fleet; }
    AFiskDaySettlement* GetSettlement() const { return Settlement; }
    bool IsWeighInStarted() const;   // The shared weigh-in (#183) closes departures and bets for the day
    AFiskExpeditionState* GetExpedition() const { return Expedition; }
    bool StartNextDay(APlayerController* Requester, int32 ExpectedDay);
    void BindCareer(int32 Player,const FGuid& Profile) { DayProfiles.Add(Player,Profile); }
    int64 CareerIncome(const FGuid& Profile) const;
    bool ArchiveConfirmedDay();
    int32 GetArchiveRevision() const { return ArchiveRevision; }
    const TMap<int32,FGuid>& GetDayProfiles() const { return DayProfiles; }

    // Beach bar bets (#22 step 2), FiskBootstrapBetting.cpp. The book lives here for one day; payouts
    // wait in UnclaimedBets until the bettor's controller has credited and saved them (rule 8).
    bool IsAtSea(int32 Player) const;
    bool AreBetsOpen() const;
    EFiskBetResult PlaceBet(int32 Player,const FGuid& Bettor,const FGuid& Bet,const FGuid& Fisher,int64 Stake,TFunctionRef<bool()> Charge);
    void SettleBets();
    TArray<FFiskBetPayout> UnclaimedBetPayouts(const FGuid& Bettor) const;
    void ClaimBetPayout(const FGuid& Receipt) { UnclaimedBets.Remove(Receipt); }
    bool IsBetPending(const FGuid& Bet) const;
    void BetBoard(const FGuid& Bettor,FGuid& Fisher,int64& Stake,int64& Payout) const;
    int64 BetPot() const { return BetBook.Pot(); }
    int32 BetCount() const { return BetBook.GetBets().Num(); }
    bool AreBetsSettled() const { return bBetsSettled; }
    const FFiskBetSummary& GetBetSummary() const { return BetSummary; }   // Public after 17:00 (#18)

private:
    AFiskStarterBoat* SpawnFleetBoat(int32 Player);
    UPROPERTY() TArray<TObjectPtr<AFiskStarterBoat>> Fleet;
    TMap<FGuid,int32> OwnedFleet;
    FFiskFleetMembership FleetMembership;
    UPROPERTY() TObjectPtr<AFiskExpeditionState> Expedition;
    TMap<int32,FGuid> DayProfiles;
    FFiskBetBook BetBook;
    TSet<FGuid> BetFishers;                    // Careers out at sea when a bet was registered (2026-11-10)
    TMap<FGuid,FFiskBetPayout> UnclaimedBets;  // By payout receipt; kept across days until claimed
    TMap<FGuid,int64> BetResults;              // Today's payout per bettor, for the bar board
    bool bBetsSettled=false;
    FFiskBetSummary BetSummary;
    void ResetBetsForDay();
    FGuid ArchivedDay;
    int32 ArchiveRevision=0;
    double ArchiveRetryAt=0;
    UPROPERTY()
    TObjectPtr<AFiskStarterBoat> StarterBoat;
    UPROPERTY()
    TObjectPtr<AFiskDaySettlement> Settlement;
};
