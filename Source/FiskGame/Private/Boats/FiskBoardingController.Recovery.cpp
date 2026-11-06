#include "Boats/FiskBoardingController.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Career/FiskJournalAuthority.h"
#include "UI/FiskGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Misc/Paths.h"

void AFiskBoardingController::TickJournalRecovery()
{
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();if(!Mode || !bCareerReady) { return; }
    if(RecoveryRevision!=Mode->GetArchiveRevision())
    {
        FFiskJournalModel Snapshot;
        if(!FiskJournalAuthority::Load(FPaths::GetPath(CareerAuthorityFile()),ServerCareer.Id,Snapshot)) { ClientActionFeedback(TEXT("Vertens journalarkiv kan ikke leses. Data er bevart; ny dag kan vente på reparasjon."));return; }
        RecoverySnapshot=MoveTemp(Snapshot);RecoveryRevision=Mode->GetArchiveRevision();RecoveryOffset=0;bRecoveryAwaiting=false;bRecoveryReported=false;
    }
    const TArray<FFiskJournalEntry>& Entries=RecoverySnapshot.GetEntries();
    if(RecoveryOffset>=Entries.Num())
    {
        if(!bRecoveryReported) { bRecoveryReported=true;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_SYNCED player=%d count=%d"),PlayerState?PlayerState->GetPlayerId():-1,Entries.Num()); }return;
    }
    if(!bRecoveryAwaiting) { ++RecoverySerial;RecoverySentCount=FMath::Min(FiskJournalAuthority::BatchSize,Entries.Num()-RecoveryOffset);bRecoveryAwaiting=true; }
    TArray<FFiskJournalEntry> Batch;Batch.Append(Entries.GetData()+RecoveryOffset,RecoverySentCount);ClientJournalBatch(ServerCareer.Id,RecoverySerial,Batch);
}
void AFiskBoardingController::ClientJournalBatch_Implementation(FGuid Profile,int32 Serial,const TArray<FFiskJournalEntry>& Entries)
{
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetGameInstance());
    if(Profile!=GI->Career.Id || Serial<1 || Entries.Num()<1 || Entries.Num()>FiskJournalAuthority::BatchSize) { return; }
    if(Serial==ClientRecoverySerial) { ServerJournalAck(Profile,Serial);return; }
    if(Serial<ClientRecoverySerial || !GI->AcceptRecoveredJournal(Profile,Entries)) { return; }
    ClientRecoverySerial=Serial;ServerJournalAck(Profile,Serial);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_BATCH_SAVED serial=%d records=%d total=%d"),Serial,Entries.Num(),GI->GetJournal().GetEntries().Num());
}
void AFiskBoardingController::ServerJournalAck_Implementation(FGuid Profile,int32 Serial)
{
    if(!bCareerReady || Profile!=ServerCareer.Id || !bRecoveryAwaiting || Serial!=RecoverySerial)
    { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_ACK_REJECTED"));return; }
    RecoveryOffset+=RecoverySentCount;bRecoveryAwaiting=false;
}
