#include "Boats/Controller/FiskCareerSync.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/Controller/FiskRecoverySync.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Career/FiskCareerStore.h"
#include "Career/FiskJournalAuthority.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskUserSettings.h"
#include "Economy/FiskDaySettlement.h"
#include "DayCycle/FiskDayState.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Misc/Paths.h"

UFiskCareerSync::UFiskCareerSync() { SetIsReplicatedByDefault(true); }
AFiskBoardingController& UFiskCareerSync::PC() const { return *CastChecked<AFiskBoardingController>(GetOwner()); }
void UFiskCareerSync::RetryLocalSave()
{
    // F5 in the career panel: save the pending snapshot again, or say hello again if the host never confirmed.
    AFiskBoardingController& Owner=PC();UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(Owner.GetGameInstance());
    if(GI->AcceptCareer(PendingCareer.IsValid()?PendingCareer:GI->Career))
    {
        if(PendingCareer.IsValid()) { if(!Owner.HasAuthority()) { bCareerReady=true; }ServerCareerAck(PendingCareer.Id,PendingCareer.Revision);PendingCareer=FFiskCareerProfile(); }
        else if(!bCareerReady) { bCareerHelloSent=false; }
    }
}
FString UFiskCareerSync::AuthorityFile() const
{ return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Authority"),ServerCareer.Id.ToString(EGuidFormats::Digits)+TEXT(".ini")); }
bool UFiskCareerSync::SaveServerCareer(const FFiskCareerProfile& P,const FString& Message)
{
    AFiskBoardingController& Owner=PC();
    if(!P.IsValid() || !FFiskCareerStore::Save(AuthorityFile(),P))
    { Owner.ClientActionFeedback(TEXT("Verten kunne ikke lagre karrieren. Ingen endring bekreftet; prøv igjen."));return false; }
    ServerCareer=P;ClientCareerSnapshot(ServerCareer,Message);return true;
}
void UFiskCareerSync::ServerCareerHello_Implementation(FFiskCareerProfile P)
{
    AFiskBoardingController& Owner=PC();
    if(!Owner.PlayerState || !P.IsValid()) { Owner.ClientActionFeedback(TEXT("Ugyldig karriere. Eksisterende filer er bevart."));return; }
    if(bCareerReady) { ClientCareerSnapshot(ServerCareer,TEXT("Karriere er allerede tilkoblet."));return; }
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
    { const AFiskBoardingController* Other=Cast<AFiskBoardingController>(It->Get());if(Other && Other!=&Owner && Other->IsCareerReady() && Other->GetServerCareer().Id==P.Id)
        { Owner.ClientActionFeedback(TEXT("Denne karrieren er allerede tilkoblet. Bruk en egen profil per spiller."));return; } }
    ServerCareer=P;FFiskCareerProfile Known;const EFiskCareerLoad Status=FFiskCareerStore::Load(AuthorityFile(),Known);
    if(Status==EFiskCareerLoad::Blocked || ((Status==EFiskCareerLoad::Loaded || Status==EFiskCareerLoad::Recovered) && !P.Extends(Known) && !Known.Extends(P)))
    { ServerCareer=FFiskCareerProfile();Owner.ClientActionFeedback(TEXT("Vertens profilkopi er skadet eller i konflikt. Filene er bevart."));return; }
    if(Known.IsValid() && Known.Revision>P.Revision) { P=Known; }
    if(!FiskJournalAuthority::RecoverIncome(FPaths::GetPath(AuthorityFile()),P)) { Owner.ClientActionFeedback(TEXT("Vertens bekreftede journal/inntekt kan ikke leses. Eksisterende data er bevart."));return; }
    RefundOrphanBets(P);
    if(!SaveServerCareer(P,TEXT("Karriere bekreftet av verten."))) { return; }
    bCareerReady=true;
    if(auto* M=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>()) { M->RegisterFleetCareer(Owner.PlayerState->GetPlayerId(),P.Id); }
    if(AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>()) { Mode->BindCareer(Owner.PlayerState->GetPlayerId(),P.Id); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_READY player=%d revision=%d cash=%lld owned=%d"),Owner.PlayerState->GetPlayerId(),P.Revision,static_cast<long long>(P.Cash),P.Owned);
}
void UFiskCareerSync::ClientCareerSnapshot_Implementation(FFiskCareerProfile P,const FString& Message)
{
    AFiskBoardingController& Owner=PC();
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(Owner.GetGameInstance());
    if(!P.IsValid()) { return; }PendingCareer=P;
    if(GI->AcceptCareer(P)) { PendingCareer=FFiskCareerProfile();if(!Owner.HasAuthority()) { bCareerReady=true; }ServerCareerAck(P.Id,P.Revision); }
    Owner.ClientActionFeedback_Implementation(Message+TEXT(" ")+GI->CareerMessage);
}
void UFiskCareerSync::ServerCareerAck_Implementation(FGuid Profile,int32 Revision)
{
    AFiskBoardingController& Owner=PC();
    if(ServerCareer.Id==Profile && ServerCareer.Revision==Revision)
    { CareerAck=Revision;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_SAVED player=%d revision=%d"),Owner.PlayerState?Owner.PlayerState->GetPlayerId():-1,Revision); }
}
void UFiskCareerSync::TickSync()
{
    AFiskBoardingController& Owner=PC();
    const double Now=GetWorld()->GetRealTimeSeconds();
    if(Owner.IsLocalController() && !bCareerHelloSent && GetNetMode()!=NM_Standalone)
    {
        const UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(Owner.GetGameInstance());
        if(!GI->bCareerBlocked && GI->Career.IsValid()) { bCareerHelloSent=true;ServerCareerHello(GI->Career); }
    }
    if(Now<CareerRetryAt) { return; }CareerRetryAt=Now+2;
    if(Owner.IsLocalController() && PendingCareer.IsValid())
    { UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(Owner.GetGameInstance());if(GI->AcceptCareer(PendingCareer)) { if(!Owner.HasAuthority()) { bCareerReady=true; }ServerCareerAck(PendingCareer.Id,PendingCareer.Revision);PendingCareer=FFiskCareerProfile(); } }
    if(!Owner.HasAuthority() || !bCareerReady) { return; }
    Owner.GetRecoverySync()->TickJournalRecovery();
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();const AFiskDaySettlement* Settlement=Mode?Mode->GetSettlement():nullptr;
    if(Day && Settlement && Settlement->GetView().bRevealed)
    {
        const FGuid Id=Settlement->GetView().DayId;
        if(!ServerCareer.Receipts.ContainsByPredicate([&](const FFiskCareerReceipt& R){return R.Id==Id;}))
        {
            FFiskCareerProfile P=ServerCareer;const int64 Income=Mode->CareerIncome(P.Id);
            if(P.Credit(Id,Income) && SaveServerCareer(P,TEXT("17:00: dagens kvalifiserte fangst er bokført.")))
            { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_SETTLED player=%d amount=%lld revision=%d cash=%lld"),Owner.PlayerState->GetPlayerId(),static_cast<long long>(Income),P.Revision,static_cast<long long>(P.Cash)); }
            else { Owner.ClientActionFeedback(TEXT("Karriereoppgjør venter på lagring eller ledig kvitteringskapasitet. Neste dag er sperret."));CareerAck=0; }
        }
    }
    if(Mode && Day) { Mode->SettleBets(); }   // SettleBets waits for the end of the weigh-in (#183)
    ClaimBetPayouts();
    if(CareerAck<ServerCareer.Revision) { ClientCareerSnapshot(ServerCareer,TEXT("Venter på lokal lagringsbekreftelse.")); }
}
void UFiskCareerSync::ServerPlaceBet_Implementation(FGuid Bet,FGuid Fisher,int64 Stake)
{
    AFiskBoardingController& Owner=PC();
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    EFiskBetResult Result=EFiskBetResult::Closed;
    if(bCareerReady && Owner.PlayerState && Mode)
    {
        Result=Mode->PlaceBet(Owner.PlayerState->GetPlayerId(),ServerCareer.Id,Bet,Fisher,Stake,[&]()
            { FFiskCareerProfile P=ServerCareer;return P.Stake(Bet,Stake) && SaveServerCareer(P,TEXT("Veddemålet er registrert. Innsatsen er trukket.")); });
    }
    FString Message;
    switch(Result)
    {
    case EFiskBetResult::Registered: Message=TEXT("Veddemålet er registrert. Resultatet kommer ved 17:00.");break;
    case EFiskBetResult::AlreadyRegistered: Message=TEXT("Veddemålet var allerede registrert. Ingen ny innsats er trukket.");break;
    case EFiskBetResult::InvalidStake: Message=TEXT("Innsatsen må være $50, $100 eller $250.");break;
    case EFiskBetResult::AlreadyBetToday: Message=TEXT("Du har allerede et veddemål i dag.");break;
    case EFiskBetResult::SelfBet: Message=TEXT("Du kan ikke satse på deg selv.");break;
    case EFiskBetResult::Conflict: Message=TEXT("Veddemålet er i konflikt med et registrert veddemål. Ingen endring.");break;
    case EFiskBetResult::NotAshore: Message=TEXT("Veddemål settes i strandbaren når du er i land.");break;
    case EFiskBetResult::FisherNotAtSea: Message=TEXT("Den fiskeren er ikke ute på sjøen nå.");break;
    case EFiskBetResult::NotCharged: Message=TEXT("Innsatsen kunne ikke trekkes (saldo, kvitteringer eller lagring). Ingen endring.");break;
    default: Message=TEXT("Veddemål er stengt: dagen har ikke startet eller klokka har passert 17:00.");break;
    }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BET_REPLY player=%d result=%d stake=%lld cash=%lld"),Owner.PlayerState?Owner.PlayerState->GetPlayerId():-1,static_cast<int32>(Result),static_cast<long long>(Stake),static_cast<long long>(ServerCareer.Cash));
    SendBetBoard(Message);
}
void UFiskCareerSync::ServerBetBoard_Implementation() { if(bCareerReady) { SendBetBoard(FString()); } }
void UFiskCareerSync::SendBetBoard(const FString& Message)
{
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();if(!Mode) { return; }
    FGuid Fisher;int64 Stake=0,Payout=-1;Mode->BetBoard(ServerCareer.Id,Fisher,Stake,Payout);
    ClientBetBoard(Fisher,Stake,Mode->BetPot(),Mode->BetCount(),Payout,Mode->AreBetsOpen(),Message);
    if(Mode->AreBetsSettled())
    { const FFiskBetSummary& S=Mode->GetBetSummary();ClientBetSummary(S.Pot,S.Bets,S.bRefund,S.WinningFishers,S.WinningGrams,S.Winners); }
}
void UFiskCareerSync::ClientBetBoard_Implementation(FGuid Fisher,int64 Stake,int64 Pot,int32 Count,int64 Payout,bool bOpen,const FString& Message)
{
    if(bOpen) { BetBoard.bSettled=false;BetBoard.WinningFishers.Reset(); }
    BetBoard.Fisher=Fisher;BetBoard.Stake=Stake;BetBoard.Pot=Pot;BetBoard.Count=Count;BetBoard.Payout=Payout;BetBoard.bOpen=bOpen;BetBoard.bKnown=true;
    if(!Message.IsEmpty()) { BetBoard.Message=Message; }
}
void UFiskCareerSync::ClientBetSummary_Implementation(int64 Pot,int32 Bets,bool bRefund,const TArray<FGuid>& WinningFishers,int32 WinningGrams,int32 Winners)
{
    BetBoard.bSettled=true;BetBoard.Pot=Pot;BetBoard.Count=Bets;BetBoard.bRefund=bRefund;BetBoard.WinningFishers=WinningFishers;BetBoard.WinningGrams=WinningGrams;BetBoard.Winners=Winners;
}
void UFiskCareerSync::ClaimBetPayouts()
{
    AFiskBoardingController& Owner=PC();
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();if(!Mode || !bCareerReady) { return; }
    const TArray<FFiskBetPayout> Due=Mode->UnclaimedBetPayouts(ServerCareer.Id);if(Due.IsEmpty()) { return; }
    // Rule 8: one settlement receipt per bet. A receipt already in the career (for example a refund
    // from another host) is never paid again.
    FFiskCareerProfile P=ServerCareer;TArray<FGuid> Done;int64 Paid=0;
    for(const FFiskBetPayout& Payout:Due)
    {
        if(P.HasReceipt(Payout.Receipt)) { Done.Add(Payout.Receipt);continue; }
        if(P.Credit(Payout.Receipt,Payout.Amount)) { Done.Add(Payout.Receipt);Paid+=Payout.Amount; }
    }
    if(P.Revision!=ServerCareer.Revision)
    {
        const FString Message=Paid>0?FString::Printf(TEXT("Strandbaren: veddemålet ga $%lld.%02lld."),static_cast<long long>(Paid/100),static_cast<long long>(Paid%100)):TEXT("Strandbaren: veddemålet ga ingen gevinst denne gangen.");
        if(!SaveServerCareer(P,Message)) { return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BET_PAID player=%d receipts=%d amount=%lld cash=%lld"),Owner.PlayerState?Owner.PlayerState->GetPlayerId():-1,Done.Num(),static_cast<long long>(Paid),static_cast<long long>(P.Cash));
    }
    for(const FGuid& Receipt:Done) { Mode->ClaimBetPayout(Receipt); }
}
void UFiskCareerSync::RefundOrphanBets(FFiskCareerProfile& P) const
{
    // A stake whose bet this host no longer knows (the host restarted, or the bet was placed on another
    // host) is refunded under the bet's payout receipt, so a later payout of the same bet is ignored.
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();if(!Mode) { return; }
    const TArray<FFiskCareerReceipt> Receipts=P.Receipts;
    for(const FFiskCareerReceipt& R:Receipts)
    {
        const FGuid Payout=FFiskCareerProfile::BetPayoutId(R.Id);
        if(R.Kind!=FFiskCareerReceipt::KindBet || P.HasReceipt(Payout) || Mode->IsBetPending(R.Id)) { continue; }
        const bool Refunded=P.Credit(Payout,-R.Amount);
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BET_ORPHAN_REFUND refunded=%d amount=%lld"),Refunded,static_cast<long long>(-R.Amount));
    }
}
void UFiskCareerSync::ServerBuyCareer_Implementation(FGuid Command,int32 Revision,int32 Item)
{
    AFiskBoardingController& Owner=PC();
    const AFiskStarterBoat* Boat=Owner.FindStarterBoat();const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    const bool Dock=Boat && Owner.PlayerState && (!Boat->IsAboard(Owner.PlayerState->GetPlayerId()) || Boat->GetTripProgress()==AFiskStarterBoat::AtDock || Boat->GetTripProgress()==AFiskStarterBoat::AtMarina);
    FFiskCareerProfile P=ServerCareer;
    const bool Accepted=bCareerReady && Day && Day->HasStarted() && Dock && P.Buy(Command,Revision,Item)
        && SaveServerCareer(P,TEXT("Kjøp bekreftet: ")+FFiskCareerProfile::ItemName(Item));
    if(!Accepted) { Owner.ClientActionFeedback(TEXT("Kjøp avvist: kontroller saldo, eid utstyr, brygge og oppdatert karriere.")); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_BUY_%s player=%d item=%d cash=%lld"),Accepted?TEXT("ACCEPTED"):TEXT("REJECTED"),Owner.PlayerState?Owner.PlayerState->GetPlayerId():-1,Item,static_cast<long long>(ServerCareer.Cash));
}
void UFiskCareerSync::ServerChooseBoat_Implementation(int32 Revision,int32 Kind)
{
    AFiskBoardingController& Owner=PC();
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();AFiskStarterBoat* Boat=Mode?Mode->GetBoatForPlayer(Owner.PlayerState?Owner.PlayerState->GetPlayerId():INDEX_NONE):nullptr;
    const bool Accepted=bCareerReady && Boat && Owner.PlayerState && (Kind==0 || (Kind==1 && ServerCareer.Has(FFiskCareerProfile::ItemOffshoreBoat))) && Boat->ChooseKind(Owner.PlayerState->GetPlayerId(),Revision,Kind);
    Owner.ClientActionFeedback(Accepted?TEXT("Båt valgt. Mannskap og stangfordeling er oppdatert."):TEXT("Båtvalg avvist. Bare eieren kan velge eid båt ved brygga, med plass til alle."));
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BOAT_CHOICE accepted=%d kind=%d"),Accepted,Kind);
}
