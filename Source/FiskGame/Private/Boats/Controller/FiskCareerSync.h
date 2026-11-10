#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskCareerProfile.h"
#include "FiskCareerSync.generated.h"
class AFiskBoardingController;

// What the beach bar shows of the server's bet book (#22 step 2): own bet, pot and own result.
struct FFiskBetBoardView { FGuid Fisher;int64 Stake=0,Pot=0,Payout=-1;int32 Count=0;bool bOpen=false,bKnown=false;FString Message; };

// The player's career on the host: hello and snapshots, local save acknowledgements, the shop, boat
// choice, the 17:00 settlement and beach bar bets (#113 step 5). Moved from AFiskBoardingController
// with the same network calls; the component is replicated and owned by the player's controller.
UCLASS()
class UFiskCareerSync final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskCareerSync();
    void TickSync();             // Every 0.5 s on server and owning client, from the controller's career timer
    void RetryLocalSave();       // F5 in the career panel
    bool IsReady() const { return bCareerReady; }
    bool IsSynced() const { return bCareerReady && CareerAck>=ServerCareer.Revision; }
    const FFiskCareerProfile& GetServerCareer() const { return ServerCareer; }
    FString AuthorityFile() const;   // Host copy of this player's career; its folder holds the journal archive
    const FFiskBetBoardView& GetBetBoard() const { return BetBoard; }
    UFUNCTION(Server,Reliable) void ServerCareerHello(FFiskCareerProfile Profile);
    UFUNCTION(Client,Reliable) void ClientCareerSnapshot(FFiskCareerProfile Profile,const FString& Message);
    UFUNCTION(Server,Reliable) void ServerCareerAck(FGuid Profile,int32 Revision);
    UFUNCTION(Server,Reliable) void ServerBuyCareer(FGuid Command,int32 Revision,int32 Item);
    UFUNCTION(Server,Reliable) void ServerChooseBoat(int32 Revision,int32 Kind);
    // Beach bar bets (#22 step 2). The server owns the book; the client only sees its own bet and the pot.
    UFUNCTION(Server,Reliable) void ServerPlaceBet(FGuid Bet,FGuid Fisher,int64 Stake);
    UFUNCTION(Server,Reliable) void ServerBetBoard();
    UFUNCTION(Client,Reliable) void ClientBetBoard(FGuid Fisher,int64 Stake,int64 Pot,int32 Count,int64 Payout,bool bOpen,const FString& Message);
private:
    AFiskBoardingController& PC() const;
    bool SaveServerCareer(const FFiskCareerProfile& Candidate,const FString& Message);
    void SendBetBoard(const FString& Message);
    void ClaimBetPayouts();
    void RefundOrphanBets(FFiskCareerProfile& Profile) const;
    FFiskCareerProfile ServerCareer,PendingCareer;
    FFiskBetBoardView BetBoard;
    int32 CareerAck=0;
    bool bCareerReady=false,bCareerHelloSent=false;
    double CareerRetryAt=0;
};
