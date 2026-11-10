#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/FiskJournalModel.h"
#include "Expedition/FiskExpeditionModel.h"
#include "FiskRecoverySync.generated.h"
class AFiskBoardingController;

// Sends the host's confirmed journal and expedition medals to the owning client in acknowledged
// batches after a connect or a new archived day (#113 step 4). Moved from AFiskBoardingController
// with the same network calls; the component is replicated and owned by the player's controller.
UCLASS()
class UFiskRecoverySync final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskRecoverySync();
    void TickJournalRecovery();   // Server, from the controller's career tick
    UFUNCTION(Client,Reliable) void ClientJournalBatch(FGuid Profile,int32 Serial,const TArray<FFiskJournalEntry>& Entries);
    UFUNCTION(Server,Reliable) void ServerJournalAck(FGuid Profile,int32 Serial);
    UFUNCTION(Client,Reliable) void ClientExpeditionBatch(FGuid Profile,int32 Serial,const TArray<FFiskExpeditionRecord>& Entries);
    UFUNCTION(Server,Reliable) void ServerExpeditionAck(FGuid Profile,int32 Serial);
private:
    AFiskBoardingController& Controller() const;
    void TickExpeditionRecovery();
    FFiskJournalModel RecoverySnapshot;
    TArray<FFiskExpeditionRecord> ExpeditionSnapshot;
    int32 ExpeditionOffset=0,ExpeditionSerial=0,ExpeditionSentCount=0,ClientExpeditionSerial=0;
    bool bExpeditionAwaiting=false;
    int32 RecoveryRevision=-1,RecoveryOffset=0,RecoverySerial=0,RecoverySentCount=0,ClientRecoverySerial=0;
    bool bRecoveryAwaiting=false,bRecoveryReported=false;
#if WITH_DEV_AUTOMATION_TESTS
    bool bTestJournalAckDropped=false;   // -FiskDropJournalAck drops the first acknowledgement once
#endif
};
