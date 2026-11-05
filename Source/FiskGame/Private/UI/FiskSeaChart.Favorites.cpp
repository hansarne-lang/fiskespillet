#include "UI/FiskSeaChart.h"
#include "UI/FiskGameInstance.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "FiskFishingGround.h"
#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"

bool UFiskSeaChart::HandleFavorites(const FInputKeyEventArgs& Args)
{
    if(Args.Event!=IE_Pressed && Args.Event!=IE_Repeat) { return true; }
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    const FKey K=Args.Key;
    if(bRenaming)
    {
        if(K==EKeys::Escape) { bRenaming=false;EditName.Reset(); }
        else if(K==EKeys::BackSpace) { EditName.LeftChopInline(1); }
        else if(K==EKeys::Enter)
        {
            const FFiskSavedPlace* Saved=GI->Places.Get(FavoriteSlot);
            const FString Name=EditName.TrimStartAndEnd();
            if(!Saved || !FFiskNavigationNotebook::ValidName(Name)) { Message=TEXT("Bruk 1-24 bokstaver/tall, mellomrom, - eller _. Navnet kan ikke være tomt."); }
            else { GI->SaveSeaPlace(FavoriteSlot,Saved->Point,Name);Message=GI->PlacesMessage;bRenaming=false; }
        }
        else if(EditName.Len()<FFiskNavigationNotebook::NameLimit)
        {
            FString Character=K.GetFName().ToString();
            if(K==EKeys::SpaceBar) { Character=TEXT(" "); }
            if(K==EKeys::Hyphen) { Character=TEXT("-"); }
            const TArray<FKey> Digits={EKeys::Zero,EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};
            const int32 Digit=Digits.IndexOfByKey(K);if(Digit!=INDEX_NONE) { Character=FString::FromInt(Digit); }
            if(Character.Len()==1 && (FChar::IsAlnum(Character[0]) || Character==TEXT(" ") || Character==TEXT("-"))) { EditName+=Character; }
        }
        return true;
    }
    if(K==EKeys::Escape) { Show(false);return true; }
    if(K==EKeys::Up) { FavoriteSlot=(FavoriteSlot+5)%6; }
    else if(K==EKeys::Down) { FavoriteSlot=(FavoriteSlot+1)%6; }
    const TArray<FKey> Slots={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six};
    const int32 Selected=Slots.IndexOfByKey(K);if(Selected!=INDEX_NONE) { FavoriteSlot=Selected; }
    if(K==EKeys::F5)
    {
        const AFiskStarterBoat* Boat=PC->FindStarterBoat();
        if(Boat && FiskGrounds::Find(Boat->GetGroundId()))
        { GI->SaveSeaPlace(FavoriteSlot,Boat->GetDestination(),FiskGrounds::Find(Boat->GetGroundId())->Name);Message=GI->PlacesMessage; }
    }
    else if(K==EKeys::F2)
    {
        if(const FFiskSavedPlace* Saved=GI->Places.Get(FavoriteSlot)) { EditName=Saved->Name;bRenaming=true;Message.Reset(); }
        else { Message=TEXT("Lagre en plass med F5 før du gir den navn."); }
    }
    else if(K==EKeys::Delete) { GI->ClearPlace(FavoriteSlot);Message=GI->PlacesMessage; }
    else if(K==EKeys::Enter)
    {
        if(const FFiskSavedPlace* Saved=GI->Places.Get(FavoriteSlot)) { if(const AFiskStarterBoat* B=PC->FindStarterBoat()) { PC->ServerSelectSeaPoint(B->GetRouteRevision(),Saved->Point); } }
        else { Message=TEXT("Denne favorittplassen er tom. F5 lagrer båtens valgte rute."); }
    }
    else if(K==EKeys::LeftMouseButton)
    {
        FVector2D Mouse;
        if(UGameViewportClient* VP=GetWorld()->GetGameViewport();VP && VP->GetMousePosition(Mouse))
        { for(int32 I=0;I<Buttons.Num();++I) { if(Buttons[I].IsInside(Mouse)) { FavoriteSlot=I;break; } } }
    }
    return true;
}
void UFiskSeaChart::DrawFavorites(AHUD& HUD,UCanvas& Canvas)
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f);
    const FLinearColor Teal(.45f,.94f,.82f),Muted(.64f,.81f,.85f);
    auto Text=[&](const FString& T,float X,float Y,float Size=1.0f,FLinearColor C=FLinearColor::White){HUD.DrawText(T,C,X*S,Y*S,nullptr,Size*S);};
    HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,642*S);
    Text(TEXT("MINE FISKEPLASSER"),52,58,2.0f);
    Text(TEXT("Tab: tilbake til sjøkartet | Seks navngitte sjøpunkter, lagret lokalt"),52,112,1.0f,Muted);
    Buttons.Reset();
    for(int32 I=0;I<6;++I)
    {
        const FFiskSavedPlace* Saved=GI->Places.Get(I);const float Y=166+I*57;
        Buttons.Add(FBox2D(FVector2D(52*S,Y*S),FVector2D(1212*S,(Y+48)*S)));
        HUD.DrawRect(I==FavoriteSlot?FLinearColor(.12f,.32f,.34f):FLinearColor(.025f,.10f,.13f),52*S,Y*S,1160*S,48*S);
        Text(FString::Printf(TEXT("%d. %s"),I+1,Saved?*Saved->Name:TEXT("Tom plass")),68,Y+12,1.15f,I==FavoriteSlot?Teal:FLinearColor::White);
        if(Saved)
        {
            const FFiskFishingGround& G=*FiskGrounds::Find(Saved->Ground);
            int32 Habitat=Saved->Ground,Duration=G.TravelMs;FiskGrounds::MakeSeaPoint(Saved->Point,Habitat,Duration);
            Text(FString::Printf(TEXT("%s | %d m | %.0f, %.0f m | %.1f s"),G.Name,G.BottomDepth,Saved->Point.X/100,Saved->Point.Y/100,Duration/1000.0*(Boat && Boat->GetKind()==1?.8:1.0)),560,Y+14,.82f,Muted);
        }
    }
    if(bRenaming)
    {
        Text(FString::Printf(TEXT("Navn: %s_"),*EditName),52,520,1.3f,Teal);
        Text(TEXT("Skriv A-Z/0-9, mellomrom eller -. Backspace: fjern. Enter: lagre. Escape: avbryt."),52,554,.95f,Muted);
    }
    else
    {
        Text(TEXT("1-6 / piler / klikk: velg | Enter: bruk favoritt | F2: gi navn | Delete: slett"),52,520,1.0f,Teal);
        const FFiskFishingGround* G=Boat?FiskGrounds::Find(Boat->GetGroundId()):nullptr;
        Text(FString::Printf(TEXT("F5: lagre båtens valgte rute (%s) i valgt plass"),G?G->Name:TEXT("ukjent")),52,554,1.0f);
    }
    Text(Message.IsEmpty()?GI->PlacesMessage:Message,52,596,.95f,FLinearColor(1,.8f,.4f));
    Text(TEXT("Bare kapteinen velger rute ved brygga. Lokal lagring flytter aldri båten. M / Escape: lukk."),52,636,.92f,Muted);
}
