#include "UI/FiskMarinaPanel.h"
#include "Boats/FiskBoardingController.h"
#include "Camera/PlayerCameraManager.h"
#include "Boats/FiskStarterBoat.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskCrewPanel.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskJournalPanel.h"
#include "UI/FiskDeckPanel.h"
#include "Career/FiskMarinaStore.h"
#include "UI/FiskUserSettings.h"
#include "World/FiskMarinaHub.h"
#include "Fishing/FiskCatchCatalog.h"
#include "Economy/FiskDaySettlement.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"
#include "Misc/Paths.h"

UFiskMarinaPanel::UFiskMarinaPanel() { PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=.5f; }
FString UFiskMarinaPanel::File() const { return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("FiskelagetMarina-")+LoadedProfile.ToString(EGuidFormats::Digits)+TEXT(".ini")); }
AFiskMarinaHub* UFiskMarinaPanel::FindHub() const { TActorIterator<AFiskMarinaHub> Hub(GetWorld());return Hub?*Hub:nullptr; }
bool UFiskMarinaPanel::IsAtDock() const
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    return Boat && PC->PlayerState && (!Boat->IsAboard(PC->PlayerState->GetPlayerId()) || Boat->GetTripProgress()==-1 || Boat->GetTripProgress()==200);
}
void UFiskMarinaPanel::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Function)
{
    Super::TickComponent(Delta,Type,Function);AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());if(!PC->IsLocalController()) { return; }
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());if(!GI->Career.IsValid()) { return; }
    if(LoadedProfile!=GI->Career.Id)
    {
        LoadedProfile=GI->Career.Id;Identity=FFiskMarinaIdentity();Identity.DisplayName=GI->Career.Name==TEXT("FISKER")?TEXT("FISKER-")+LoadedProfile.ToString(EGuidFormats::Digits).Left(4):GI->Career.Name;
        const EFiskMarinaLoad Load=FFiskMarinaStore::Load(File(),LoadedProfile,Identity);bBlocked=Load==EFiskMarinaLoad::Blocked;
        if(Load==EFiskMarinaLoad::Empty) { bBlocked=!FFiskMarinaStore::Save(File(),LoadedProfile,Identity); }
        Draft=Identity;Status=bBlocked?TEXT("Naustfilen kunne ikke leses. Eksisterende filer er bevart."):Load==EFiskMarinaLoad::Recovered?TEXT("Naust hentet fra backup. Lagre for å reparere hovedfilen."):TEXT("Lagre valgene for å vise dem til venner.");
        bSent=false;SentJournalRevision=-1;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MARINA_LOADED blocked=%d recovery=%d"),bBlocked,Load==EFiskMarinaLoad::Recovered);
    }
    if(WantsCamera() && !IsAtDock()) { Show(0); }
    if(GetWorld()->GetRealTimeSeconds()<NextUpdate) { return; }NextUpdate=GetWorld()->GetRealTimeSeconds()+2;
    if(bPendingSave && FFiskMarinaStore::Save(File(),LoadedProfile,Pending)) { Identity=Pending;bPendingSave=false;Status=TEXT("Naust og navn er lagret lokalt."); }
    if(!bSent && !bBlocked && PC->IsCareerReady()) { bSent=true;PC->ServerMarinaIdentity(LoadedProfile,Identity); }
    if(bSent && PC->IsCareerReady() && SentJournalRevision!=GI->GetJournalRevision())
    {
        TArray<FFiskMarinaTrophy> Records;
        for(int32 I=0;I<FiskCatches::Count;++I) { if(const FFiskJournalEntry* Best=GI->GetJournal().Best(I)) { FFiskMarinaTrophy T;T.Species=I;T.Grams=Best->Fish.WeightGrams;T.LengthMm=Best->Fish.LengthMm;T.DayId=Best->DayId;Records.Add(T); } }
        PC->ServerMarinaRecords(LoadedProfile,Records);SentJournalRevision=GI->GetJournalRevision();
    }
}
void UFiskMarinaPanel::ReceiveIdentity(const FFiskMarinaIdentity& Value)
{
    if(!Value.IsValid()) { return; }bSending=false;Pending=Value;bPendingSave=!FFiskMarinaStore::Save(File(),LoadedProfile,Value);
    if(!bPendingSave) { Identity=Value;Draft=Value; }Status=bPendingSave?TEXT("Naust bekreftet av verten. Lokal lagring prøves igjen."):TEXT("Naust og navn er bekreftet og lagret.");
}
void UFiskMarinaPanel::Show(int32 NewPage)
{
    Page=NewPage;bEditing=false;Row=0;Draft=Identity;AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    PC->ReleaseRodInputsForMenu();PC->bShowMouseCursor=Page!=0;
    if(Page) { PC->SetInputMode(FInputModeGameAndUI()); }else { PC->SetInputMode(FInputModeGameOnly()); }
    if(Page==1) { if(const AFiskMarinaHub* H=FindHub()) { const int32 Own=H->GetResidents().IndexOfByPredicate([&](const FFiskMarinaResident& R){return R.Profile==LoadedProfile;});HouseIndex=FMath::Max(0,Own); } }
    if(PC->PlayerCameraManager) { PC->PlayerCameraManager->SetGameCameraCutThisFrame(); }
}
void UFiskMarinaPanel::Choose()
{
    if(bBlocked || bSending) { return; }AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if(Row<2) { bEditing=true;EditText=Row==0?Draft.DisplayName:Draft.HouseName;return; }
    if(Row==2) { Draft.Color=(Draft.Color+1)%6; }if(Row==3) { Draft.Style=(Draft.Style+1)%3; }if(Row==4) { Draft.Decor=(Draft.Decor+1)%3; }
    if(Row==5 && Draft.IsValid()) { bSending=true;Status=TEXT("Venter på vertens bekreftelse ...");PC->ServerMarinaIdentity(LoadedProfile,Draft); }
}
bool UFiskMarinaPanel::HandleKey(const FInputKeyEventArgs& A)
{
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());const bool Press=A.Event==IE_Pressed || A.Event==IE_Repeat;
    if(!Page)
    {
        if(!Press || (A.Key!=EKeys::H && A.Key!=EKeys::B && A.Key!=EKeys::I)) { return false; }
        if(PC->GetFrontEnd()->IsVisible() || PC->GetSeaChart()->IsVisible() || PC->GetCrewPanel()->IsVisible() || PC->GetRigPanel()->IsVisible() || PC->GetJournalPanel()->IsVisible() || PC->GetDeckPanel()->IsVisible() || PC->IsCareerVisible() || PC->IsHelpVisible() || PC->IsResultsVisible() || PC->IsHistoryVisible()) { return false; }
        if(A.Key!=EKeys::I && !IsAtDock()) { PC->ClientActionFeedback(TEXT("Naust og strandbar brukes ved marinaen."));return true; }
        Show(A.Key==EKeys::H?1:A.Key==EKeys::B?2:3);return true;
    }
    if(!Press) { return true; }
    if(bEditing)
    {
        if(A.Key==EKeys::Escape) { bEditing=false;return true; }
        if(A.Key==EKeys::Enter) { if(FFiskCareerProfile::ValidName(EditText)) { if(Row==0) { Draft.DisplayName=EditText; }else { Draft.HouseName=EditText; }bEditing=false; }return true; }
        if(A.Key==EKeys::BackSpace) { EditText=EditText.LeftChop(1);return true; }
        FString C=A.Key.GetFName().ToString();if(A.Key==EKeys::SpaceBar) { C=TEXT(" "); }if(A.Key==EKeys::Hyphen) { C=TEXT("-"); }
        const FKey Digits[]={EKeys::Zero,EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};
        const FKey Pad[]={EKeys::NumPadZero,EKeys::NumPadOne,EKeys::NumPadTwo,EKeys::NumPadThree,EKeys::NumPadFour,EKeys::NumPadFive,EKeys::NumPadSix,EKeys::NumPadSeven,EKeys::NumPadEight,EKeys::NumPadNine};
        for(int32 I=0;I<10;++I) { if(A.Key==Digits[I] || A.Key==Pad[I]) { C=FString::FromInt(I); } }
        if(C.Len()==1 && (FChar::IsAlnum(C[0]) || C==TEXT(" ") || C==TEXT("-")) && EditText.Len()<24) { EditText+=C.ToUpper(); }return true;
    }
    if(A.Key==EKeys::Escape || (Page==1 && A.Key==EKeys::H) || (Page==2 && A.Key==EKeys::B) || (Page==3 && A.Key==EKeys::I)) { Show(0);return true; }
    const AFiskMarinaHub* Hub=FindHub();
    if(Page==3) { if(A.Key==EKeys::Right || A.Key==EKeys::Down || A.Key==EKeys::PageDown) { Species=(Species+1)%FiskCatches::Count; }if(A.Key==EKeys::Left || A.Key==EKeys::Up || A.Key==EKeys::PageUp) { Species=(Species+FiskCatches::Count-1)%FiskCatches::Count; }return true; }
    const FKey Numbers[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five};
    if(Page==2) { for(int32 I=0;I<5;++I) { if(A.Key==Numbers[I]) { PC->ServerMarinaSay(I); } }return true; }
    for(int32 I=0;I<4;++I) { if(A.Key==Numbers[I] && Hub && Hub->GetResidents().IsValidIndex(I)) { HouseIndex=I;Draft=Identity;Row=0; } }
    const bool Own=Hub && Hub->GetResidents().IsValidIndex(HouseIndex) && Hub->GetResidents()[HouseIndex].Profile==LoadedProfile;
    if(!Own) { return true; }
    if(A.Key==EKeys::Up) { Row=(Row+5)%6; }if(A.Key==EKeys::Down || A.Key==EKeys::Tab) { Row=(Row+1)%6; }
    if(A.Key==EKeys::Right || A.Key==EKeys::Enter) { Choose(); }
    if(A.Key==EKeys::Left) { if(Row==2) { Draft.Color=(Draft.Color+5)%6; }if(Row==3) { Draft.Style=(Draft.Style+2)%3; }if(Row==4) { Draft.Decor=(Draft.Decor+2)%3; } }
    if(A.Key==EKeys::LeftMouseButton)
    {
        FVector2D Mouse;int32 W=0,H=0;PC->GetViewportSize(W,H);const double S=FMath::Min(W/1280.0,H/720.0);
        if(S>0 && GetWorld()->GetGameViewport()->GetMousePosition(Mouse)) { for(int32 I=0;I<6;++I) { if(FBox2D(FVector2D(42*S,(196+48*I)*S),FVector2D(610*S,(238+48*I)*S)).IsInside(Mouse)) { Row=I;Choose(); } } }
    }
    return true;
}
void UFiskMarinaPanel::Draw(AHUD& HUD,UCanvas& C)
{
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& Text,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){HUD.DrawText(Text,Color,X*S,Y*S,nullptr,Size*S);};
    const UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());const AFiskMarinaHub* Hub=FindHub();
    HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.97f),24*S,28*S,(Page==3?1232:610)*S,650*S);
    if(Page==3)
    {
        T(TEXT("ARTSBOK OG ARTSJAKT"),52,60,2);int32 Discovered=0;for(int32 I=0;I<FiskCatches::Count;++I) { if(GI->GetJournal().Best(I)) { ++Discovered; } }
        T(FString::Printf(TEXT("Oppdaget %d/%d arter | Dagens nye funn vises først ved 17:00"),Discovered,FiskCatches::Count),52,112,1.05f,Teal);
        for(int32 I=0;I<FiskCatches::Count;++I) { if(I==Species) { HUD.DrawRect(FLinearColor(.14f,.34f,.36f),44*S,(165+49*I)*S,370*S,43*S); }T(FString::Printf(TEXT("%s %s"),GI->GetJournal().Best(I)?TEXT("+"):TEXT("-"),FiskCatches::Find(I)->Name),58,176+49*I,1.1f); }
        const FFiskSpecies& V=*FiskCatches::Find(Species);T(V.Name,450,180,1.8f,Gold);T(FiskCatches::HabitatTip(Species),450,240,.95f,Teal);T(FiskCatches::FightTip(Species),450,287,.95f);
        T(FString::Printf(TEXT("Størrelse: %.1f–%.1f kg | Verdi $%d/kg"),V.MinGrams/1000.0,V.MaxGrams/1000.0,V.PricePerKg),450,337,1.f);
        const FFiskJournalEntry* Best=GI->GetJournal().Best(Species);T(Best?FString::Printf(TEXT("Din rekord: %.1f kg / %.1f cm"),Best->Fish.WeightGrams/1000.0,Best->Fish.LengthMm/10.0):TEXT("Ingen bekreftet fangst av denne arten ennå."),450,392,1.1f,Gold);
        T(FString::Printf(TEXT("Habitatfordeling: rev %d%% | skrent %d%% | åpent hav %d%%"),FiskCatches::HabitatChance(0,Species),FiskCatches::HabitatChance(1,Species),FiskCatches::HabitatChance(2,Species)),450,450,.9f,Teal);
        T(TEXT("Tips gjelder habitat og rigg. Ingen rigg eller reise garanterer en bestemt art."),52,575,.95f);T(TEXT("Piler / PgUp/PgDn: velg art | I / Escape: lukk | F4 utenfor panelet: journal"),52,630,.95f,Teal);return;
    }
    if(Page==2)
    {
        T(TEXT("FISKELAGETS STRANDBAR"),46,60,1.7f);T(TEXT("Velkommen hjem. Dagens fangst skjules til 17:00."),46,112,.85f,Teal);
        if(Hub) { int32 Y=158;const AFiskStarterBoat* Boat=CastChecked<AFiskBoardingController>(GetOwner())->FindStarterBoat();for(const FFiskMarinaResident& R:Hub->GetResidents()) { T(R.Identity.DisplayName+TEXT(" | ")+(R.PlayerId<0?TEXT("Frakoblet"):Boat && Boat->IsAboard(R.PlayerId) && Boat->GetTripProgress()!=-1 && Boat->GetTripProgress()!=200?TEXT("På sjøen"):TEXT("Ved marinaen")),46,Y,.95f);Y+=28; }
            int32 Start=FMath::Max(0,Hub->GetMessages().Num()-3);for(int32 I=Start;I<Hub->GetMessages().Num();++I) { const FFiskMarinaMessage& M=Hub->GetMessages()[I];T(M.Name+TEXT(": ")+AFiskMarinaHub::Phrase(M.Code),46,284+(I-Start)*34,.75f,Teal); } }
        TActorIterator<AFiskDaySettlement> Settlement(GetWorld());if(Settlement && Settlement->GetView().bRevealed) { int32 Y=401;for(const FFiskDayResult& R:Settlement->GetView().Results) { T(FString::Printf(TEXT("Spiller %d | %d levert | $%lld"),R.PlayerId,R.DeliveredFish,static_cast<long long>(R.SessionBalance)),46,Y,.85f,Gold);Y+=26; } }else { T(TEXT("Dagens tavle åpner ved 17:00."),46,420,.95f,Gold); }
        for(int32 I=0;I<5;++I) { T(FString::Printf(TEXT("%d: %s"),I+1,AFiskMarinaHub::Phrase(I)),46,535+20*I,.73f); }T(TEXT("2 s mellom meldinger. B / Escape: tilbake. Klokken går."),46,652,.78f,Teal);return;
    }
    T(TEXT("NAUST OG TROFÉVEGG"),46,55,1.65f);T(TEXT("1–4: besøk naust | Egne valg er gratis kosmetikk"),46,105,.85f,Teal);
    const FFiskMarinaResident* R=Hub && Hub->GetResidents().IsValidIndex(HouseIndex)?&Hub->GetResidents()[HouseIndex]:nullptr;
    const bool Own=R && R->Profile==LoadedProfile;T(R?R->Identity.DisplayName:TEXT("Venter på naust ..."),46,150,1.2f,Gold);
    if(Own)
    {
        const TCHAR* Colors[]={TEXT("Havblå"),TEXT("Korall"),TEXT("Sand"),TEXT("Palmegrønn"),TEXT("Lavendel"),TEXT("Krem")};const TCHAR* Styles[]={TEXT("Klassisk"),TEXT("Saltak"),TEXT("Veranda")};const TCHAR* Decor[]={TEXT("Enkelt"),TEXT("Planter"),TEXT("Benker")};
        const FString Rows[]={TEXT("Navn: ")+(bEditing && Row==0?EditText:Draft.DisplayName),TEXT("Naust: ")+(bEditing && Row==1?EditText:Draft.HouseName),TEXT("Farge: ")+FString(Colors[Draft.Color]),TEXT("Bygg: ")+FString(Styles[Draft.Style]),TEXT("Dekor: ")+FString(Decor[Draft.Decor]),TEXT("LAGRE OG VIS TIL VENNER")};
        for(int32 I=0;I<6;++I) { HUD.DrawRect(I==Row?FLinearColor(.14f,.34f,.36f):FLinearColor(.03f,.12f,.15f),42*S,(196+48*I)*S,568*S,42*S);T(Rows[I],54,209+48*I,.95f); }
        T(bEditing?TEXT("Skriv A–Z/tall, mellomrom og -. Enter: ferdig."):TEXT("Piler / klikk: endre | Enter: velg eller lagre"),46,510,.83f,Teal);T(Status,46,548,.78f,Gold);
    }
    else if(R) { T(R->Identity.HouseName,46,210,1.3f);T(TEXT("Du besøker en venn. Bare eieren kan endre naustet."),46,265,.82f,Teal);int32 Y=320;for(const FFiskMarinaTrophy& Trophy:R->Trophies) { T(FString::Printf(TEXT("%s: %.1f kg"),FiskCatches::Find(Trophy.Species)->Name,Trophy.Grams/1000.0),46,Y,.95f,Gold);Y+=32; } }
    if(bBlocked && !Own) { T(Status,46,548,.78f,Gold); }
    T(TEXT("Troféveggen bruker bekreftede rekorder. Dagens"),46,600,.8f,Teal);T(TEXT("nye rekorder vises først ved 17:00. H / Escape: lukk."),46,624,.8f,Teal);
}
