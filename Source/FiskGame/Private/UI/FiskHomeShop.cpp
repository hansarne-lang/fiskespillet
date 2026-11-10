#include "UI/FiskMarinaPanel.h"
#include "UI/FiskGameInstance.h"
#include "Boats/FiskBoardingController.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"

void UFiskMarinaPanel::ChooseHomeItem()
{
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    if(bBlocked || bSending || !IsAtDock() || !PC->IsCareerReady()) { Status=TEXT("Vent på bekreftelse, eller åpne butikken ved marinaen.");return; }
    const int32 Item=FFiskMarinaIdentity::PremiumItem(ShopCategory,ShopLevel);
    if(ShopLevel>0 && !GI->Career.Has(Item))
    {
        if(GI->Career.Cash<FFiskCareerProfile::Price(Item)) { Status=TEXT("Du har ikke nok disponible fiskepenger. Dagens inntekt kommer først 17:00.");return; }
        if(ShopBuyItem!=Item || !ShopBuyCommand.IsValid()) { ShopBuyItem=Item;ShopBuyCommand=FGuid::NewGuid(); }
        PC->ServerBuyCareer(ShopBuyCommand,GI->Career.Revision,Item);Status=TEXT("Kjøp sendt. Velg varen igjen etter bekreftelse for å sette den på naustet.");return;
    }
    FFiskMarinaIdentity Choice=Identity;
    if(ShopCategory==0) { Choice.Lighting=ShopLevel; }if(ShopCategory==1) { Choice.Pier=ShopLevel; }if(ShopCategory==2) { Choice.Display=ShopLevel; }
    if(!Choice.IsOwnedBy(GI->Career)) { Status=TEXT("Dette nivået må kjøpes først.");return; }
    Draft=Choice;bSending=true;Status=TEXT("Venter på bekreftet naustvalg ...");PC->GetMarinaSync()->ServerMarinaIdentity(LoadedProfile,Choice);
}
void UFiskMarinaPanel::DrawHomeShop(AHUD& HUD,UCanvas& C)
{
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){HUD.DrawText(V,Color,X*S,Y*S,nullptr,Size*S);};
    const TCHAR* Categories[]={TEXT("LYS OG LANTERNER"),TEXT("BRYGGE OG TERRASSE"),TEXT("TROFÉUTSTILLING")};
    const int32 Active=ShopCategory==0?Identity.Lighting:ShopCategory==1?Identity.Pier:Identity.Display;
    if(bShopPreview)
    {
        HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.93f),24*S,592*S,1232*S,86*S);
        T(TEXT("DITT BEKREFTEDE NAUST | V: tilbake til butikken | H: naust | Escape: lukk"),46,612,1.f,Teal);
        T(TEXT("Bare aktivert, kjøpt pynt vises. Troféfangster vises først etter bekreftet oppgjør."),46,646,.9f);return;
    }
    T(TEXT("NAUSTBUTIKKEN"),46,58,1.8f);T(GI->Career.Money(),46,108,1.3f,Gold);
    T(FString::Printf(TEXT("< %s >"),Categories[ShopCategory]),46,151,1.15f,Teal);
    for(int32 Level=0;Level<4;++Level)
    {
        const int32 Item=FFiskMarinaIdentity::PremiumItem(ShopCategory,Level);const bool Owned=Level==0 || GI->Career.Has(Item);
        HUD.DrawRect(ShopLevel==Level?FLinearColor(.14f,.34f,.36f):FLinearColor(.03f,.12f,.15f),42*S,(199+78*Level)*S,568*S,70*S);
        T(Level==0?TEXT("GRUNNUTFØRELSE – GRATIS"):FFiskCareerProfile::ItemName(Item),54,211+78*Level,1.f);
        const FString Price=Level==0?TEXT("Alltid tilgjengelig"):FString::Printf(TEXT("$%lld | %s"),static_cast<long long>(FFiskCareerProfile::Price(Item)/100),Owned?TEXT("EID"):TEXT("IKKE KJØPT"));
        T(Price+(Active==Level?TEXT(" | AKTIV"):TEXT("")),54,242+78*Level,.85f,Active==Level?Gold:Teal);
    }
    T(TEXT("Piler: kategori/nivå | Enter: kjøp eller aktiver"),46,530,.85f,Teal);
    T(Status,46,562,.73f,Gold);
    T(TEXT("Pynt gir ingen fiskebonus. Hvert nivå kjøpes separat."),46,601,.79f);
    T(TEXT("V: se hele naustet | H / N: tilbake | Escape: lukk"),46,628,.8f,Teal);
    T(TEXT("Tidligere penger kan brukes. Ny fangstinntekt først 17:00."),46,651,.74f,Teal);
}
