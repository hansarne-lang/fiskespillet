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
#include "Fishing/FiskPrototypeRod.h"

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
        bSent=false;bSending=false;bPendingSave=false;NextUpdate=0;SentJournalRevision=-1;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MARINA_LOADED blocked=%d recovery=%d"),bBlocked,Load==EFiskMarinaLoad::Recovered);
    }
    if(WantsCamera() && !IsAtDock()) { Show(0); }
    if(GetWorld()->GetRealTimeSeconds()<NextUpdate) { return; }NextUpdate=GetWorld()->GetRealTimeSeconds()+2;
    if(bPendingSave && FFiskMarinaStore::Save(File(),LoadedProfile,Pending)) { bPendingSave=false;Status=TEXT("Naust og navn er lagret lokalt."); }
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
    if(!Value.IsValid()) { return; }
    const bool RefreshDraft=Draft==Identity;
    bSending=false;Identity=Value;Pending=Value;
    if(RefreshDraft) { Draft=Value; }
    bPendingSave=!FFiskMarinaStore::Save(File(),LoadedProfile,Value);
    Status=bPendingSave?TEXT("Naust bekreftet av verten. Lokal lagring prøves igjen."):TEXT("Naust og navn er bekreftet og lagret.");
}
void UFiskMarinaPanel::Show(int32 NewPage)
{
    Page=NewPage;bShopPreview=false;ShopCategory=0;ShopLevel=0;bEditing=false;Row=0;Draft=Identity;AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    PC->ReleaseRodInputsForMenu();PC->bShowMouseCursor=Page!=0;
    if(Page) { PC->SetInputMode(FInputModeGameAndUI()); }else { PC->SetInputMode(FInputModeGameOnly()); }
    if(Page==1 || Page==5) { if(const AFiskMarinaHub* H=FindHub()) { const int32 Own=H->GetResidents().IndexOfByPredicate([&](const FFiskMarinaResident& R){return R.Profile==LoadedProfile;});HouseIndex=FMath::Max(0,Own); } }
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
        if(!Press || (A.Key!=EKeys::H && A.Key!=EKeys::B && A.Key!=EKeys::I && A.Key!=EKeys::U)) { return false; }
        if(PC->GetFrontEnd()->IsVisible() || PC->GetSeaChart()->IsVisible() || PC->GetCrewPanel()->IsVisible() || PC->GetRigPanel()->IsVisible() || PC->GetJournalPanel()->IsVisible() || PC->GetDeckPanel()->IsVisible() || PC->IsCareerVisible() || PC->IsHelpVisible() || PC->IsResultsVisible() || PC->IsHistoryVisible()) { return false; }
        if(A.Key!=EKeys::I && !IsAtDock()) { PC->ClientActionFeedback(TEXT("Naust og strandbar brukes ved marinaen."));return true; }
        Show(A.Key==EKeys::H?1:A.Key==EKeys::B?2:A.Key==EKeys::U?4:3);return true;
    }
    if(!Press) { return true; }
    if(bEditing)
    {
        if(A.Key==EKeys::Escape) { bEditing=false;return true; }
        if(A.Key==EKeys::Enter) { if(FFiskCareerProfile::ValidName(EditText)) { if(Page==4) { Status=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance())->RenameRigPreset(PresetSlot,EditText)?TEXT("Oppsettsnavnet er lagret."):TEXT("Kunne ikke lagre navnet. Filene er bevart."); }else if(Row==0) { Draft.DisplayName=EditText; }else { Draft.HouseName=EditText; }bEditing=false; }return true; }
        if(A.Key==EKeys::BackSpace) { EditText=EditText.LeftChop(1);return true; }
        FString C=A.Key.GetFName().ToString();if(A.Key==EKeys::SpaceBar) { C=TEXT(" "); }if(A.Key==EKeys::Hyphen) { C=TEXT("-"); }
        const FKey Digits[]={EKeys::Zero,EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};
        const FKey Pad[]={EKeys::NumPadZero,EKeys::NumPadOne,EKeys::NumPadTwo,EKeys::NumPadThree,EKeys::NumPadFour,EKeys::NumPadFive,EKeys::NumPadSix,EKeys::NumPadSeven,EKeys::NumPadEight,EKeys::NumPadNine};
        for(int32 I=0;I<10;++I) { if(A.Key==Digits[I] || A.Key==Pad[I]) { C=FString::FromInt(I); } }
        if(C.Len()==1 && (FChar::IsAlnum(C[0]) || C==TEXT(" ") || C==TEXT("-")) && EditText.Len()<24) { EditText+=C.ToUpper(); }return true;
    }
    if(A.Key==EKeys::Escape || (Page==1 && A.Key==EKeys::H) || (Page==2 && A.Key==EKeys::B) || (Page==3 && A.Key==EKeys::I) || (Page==4 && A.Key==EKeys::U)) { Show(0);return true; }
    if(Page==1 && A.Key==EKeys::N) { Show(5);return true; }
    if(Page==5)
    {
        if(A.Key==EKeys::H || A.Key==EKeys::N) { Show(1);return true; }
        if(A.Key==EKeys::V) { bShopPreview=!bShopPreview;return true; }
        if(A.Key==EKeys::Home) { ShopCategory=ShopLevel=0; }
        if(A.Key==EKeys::Left) { ShopCategory=(ShopCategory+2)%3; }if(A.Key==EKeys::Right) { ShopCategory=(ShopCategory+1)%3; }
        if(A.Key==EKeys::Up) { ShopLevel=(ShopLevel+3)%4; }if(A.Key==EKeys::Down || A.Key==EKeys::Tab) { ShopLevel=(ShopLevel+1)%4; }
        if(A.Key==EKeys::Enter) { ChooseHomeItem(); }return true;
    }
    if(Page==1 && A.Key==EKeys::U) { Show(4);return true; }
    if(Page==4)
    {
        auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());const FKey Slots[]={EKeys::One,EKeys::Two,EKeys::Three};
        for(int32 I=0;I<3;++I) { if(A.Key==Slots[I]) { PresetSlot=I; } }
        if(A.Key==EKeys::H) { Show(1);return true; }const auto* Saved=GI->RigPresets.Get(PresetSlot);
        if(A.Key==EKeys::F2 && Saved && Saved->bExists) { bEditing=true;EditText=Saved->Name; }
        if(A.Key==EKeys::F5) { Status=PC->GetRod() && GI->SaveRigPreset(PresetSlot,PC->GetRod()->GetRig())?TEXT("Bekreftet stangoppsett er lagret."):TEXT("Kunne ikke lagre oppsettet. Filene er bevart."); }
        if(A.Key==EKeys::F9 && Saved && Saved->bExists)
        {
            if(PC->CanOpenRig()) { PC->RequestRig(Saved->Rig.Bait,Saved->Rig.Depth,Saved->Rig.Program);Status=TEXT("Sendt til verten. Se bekreftede valg i G."); }
            else { Status=TEXT("Gå om bord og velg en ledig egen stang før du henter oppsett."); }
        }return true;
    }
    const AFiskMarinaHub* Hub=FindHub();
    if(Page==3) { if(A.Key==EKeys::Right || A.Key==EKeys::Down || A.Key==EKeys::PageDown) { Species=(Species+1)%FiskCatches::Count; }if(A.Key==EKeys::Left || A.Key==EKeys::Up || A.Key==EKeys::PageUp) { Species=(Species+FiskCatches::Count-1)%FiskCatches::Count; }return true; }
    const FKey Numbers[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five};
    if(Page==2) { for(int32 I=0;I<5;++I) { if(A.Key==Numbers[I]) { PC->ServerMarinaSay(I); } }return true; }
    if(Page==1 && Hub && !Hub->GetResidents().IsEmpty() && (A.Key==EKeys::PageDown || A.Key==EKeys::PageUp))
    { HouseIndex=(HouseIndex+(A.Key==EKeys::PageDown?1:Hub->GetResidents().Num()-1))%Hub->GetResidents().Num();Draft=Identity;Row=0;return true; }
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
    if(Page==5) { if(!bShopPreview) { HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.97f),24*S,28*S,610*S,650*S); }DrawHomeShop(HUD,C);return; }
    HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.97f),24*S,28*S,(Page==3 || Page==4?1232:610)*S,650*S);
    if(Page==4)
    {
        T(TEXT("DITT UTSTYRSROM"),48,58,2);T(TEXT("Eid utstyr og tre personlige oppsett. P viser butikk og priser."),48,112,1.f,Teal);
        T(TEXT("Saldo: ")+GI->Career.Money(),48,160,1.2f,Gold);
        for(int32 I=0;I<4;++I) { T(FFiskCareerProfile::ItemName(I)+TEXT(": ")+(GI->Career.Has(I)?TEXT("EID"):TEXT("Ikke kjøpt")),48,207+35*I,1.f); }
        int32 Found=0;for(int32 I=0;I<FiskCatches::Count;++I) { Found+=GI->GetJournal().Best(I)?1:0; }
        T(FString::Printf(TEXT("Artsfunn %d/%d | I: artsbok utenfor panelet"),Found,FiskCatches::Count),48,375,.95f,Teal);
        T(TEXT("Startbåten er alltid tilgjengelig."),48,415,.95f);T(TEXT("Offshorebåten krever eget kjøp og nok plass."),48,451,.9f,Teal);
        const auto* PC=CastChecked<AFiskBoardingController>(GetOwner());T(FString::Printf(TEXT("Valgt egen stang: %d"),PC->GetSelectedRod()+1),650,160,1.15f,Gold);
        for(int32 I=0;I<3;++I)
        {
            const auto* Saved=GI->RigPresets.Get(I);HUD.DrawRect(I==PresetSlot?FLinearColor(.14f,.34f,.36f):FLinearColor(.03f,.12f,.15f),640*S,(204+92*I)*S,570*S,82*S);
            T(FString::Printf(TEXT("%d  %s"),I+1,*(bEditing && I==PresetSlot?EditText:Saved->Name)),654,219+92*I,1.1f);
            T(Saved->bExists?FString::Printf(TEXT("%s | %d m | %s"),FFiskRigRules::BaitName(Saved->Rig.Bait),Saved->Rig.Depth,FFiskRigRules::ProgramName(Saved->Rig.Program)):TEXT("Tom plass. F5 lagrer valgte stangs bekreftede oppsett."),654,252+92*I,.85f,Teal);
        }
        T(bEditing?TEXT("Skriv navn, Enter: lagre. Escape: avbryt."):TEXT("1/2/3: plass | F2: navn | F5: lagre | F9: hent til egen stang"),48,525,.95f,Teal);
        T(Status,48,568,.9f,Gold);T(TEXT("U / Escape: lukk | H: naust | Klokken fortsetter. Hentede valg kontrolleres av verten."),48,635,.9f,Teal);return;
    }
    if(Page==3)
    {
        T(TEXT("ARTSBOK OG ARTSJAKT"),52,60,2);int32 Discovered=0;for(int32 I=0;I<FiskCatches::Count;++I) { if(GI->GetJournal().Best(I)) { ++Discovered; } }
        T(FString::Printf(TEXT("Oppdaget %d/%d arter | Dagens nye funn vises først ved 17:00"),Discovered,FiskCatches::Count),52,112,1.05f,Teal);
        for(int32 I=(Species/8)*8;I<FMath::Min((Species/8+1)*8,FiskCatches::Count);++I) { if(I==Species) { HUD.DrawRect(FLinearColor(.14f,.34f,.36f),44*S,(165+44*(I%8))*S,370*S,38*S); }T(FString::Printf(TEXT("%s %s"),GI->GetJournal().Best(I)?TEXT("+"):TEXT("-"),FiskCatches::Find(I)->Name),58,173+44*(I%8),1.03f); }
        const FFiskSpecies& V=*FiskCatches::Find(Species);T(V.Name,450,180,1.8f,Gold);T(FiskCatches::HabitatTip(Species),450,240,.95f,Teal);T(FiskCatches::FightTip(Species),450,287,.95f);
        T(FString::Printf(TEXT("Størrelse: %.1f–%.1f kg | Verdi $%d/kg"),V.MinGrams/1000.0,V.MaxGrams/1000.0,V.PricePerKg),450,337,1.f);
        const FFiskJournalEntry* Best=GI->GetJournal().Best(Species);T(Best?FString::Printf(TEXT("Din rekord: %.1f kg / %.1f cm"),Best->Fish.WeightGrams/1000.0,Best->Fish.LengthMm/10.0):TEXT("Ingen bekreftet fangst av denne arten ennå."),450,392,1.1f,Gold);
        T(FString::Printf(TEXT("Habitatfordeling: rev %d%% | skrent %d%% | hav %d%% | mangrove %d%% | dyphav %d%%"),FiskCatches::HabitatChance(0,Species),FiskCatches::HabitatChance(1,Species),FiskCatches::HabitatChance(2,Species),FiskCatches::HabitatChance(3,Species),FiskCatches::HabitatChance(4,Species)),450,450,.8f,Teal);
        T(TEXT("Tips gjelder habitat og rigg. Ingen rigg eller reise garanterer en bestemt art."),52,575,.95f);T(TEXT("Piler / PgUp/PgDn: velg blant 15 arter | I / Escape: lukk | F4 utenfor panelet: journal"),52,630,.95f,Teal);return;
    }
    if(Page==2)
    {
        T(TEXT("FISKELAGETS STRANDBAR"),46,60,1.7f);T(TEXT("Velkommen hjem. Dagens fangst skjules til 17:00."),46,112,.85f,Teal);
        if(Hub)
        {
            const auto* PC=CastChecked<AFiskBoardingController>(GetOwner());int32 I=0;
            for(const FFiskMarinaResident& R:Hub->GetResidents())
            {
                const AFiskStarterBoat* Aboard=nullptr;for(auto* B:PC->FleetBoats()) { if(B->IsAboard(R.PlayerId)) { Aboard=B;break; } }
                const bool Sea=Aboard && Aboard->GetTripProgress()!=-1 && Aboard->GetTripProgress()!=200;
                T(R.Identity.DisplayName+TEXT(" | ")+(R.PlayerId<0?TEXT("Frakoblet"):Sea?TEXT("På sjøen"):TEXT("Ved marinaen")),46+(I/6)*285,158+(I%6)*26,.84f);++I;
            }
            const int32 Start=FMath::Max(0,Hub->GetMessages().Num()-3);
            for(int32 MessageIndex=Start;MessageIndex<Hub->GetMessages().Num();++MessageIndex) { const auto& M=Hub->GetMessages()[MessageIndex];T(M.Name+TEXT(": ")+AFiskMarinaHub::Phrase(M.Code),46,328+(MessageIndex-Start)*27,.72f,Teal); }
        }
        TActorIterator<AFiskDaySettlement> Settlement(GetWorld());
        if(Settlement && Settlement->GetView().bRevealed) { int32 I=0;for(const FFiskDayResult& R:Settlement->GetView().Results) { T(FString::Printf(TEXT("Spiller %d | %d levert | $%lld"),R.PlayerId,R.DeliveredFish,static_cast<long long>(R.SessionBalance)),46+(I/4)*285,425+(I%4)*24,.75f,Gold);++I; } }
        else { T(TEXT("Dagens tavle åpner ved 17:00. Tab viser flåten utenfor baren."),46,440,.85f,Gold); }
        for(int32 I=0;I<5;++I) { T(FString::Printf(TEXT("%d: %s"),I+1,AFiskMarinaHub::Phrase(I)),46,535+20*I,.73f); }T(TEXT("2 s mellom meldinger. B / Escape: tilbake. Klokken går."),46,652,.78f,Teal);return;
    }
    T(TEXT("NAUST OG TROFÉVEGG"),46,55,1.65f);T(TEXT("PgUp/PgDn: alle 12 | 1–4: besøk | N: naustbutikk"),46,105,.85f,Teal);
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
    T(TEXT("N: kjøp/velg lys, brygge og troféutstilling. Dagens"),46,600,.8f,Teal);T(TEXT("nye rekorder vises først ved 17:00. H / Escape: lukk."),46,624,.8f,Teal);
}
