#include "Bootstrap/FiskBootstrapWeighIn.h"
#include "Economy/FiskDaySettlement.h"
#include "Fishing/FiskCatchCatalog.h"
#include "UI/FiskPlayerNames.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"

namespace
{
    FString FishLine(const UWorld* World,const FFiskWeighEntry& E)
    {
        const FFiskSpecies* Species=FiskCatches::Find(E.Fish.Species);
        return FString::Printf(TEXT("%s: %s, %.1f kg, %.0f cm, %d fiskepenger"),*FiskPlayerNames::Name(World,E.PlayerId),
            Species?Species->Name:TEXT("Ukjent"),E.Fish.WeightGrams/1000.0,E.Fish.LengthMm/10.0,E.Fish.Value);
    }
}

void FiskWeighInScreen::Draw(AHUD& HUD,UCanvas& Canvas)
{
    const UWorld* World=HUD.GetWorld();if(!World) { return; }
    TActorIterator<AFiskDaySettlement> It(const_cast<UWorld*>(World));if(!It) { return; }
    const FFiskSettlementView& View=It->GetView();const FFiskWeighInView& W=View.WeighIn;
    if(!W.bStarted || View.bRevealed) { return; }
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f);
    const float X=(Canvas.ClipX-900*S)/2,Y=40*S;
    const FLinearColor Gold(1,0.86f,0.5f),Teal(0.5f,0.9f,0.8f),Muted(0.75f,0.8f,0.85f);
    HUD.DrawRect(FLinearColor(0.015f,0.055f,0.075f,0.94f),X,Y,900*S,330*S);
    HUD.DrawText(TEXT("VEIING"),FLinearColor::White,X+24*S,Y+14*S,nullptr,1.8f*S);
    HUD.DrawText(FString::Printf(TEXT("Veid %d av %d  |  minste først, største til slutt"),W.Weighed.Num(),W.Total),Teal,X+190*S,Y+24*S,nullptr,1.0f*S);
    if(W.Total==0)
    { HUD.DrawText(TEXT("Ingen levert fisk å veie i dag."),Muted,X+24*S,Y+80*S,nullptr,1.2f*S); }
    else if(!W.Weighed.IsEmpty())
    {
        const FFiskWeighEntry& Now=W.Weighed.Last();const FFiskSpecies* Species=FiskCatches::Find(Now.Fish.Species);
        const bool bLargest=W.Weighed.Num()==W.Total;
        HUD.DrawText(bLargest?TEXT("PÅ VEKTA: DAGENS STØRSTE"):TEXT("PÅ VEKTA"),Gold,X+24*S,Y+66*S,nullptr,1.0f*S);
        HUD.DrawText(FString::Printf(TEXT("%s  %.1f kg"),Species?Species->Name:TEXT("Ukjent"),Now.Fish.WeightGrams/1000.0),FLinearColor::White,X+24*S,Y+88*S,nullptr,1.9f*S);
        HUD.DrawText(FString::Printf(TEXT("Fisket av %s  |  %.0f cm  |  %d fiskepenger"),*FiskPlayerNames::Name(World,Now.PlayerId),Now.Fish.LengthMm/10.0,Now.Fish.Value),Gold,X+24*S,Y+134*S,nullptr,1.05f*S);
        HUD.DrawText(TEXT("Siste veid"),Muted,X+24*S,Y+176*S,nullptr,.9f*S);
        for(int32 I=1;I<=5 && W.Weighed.Num()-1-I>=0;++I)
        { HUD.DrawText(FishLine(World,W.Weighed[W.Weighed.Num()-1-I]),Muted,X+24*S,Y+(176+24*I)*S,nullptr,.85f*S); }
    }
    // The board: public from the start of the weigh-in (owner decision 2026-11-14).
    const float BX=X+560*S;
    HUD.DrawText(TEXT("Deltaker"),Teal,BX,Y+66*S,nullptr,.9f*S);HUD.DrawText(TEXT("Fisket"),Teal,BX+170*S,Y+66*S,nullptr,.9f*S);
    HUD.DrawText(TEXT("Veid"),Teal,BX+235*S,Y+66*S,nullptr,.9f*S);HUD.DrawText(TEXT("Tapt"),Teal,BX+290*S,Y+66*S,nullptr,.9f*S);
    for(int32 I=0;I<W.Tally.Num() && I<10;++I)
    {
        const FFiskWeighTally& T=W.Tally[I];const float RY=Y+(92+22*I)*S;
        HUD.DrawText(FiskPlayerNames::Name(World,T.PlayerId).Left(16),FLinearColor::White,BX,RY,nullptr,.9f*S);
        HUD.DrawText(FString::FromInt(T.Caught),FLinearColor::White,BX+180*S,RY,nullptr,.9f*S);
        HUD.DrawText(FString::FromInt(T.Weighed),Gold,BX+245*S,RY,nullptr,.9f*S);
        HUD.DrawText(FString::FromInt(T.Lost),Muted,BX+300*S,RY,nullptr,.9f*S);
    }
    if(W.Tally.IsEmpty()) { HUD.DrawText(TEXT("Ingen fangst i dag."),Muted,BX,Y+92*S,nullptr,.9f*S); }
    HUD.DrawText(TEXT("Resultat og utbetaling kommer når siste fisk er veid."),Muted,X+24*S,Y+300*S,nullptr,.85f*S);
}
