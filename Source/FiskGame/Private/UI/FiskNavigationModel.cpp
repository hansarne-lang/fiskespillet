#include "UI/FiskNavigationModel.h"
#include "FiskFishingGround.h"
#include "Fishing/FiskPrototypeRod.h"

FFiskSonarView FiskNavigation::Sonar(int32 Ground,int32 Progress,bool bAboard,int32 BaitDepth)
{
    FFiskSonarView View;
    const FFiskFishingGround* G=FiskGrounds::Find(Ground);
    if(!G || !bAboard || Progress!=100) { return View; }
    View.bAvailable=true;View.Bottom=G->BottomDepth;View.Near=G->FishDepthMin;View.Far=G->FishDepthMax;
    View.BaitDepth=BaitDepth;View.Ground=G->Name;
    View.Structure=Ground==0?TEXT("Korallrev"):Ground==1?TEXT("Bratt skrent"):TEXT("Dypvann");
    return View;
}
bool FiskNavigation::Revealed(const FFiskSettlementView& View,int32 Day,int32 NowMs)
{ return Day>0 && View.DayNumber==Day && View.bRevealed && NowMs>=1020000; }
TArray<FFiskBoardRow> FiskNavigation::Standings(const FFiskSettlementView& View,int32 Day,int32 NowMs)
{
    TArray<FFiskBoardRow> Rows;
    if(!Revealed(View,Day,NowMs)) { return Rows; }
    for(const FFiskDayResult& Result:View.Results)
    {
        FFiskBoardRow Row;Row.Player=Result.PlayerId;Row.Day=Day;Row.Income=Result.SessionBalance;
        Row.Delivered=Result.DeliveredFish;Row.Lost=Result.LostFish;Row.Grams=Result.DeliveredWeightGrams;
        for(const FFiskCatchResult& Catch:Result.Catches)
        { if(Catch.Fish.WeightGrams>Row.BestGrams) { Row.BestGrams=Catch.Fish.WeightGrams;Row.Species=Catch.Fish.Species; } }
        Rows.Add(Row);
    }
    Rows.Sort([](const FFiskBoardRow& A,const FFiskBoardRow& B){return A.Income!=B.Income?A.Income>B.Income:A.Player<B.Player;});
    for(int32 I=0;I<Rows.Num();++I) { Rows[I].Rank=I>0 && Rows[I].Income==Rows[I-1].Income?Rows[I-1].Rank:I+1; }
    return Rows;
}
TArray<FFiskBoardRow> FiskNavigation::Records(const TArray<FFiskHistoryDay>& History,const FFiskSettlementView& View,int32 Day,int32 NowMs)
{
    TArray<FFiskBoardRow> Rows;
    for(const FFiskHistoryDay& H:History)
    {
        if(H.Number<=0 || H.Number>Day || (H.Number==Day && !Revealed(View,Day,NowMs))) { continue; }
        for(const FFiskHistoryRow& R:H.Rows)
        {
            if(R.BestGrams<=0) { continue; }
            FFiskBoardRow* Existing=Rows.FindByPredicate([&](const FFiskBoardRow& X){return X.Player==R.PlayerId;});
            if(Existing && Existing->BestGrams>=R.BestGrams) { continue; }
            FFiskBoardRow Record;Record.Player=R.PlayerId;Record.Day=H.Number;Record.BestGrams=R.BestGrams;Record.Species=R.BestSpecies;
            if(Existing) { *Existing=Record; }else { Rows.Add(Record); }
        }
    }
    Rows.Sort([](const FFiskBoardRow& A,const FFiskBoardRow& B){return A.BestGrams!=B.BestGrams?A.BestGrams>B.BestGrams:A.Player<B.Player;});
    for(int32 I=0;I<Rows.Num();++I) { Rows[I].Rank=I>0 && Rows[I].BestGrams==Rows[I-1].BestGrams?Rows[I-1].Rank:I+1; }
    return Rows;
}
FString FiskNavigation::NextAction(bool Started,int32 Ms,int32 Progress,bool Aboard,bool Captain,bool Host,int32 State,bool RigValid,bool Busy)
{
    if(!Started) { return TEXT("Velg Spill for å starte alene eller som LAN-vert."); }
    if(Ms>=1200000) { return Host?TEXT("N: start neste fiskedag. F8: LAN-tavle."):TEXT("Venter på vertens neste dag. F8: LAN-tavle."); }
    if(Ms>=1020000) { return TEXT("F2: se oppgjøret. F4: journal. F8: LAN-tavle."); }
    if(!Aboard) { return Progress==-1?TEXT("E: gå om bord. M: velg fiskeplass etterpå."):TEXT("Båten er ute. Vent på neste fiskedag."); }
    if(Progress==-1) { return Ms<120000?TEXT("M: velg plass. G: rigg stanga. Avreise fra 02:00."):Captain?TEXT("R: dra til valgt fiskeplass."):TEXT("Klar til avreise. Kapteinen starter turen."); }
    if(Progress==200) { return TEXT("Tilbake i marinaen. Resultater avsløres ved 17:00."); }
    if(Progress!=100) { return Progress<100?TEXT("På vei til fiskeplassen. Se ruten med M."):TEXT("På vei hjem. Returfristen er 17:00."); }
    if(!RigValid) { return TEXT("G: juster agndybden før du setter ut."); }
    switch(static_cast<EFiskRodState>(State))
    {
    case EFiskRodState::Fishing:return TEXT("Stanga fisker automatisk. F7: se ekkoloddet.");
    case EFiskRodState::InitialRun:return TEXT("Napp! Første utras går automatisk. Vent til det stopper.");
    case EFiskRodState::Fighting:return TEXT("Pump med musa opp, sveiv ved å senke den. Følg belastningen.");
    case EFiskRodState::ReadyToLand:return TEXT("Mellomrom: land fisken nå.");
    case EFiskRodState::Landed:if(Busy) { return TEXT("Landing pågår. Stanga blir snart klar."); }break;
    default:break;
    }
    return TEXT("F: sett ut stanga. G: velg oppsett. F7: ekkolodd.");
}
