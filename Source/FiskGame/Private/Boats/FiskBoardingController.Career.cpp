#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Career/FiskCareerStore.h"
#include "Career/FiskJournalAuthority.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskUserSettings.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskCrewPanel.h"
#include "UI/FiskJournalPanel.h"
#include "UI/FiskDeckPanel.h"
#include "Economy/FiskDaySettlement.h"
#include "DayCycle/FiskDayState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Misc/Paths.h"

FString AFiskBoardingController::CareerAuthorityFile() const
{ return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Authority"),ServerCareer.Id.ToString(EGuidFormats::Digits)+TEXT(".ini")); }
bool AFiskBoardingController::SaveServerCareer(const FFiskCareerProfile& P,const FString& Message)
{
    if(!P.IsValid() || !FFiskCareerStore::Save(CareerAuthorityFile(),P))
    { ClientActionFeedback(TEXT("Verten kunne ikke lagre karrieren. Ingen endring bekreftet; prøv igjen."));return false; }
    ServerCareer=P;ClientCareerSnapshot(ServerCareer,Message);return true;
}
void AFiskBoardingController::ServerCareerHello_Implementation(FFiskCareerProfile P)
{
    if(!PlayerState || !P.IsValid()) { ClientActionFeedback(TEXT("Ugyldig karriere. Eksisterende filer er bevart."));return; }
    if(bCareerReady) { ClientCareerSnapshot(ServerCareer,TEXT("Karriere er allerede tilkoblet."));return; }
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
    { const AFiskBoardingController* Other=Cast<AFiskBoardingController>(It->Get());if(Other && Other!=this && Other->bCareerReady && Other->ServerCareer.Id==P.Id)
        { ClientActionFeedback(TEXT("Denne karrieren er allerede tilkoblet. Bruk en egen profil per spiller."));return; } }
    ServerCareer=P;FFiskCareerProfile Known;const EFiskCareerLoad Status=FFiskCareerStore::Load(CareerAuthorityFile(),Known);
    if(Status==EFiskCareerLoad::Blocked || ((Status==EFiskCareerLoad::Loaded || Status==EFiskCareerLoad::Recovered) && !P.Extends(Known) && !Known.Extends(P)))
    { ServerCareer=FFiskCareerProfile();ClientActionFeedback(TEXT("Vertens profilkopi er skadet eller i konflikt. Filene er bevart."));return; }
    if(Known.IsValid() && Known.Revision>P.Revision) { P=Known; }
    if(!FiskJournalAuthority::RecoverIncome(FPaths::GetPath(CareerAuthorityFile()),P)) { ClientActionFeedback(TEXT("Vertens bekreftede journal/inntekt kan ikke leses. Eksisterende data er bevart."));return; }
    RefundOrphanBets(P);
    if(!SaveServerCareer(P,TEXT("Karriere bekreftet av verten."))) { return; }
    bCareerReady=true;
    if(auto* M=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>()) { M->RegisterFleetCareer(PlayerState->GetPlayerId(),P.Id); }
    if(AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>()) { Mode->BindCareer(PlayerState->GetPlayerId(),P.Id); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_READY player=%d revision=%d cash=%lld owned=%d"),PlayerState->GetPlayerId(),P.Revision,static_cast<long long>(P.Cash),P.Owned);
}
void AFiskBoardingController::ClientCareerSnapshot_Implementation(FFiskCareerProfile P,const FString& Message)
{
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetGameInstance());
    if(!P.IsValid()) { return; }PendingCareer=P;
    if(GI->AcceptCareer(P)) { PendingCareer=FFiskCareerProfile();if(!HasAuthority()) { bCareerReady=true; }ServerCareerAck(P.Id,P.Revision); }
    ClientActionFeedback_Implementation(Message+TEXT(" ")+GI->CareerMessage);
}
void AFiskBoardingController::ServerCareerAck_Implementation(FGuid Profile,int32 Revision)
{
    if(ServerCareer.Id==Profile && ServerCareer.Revision==Revision)
    { CareerAck=Revision;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_SAVED player=%d revision=%d"),PlayerState?PlayerState->GetPlayerId():-1,Revision); }
}
void AFiskBoardingController::TickCareer()
{
    if(HasAuthority()) { ReconcileRodCapacity(); }
    const double Now=GetWorld()->GetRealTimeSeconds();
    if(IsLocalController() && !bCareerHelloSent && GetNetMode()!=NM_Standalone)
    {
        const UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetGameInstance());
        if(!GI->bCareerBlocked && GI->Career.IsValid()) { bCareerHelloSent=true;ServerCareerHello(GI->Career); }
    }
    if(Now<CareerRetryAt) { return; }CareerRetryAt=Now+2;
    if(IsLocalController() && PendingCareer.IsValid())
    { UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetGameInstance());if(GI->AcceptCareer(PendingCareer)) { if(!HasAuthority()) { bCareerReady=true; }ServerCareerAck(PendingCareer.Id,PendingCareer.Revision);PendingCareer=FFiskCareerProfile(); } }
    if(!HasAuthority() || !bCareerReady) { return; }
    RecoverySync->TickJournalRecovery();
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();const AFiskDaySettlement* Settlement=Mode?Mode->GetSettlement():nullptr;
    if(Day && Settlement && Day->GetElapsedMilliseconds()>=1020000 && Settlement->GetView().bRevealed)
    {
        const FGuid Id=Settlement->GetView().DayId;
        if(!ServerCareer.Receipts.ContainsByPredicate([&](const FFiskCareerReceipt& R){return R.Id==Id;}))
        {
            FFiskCareerProfile P=ServerCareer;const int64 Income=Mode->CareerIncome(P.Id);
            if(P.Credit(Id,Income) && SaveServerCareer(P,TEXT("17:00: dagens kvalifiserte fangst er bokført.")))
            { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_SETTLED player=%d amount=%lld revision=%d cash=%lld"),PlayerState->GetPlayerId(),static_cast<long long>(Income),P.Revision,static_cast<long long>(P.Cash)); }
            else { ClientActionFeedback(TEXT("Karriereoppgjør venter på lagring eller ledig kvitteringskapasitet. Neste dag er sperret."));CareerAck=0; }
        }
    }
    if(Mode && Day && Day->GetElapsedMilliseconds()>=1020000) { Mode->SettleBets(); }
    ClaimBetPayouts();
    if(CareerAck<ServerCareer.Revision) { ClientCareerSnapshot(ServerCareer,TEXT("Venter på lokal lagringsbekreftelse.")); }
}
void AFiskBoardingController::ServerPlaceBet_Implementation(FGuid Bet,FGuid Fisher,int64 Stake)
{
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    EFiskBetResult Result=EFiskBetResult::Closed;
    if(bCareerReady && PlayerState && Mode)
    {
        Result=Mode->PlaceBet(PlayerState->GetPlayerId(),ServerCareer.Id,Bet,Fisher,Stake,[&]()
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
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BET_REPLY player=%d result=%d stake=%lld cash=%lld"),PlayerState?PlayerState->GetPlayerId():-1,static_cast<int32>(Result),static_cast<long long>(Stake),static_cast<long long>(ServerCareer.Cash));
    SendBetBoard(Message);
}
void AFiskBoardingController::ServerBetBoard_Implementation() { if(bCareerReady) { SendBetBoard(FString()); } }
void AFiskBoardingController::SendBetBoard(const FString& Message)
{
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();if(!Mode) { return; }
    FGuid Fisher;int64 Stake=0,Payout=-1;Mode->BetBoard(ServerCareer.Id,Fisher,Stake,Payout);
    ClientBetBoard(Fisher,Stake,Mode->BetPot(),Mode->BetCount(),Payout,Mode->AreBetsOpen(),Message);
}
void AFiskBoardingController::ClientBetBoard_Implementation(FGuid Fisher,int64 Stake,int64 Pot,int32 Count,int64 Payout,bool bOpen,const FString& Message)
{
    BetBoard.Fisher=Fisher;BetBoard.Stake=Stake;BetBoard.Pot=Pot;BetBoard.Count=Count;BetBoard.Payout=Payout;BetBoard.bOpen=bOpen;BetBoard.bKnown=true;
    if(!Message.IsEmpty()) { BetBoard.Message=Message; }
}
void AFiskBoardingController::ClaimBetPayouts()
{
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
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BET_PAID player=%d receipts=%d amount=%lld cash=%lld"),PlayerState?PlayerState->GetPlayerId():-1,Done.Num(),static_cast<long long>(Paid),static_cast<long long>(P.Cash));
    }
    for(const FGuid& Receipt:Done) { Mode->ClaimBetPayout(Receipt); }
}
void AFiskBoardingController::RefundOrphanBets(FFiskCareerProfile& P) const
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
void AFiskBoardingController::ServerBuyCareer_Implementation(FGuid Command,int32 Revision,int32 Item)
{
    const AFiskStarterBoat* Boat=FindStarterBoat();const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    const bool Dock=Boat && PlayerState && (!Boat->IsAboard(PlayerState->GetPlayerId()) || Boat->GetTripProgress()==AFiskStarterBoat::AtDock || Boat->GetTripProgress()==AFiskStarterBoat::AtMarina);
    FFiskCareerProfile P=ServerCareer;
    const bool Accepted=bCareerReady && Day && Day->HasStarted() && Dock && P.Buy(Command,Revision,Item)
        && SaveServerCareer(P,TEXT("Kjøp bekreftet: ")+FFiskCareerProfile::ItemName(Item));
    if(!Accepted) { ClientActionFeedback(TEXT("Kjøp avvist: kontroller saldo, eid utstyr, brygge og oppdatert karriere.")); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAREER_BUY_%s player=%d item=%d cash=%lld"),Accepted?TEXT("ACCEPTED"):TEXT("REJECTED"),PlayerState?PlayerState->GetPlayerId():-1,Item,static_cast<long long>(ServerCareer.Cash));
}
void AFiskBoardingController::ServerChooseBoat_Implementation(int32 Revision,int32 Kind)
{
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();AFiskStarterBoat* Boat=Mode?Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE):nullptr;
    const bool Accepted=bCareerReady && Boat && PlayerState && (Kind==0 || (Kind==1 && ServerCareer.Has(FFiskCareerProfile::ItemOffshoreBoat))) && Boat->ChooseKind(PlayerState->GetPlayerId(),Revision,Kind);
    ClientActionFeedback(Accepted?TEXT("Båt valgt. Mannskap og stangfordeling er oppdatert."):TEXT("Båtvalg avvist. Bare eieren kan velge eid båt ved brygga, med plass til alle."));
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BOAT_CHOICE accepted=%d kind=%d"),Accepted,Kind);
}
bool AFiskBoardingController::HandleCareerKey(const FInputKeyEventArgs& A)
{
    const bool Press=A.Event==IE_Pressed;
    if(!bCareerVisible && Press && A.Key==EKeys::P)
    {
        // The chart owns typed letters while naming a favorite.
        if(SeaChart->IsVisible()) { return false; }
        if(FrontEnd->IsVisible() || RigPanel->IsVisible() || SeaChart->IsVisible() || CrewPanel->IsVisible() || JournalPanel->IsVisible() || DeckPanel->IsVisible()
            || bHelpVisible || bResultsVisible || bHistoryVisible) { ClientActionFeedback_Implementation(TEXT("Lukk panelet først med Escape."));return true; }
        bCareerVisible=true;ReleaseRodInputsForMenu();bShowMouseCursor=true;SetInputMode(FInputModeGameAndUI());return true;
    }
    if(!bCareerVisible) { return false; }
    if(!Press) { return true; }
    if(A.Key==EKeys::P || A.Key==EKeys::Escape) { bCareerVisible=false;bShowMouseCursor=false;SetInputMode(FInputModeGameOnly());return true; }
    if(A.Key==EKeys::Up) { ShopRow=(ShopRow+5)%6; }
    if(A.Key==EKeys::Down) { ShopRow=(ShopRow+1)%6; }
    bool Choose=A.Key==EKeys::Enter;
    if(A.Key==EKeys::LeftMouseButton)
    {
        FVector2D Mouse;int32 W=0,H=0;GetViewportSize(W,H);const float S=FMath::Min(W/1280.f,H/720.f);
        if(S>0 && GetWorld()->GetGameViewport() && GetWorld()->GetGameViewport()->GetMousePosition(Mouse))
        { for(int32 I=0;I<6;++I) { if(FBox2D(FVector2D(52*S,(180+58*I)*S),FVector2D(1210*S,(230+58*I)*S)).IsInside(Mouse)) { ShopRow=I;Choose=true; } } }
    }
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetGameInstance());
    if(A.Key==EKeys::F5) { if(GI->AcceptCareer(PendingCareer.IsValid()?PendingCareer:GI->Career)) { if(PendingCareer.IsValid()) { if(!HasAuthority()) { bCareerReady=true; }ServerCareerAck(PendingCareer.Id,PendingCareer.Revision);PendingCareer=FFiskCareerProfile(); }else if(!bCareerReady) { bCareerHelloSent=false; } } }
    if(Choose)
    { if(ShopRow<4) { ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,ShopRow); }else if(const AFiskStarterBoat* Boat=FindStarterBoat()) { ServerChooseBoat(Boat->GetRouteRevision(),ShopRow-4); } }
    return true;
}
void AFiskBoardingController::ServerSelectSeaPoint_Implementation(int32 Revision,FVector_NetQuantize Point)
{
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();FString Message;
    if(Mode && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE) && PlayerState) { Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE)->SelectPoint(PlayerState->GetPlayerId(),Revision,Point,Message); }
    ClientGroundReply(Message);
}
void AFiskBoardingController::ServerTravelVote_Implementation(int32 Leg,bool Vote)
{
    auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const bool Accepted=bCareerReady && PlayerState && Mode && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE) && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE)->VoteTravel(PlayerState->GetPlayerId(),Leg,Vote);
    if(!Accepted) { ClientActionFeedback(TEXT("Reisevisning kan velges av dem om bord mens båten er underveis.")); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_TRAVEL_VOTE_REPLY accepted=%d"),Accepted);
}
void AFiskBoardingController::DrawCareer(AHUD& HUD,UCanvas& C)
{
    const UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetGameInstance());const FFiskCareerProfile& P=GI->Career;
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.45f,.94f,.82f),Gold(1,.79f,.36f);
    auto T=[&](const FString& Text,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){HUD.DrawText(Text,Color,X*S,Y*S,nullptr,Size*S);};
    HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);
    T(TEXT("KARRIERE OG MARINABUTIKK"),52,55,2);T(P.Money(),990,61,1.8f,Gold);
    T(FString::Printf(TEXT("%s | Profil %s | Kvitteringer %d/512"),*P.Name,*P.Id.ToString(EGuidFormats::Digits).Left(8),P.Revision),52,105,1.f,Teal);
    T(TEXT("Kjøp ved brygga. Dagens nye fangstinntekt bokføres først ved 17:00."),52,143,1.f);
    const TCHAR* Effects[]={TEXT("15% høyere grense for linebrudd"),TEXT("15% raskere innsveiving"),TEXT("10% sterkere pumping"),TEXT("4 plasser | 6 holdere | 25% raskere"),TEXT("Velg startbåt: 2 plasser / 2 holdere"),TEXT("Velg eid offshorebåt: 4 plasser / 6 holdere")};
    for(int32 I=0;I<6;++I)
    {
        const float Y=180+58*I;HUD.DrawRect(I==ShopRow?FLinearColor(.14f,.34f,.38f):FLinearColor(.035f,.12f,.16f),52*S,Y*S,1158*S,50*S);
        const FString Label=I<4?FFiskCareerProfile::ItemName(I):I==4?TEXT("Bruk startbåt"):TEXT("Bruk offshorebåt");
        T(Label,68,Y+7,1.1f);T(Effects[I],440,Y+12,.95f,Teal);
        T(I<4?(P.Has(I)?TEXT("EID"):FString::Printf(TEXT("$%lld"),static_cast<long long>(FFiskCareerProfile::Price(I)/100))):TEXT("VELG"),1050,Y+12,1.f,Gold);
    }
    T(GI->CareerMessage,52,551,.95f,Gold);T(GetActionFeedback(),52,581,.85f);
    T(TEXT("Piler / klikk: velg | Enter: kjøp eller bruk båt | F5: lagre igjen | P / Escape: lukk"),52,622,.95f,Teal);
    T(TEXT("Testøkonomi: ny karriere starter med $30 000 én gang. Egen fangst gir egen inntekt."),52,648,.85f);
}
