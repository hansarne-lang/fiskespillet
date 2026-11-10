#include "Boats/Controller/FiskRecoverySync.h"
#include "Boats/FiskBoardingController.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Career/FiskJournalAuthority.h"
#include "UI/FiskGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Misc/Paths.h"
#include "Expedition/FiskExpeditionStore.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

UFiskRecoverySync::UFiskRecoverySync() { SetIsReplicatedByDefault(true); }
AFiskBoardingController& UFiskRecoverySync::Controller() const { return *CastChecked<AFiskBoardingController>(GetOwner()); }

void UFiskRecoverySync::TickJournalRecovery()
{
    AFiskBoardingController& PC=Controller();
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();if(!Mode || !PC.IsCareerReady()) { return; }
    const FFiskCareerProfile& ServerCareer=PC.GetServerCareer();const FString Folder=FPaths::GetPath(PC.CareerAuthorityFile());
    if(RecoveryRevision!=Mode->GetArchiveRevision())
    {
        FFiskJournalModel Snapshot;
        if(!FiskJournalAuthority::Load(Folder,ServerCareer.Id,Snapshot)) { PC.ClientActionFeedback(TEXT("Vertens journalarkiv kan ikke leses. Data er bevart; ny dag kan vente på reparasjon."));return; }
        TArray<FFiskExpeditionRecord> Medals;
        if(!FiskExpeditionStore::Load(FiskExpeditionStore::File(Folder,ServerCareer.Id),ServerCareer.Id,Medals)) { return; }
        ExpeditionSnapshot=MoveTemp(Medals);ExpeditionOffset=0;bExpeditionAwaiting=false;
        RecoverySnapshot=MoveTemp(Snapshot);RecoveryRevision=Mode->GetArchiveRevision();RecoveryOffset=0;bRecoveryAwaiting=false;bRecoveryReported=false;
    }
    const TArray<FFiskJournalEntry>& Entries=RecoverySnapshot.GetEntries();
    if(RecoveryOffset>=Entries.Num())
    {
        if(!bRecoveryReported) { bRecoveryReported=true;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_SYNCED player=%d count=%d"),PC.PlayerState?PC.PlayerState->GetPlayerId():-1,Entries.Num()); }TickExpeditionRecovery();return;
    }
    if(!bRecoveryAwaiting) { ++RecoverySerial;RecoverySentCount=FMath::Min(FiskJournalAuthority::BatchSize,Entries.Num()-RecoveryOffset);bRecoveryAwaiting=true; }
    TArray<FFiskJournalEntry> Batch;Batch.Append(Entries.GetData()+RecoveryOffset,RecoverySentCount);ClientJournalBatch(ServerCareer.Id,RecoverySerial,Batch);
}
void UFiskRecoverySync::ClientJournalBatch_Implementation(FGuid Profile,int32 Serial,const TArray<FFiskJournalEntry>& Entries)
{
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    if(Profile!=GI->Career.Id || Serial<1 || Entries.Num()<1 || Entries.Num()>FiskJournalAuthority::BatchSize) { return; }
    if(Serial==ClientRecoverySerial) { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_BATCH_REPLAY serial=%d"),Serial);ServerJournalAck(Profile,Serial);return; }
    if(Serial<ClientRecoverySerial || !GI->AcceptRecoveredJournal(Profile,Entries)) { return; }
    ClientRecoverySerial=Serial;
#if WITH_DEV_AUTOMATION_TESTS
    if(!bTestJournalAckDropped && FParse::Param(FCommandLine::Get(),TEXT("FiskDropJournalAck")))
    { bTestJournalAckDropped=true;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_TEST_JOURNAL_ACK_DROPPED serial=%d"),Serial); }
    else
#endif
    { ServerJournalAck(Profile,Serial); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_BATCH_SAVED serial=%d records=%d total=%d"),Serial,Entries.Num(),GI->GetJournal().GetEntries().Num());
}
void UFiskRecoverySync::ServerJournalAck_Implementation(FGuid Profile,int32 Serial)
{
    const AFiskBoardingController& PC=Controller();
    if(!PC.IsCareerReady() || Profile!=PC.GetServerCareer().Id || !bRecoveryAwaiting || Serial!=RecoverySerial)
    { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_ACK_REJECTED"));return; }
    RecoveryOffset+=RecoverySentCount;bRecoveryAwaiting=false;
}
void UFiskRecoverySync::TickExpeditionRecovery()
{
    if(ExpeditionOffset>=ExpeditionSnapshot.Num()) { return; }
    if(!bExpeditionAwaiting) { ++ExpeditionSerial;ExpeditionSentCount=FMath::Min(16,ExpeditionSnapshot.Num()-ExpeditionOffset);bExpeditionAwaiting=true; }
    TArray<FFiskExpeditionRecord> Batch;Batch.Append(ExpeditionSnapshot.GetData()+ExpeditionOffset,ExpeditionSentCount);ClientExpeditionBatch(Controller().GetServerCareer().Id,ExpeditionSerial,Batch);
}
void UFiskRecoverySync::ClientExpeditionBatch_Implementation(FGuid Profile,int32 Serial,const TArray<FFiskExpeditionRecord>& Entries)
{
    auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());if(Profile!=GI->Career.Id || Serial<1 || Entries.IsEmpty() || Entries.Num()>16) { return; }
    if(Serial==ClientExpeditionSerial) { ServerExpeditionAck(Profile,Serial);return; }
    if(Serial<ClientExpeditionSerial || !GI->AcceptExpeditions(Profile,Entries)) { return; }
    ClientExpeditionSerial=Serial;ServerExpeditionAck(Profile,Serial);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPEDITION_BATCH_SAVED count=%d total=%d"),Entries.Num(),GI->Expeditions.Num());
}
void UFiskRecoverySync::ServerExpeditionAck_Implementation(FGuid Profile,int32 Serial)
{
    const AFiskBoardingController& PC=Controller();
    if(!PC.IsCareerReady() || Profile!=PC.GetServerCareer().Id || !bExpeditionAwaiting || Serial!=ExpeditionSerial)
    { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPEDITION_ACK_REJECTED"));return; }
    ExpeditionOffset+=ExpeditionSentCount;bExpeditionAwaiting=false;
}
