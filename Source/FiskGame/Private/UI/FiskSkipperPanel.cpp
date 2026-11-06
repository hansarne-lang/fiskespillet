#include "UI/FiskSkipperPanel.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskUserSettings.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Voyage/FiskVoyagePlan.h"
#include "World/FiskOceanAtlas.h"
#include "FiskRouteGeometry.h"
#include "FiskFishingGround.h"
#include "FiskCareerProfile.h"
#include "DayCycle/FiskDayState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/HUD.h"
#include "Camera/CameraTypes.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
FString UFiskSkipperPanel::PresetFile() const
{ return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Routes-")+LoadedProfile.ToString(EGuidFormats::Digits)+TEXT(".ini")); }
void UFiskSkipperPanel::LoadPresets()
{
    const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());if(LoadedProfile==GI->Career.Id) { return; }
    LoadedProfile=GI->Career.Id;Presets.Load(PresetFile(),LoadedProfile);
    if(Presets.bBlocked) { Message=TEXT("Rutefil kan ikke leses. Eksisterende filer er bevart."); }
    else if(Presets.bRecovered) { Message=TEXT("Egne ruter hentet fra backup. F5 lagrer valgt oppsett på nytt."); }
}
void UFiskSkipperPanel::Open(int32 V)
{
    Page=V;DetailPage=0;bRenaming=false;if(V==3) { bPhotoHud=true; }LoadPresets();CastChecked<AFiskBoardingController>(GetOwner())->ReleaseRodInputsForMenu();
}
void UFiskSkipperPanel::PhotoCamera(const FTransform& B,FMinimalViewInfo& Out) const
{ const FVector Focus=B.TransformPosition(FVector(0,0,130));Out.Location=Focus+Photo.Offset(B.Rotator().Yaw);Out.Rotation=(Focus-Out.Location).Rotation();Out.FOV=Photo.Fov; }
bool UFiskSkipperPanel::HandleKey(const FInputKeyEventArgs& A)
{
    if(Page==0)
    {
        if(A.Event!=IE_Pressed) { return false; }
        if(A.Key!=EKeys::O && A.Key!=EKeys::Y && A.Key!=EKeys::F9) { return false; }
        auto* PC=CastChecked<AFiskBoardingController>(GetOwner());const auto* B=PC->FindStarterBoat();
        if(A.Key==EKeys::F9 && (!B || !PC->PlayerState || !B->IsAboard(PC->PlayerState->GetPlayerId()))) { PC->ClientActionFeedback(TEXT("Gå om bord med E før du åpner fotomodus."));return true; }
        Open(A.Key==EKeys::O?1:A.Key==EKeys::Y?2:3);return true;
    }
    if(A.Event!=IE_Pressed && A.Event!=IE_Repeat) { return true; }const FKey K=A.Key;auto* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if(bRenaming)
    {
        if(K==EKeys::Escape) { bRenaming=false;return true; }
        if(K==EKeys::Enter)
        {
            auto Candidate=Presets;if(FFiskCareerProfile::ValidName(EditName)) { Candidate.Slots[Slot].Name=EditName;if(Candidate.Save(PresetFile(),LoadedProfile)) { Presets=Candidate;Message=TEXT("Rutenavn lagret.");bRenaming=false; }else { Message=TEXT("Lagring avvist. Eksisterende filer er bevart."); } }return true;
        }
        if(K==EKeys::BackSpace) { EditName.LeftChopInline(1); }else if(K==EKeys::SpaceBar && EditName.Len()<24) { EditName+=TEXT(" "); }
        else
        {
            const FKey Digits[]={EKeys::Zero,EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};FString V=K.ToString();
            for(int32 I=0;I<10;++I) { if(K==Digits[I]) { V=FString::FromInt(I);break; } }
            if(V.Len()==1 && FChar::IsAlnum(V[0]) && EditName.Len()<24) { EditName+=V.ToUpper(); }
        }return true;
    }
    if(K==EKeys::Escape || (Page==1 && K==EKeys::O) || (Page==2 && K==EKeys::Y) || (Page==3 && K==EKeys::F9)) { Page=0;return true; }
    if(Page==3)
    {
        if(K==EKeys::Left) { Photo.Yaw-=10; }if(K==EKeys::Right) { Photo.Yaw+=10; }if(K==EKeys::Up) { Photo.Pitch+=5; }if(K==EKeys::Down) { Photo.Pitch-=5; }
        if(K==EKeys::PageUp) { Photo.Distance-=200; }if(K==EKeys::PageDown) { Photo.Distance+=200; }
        if(K==EKeys::MouseScrollUp) { Photo.Fov-=5; }if(K==EKeys::MouseScrollDown) { Photo.Fov+=5; }
        if(K==EKeys::Home) { Photo=FFiskPhotoView(); }if(K==EKeys::H && A.Event==IE_Pressed) { bPhotoHud=!bPhotoHud; }
        Photo.Clamp();
        if(K==EKeys::F12 && A.Event==IE_Pressed && !FScreenshotRequest::IsScreenshotRequested())
        {
            const FString Dir=FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Photos"));
            if(IFileManager::Get().MakeDirectory(*Dir,true))
            {
                LastPhoto=FPaths::Combine(Dir,TEXT("FISKELAGET-")+FDateTime::UtcNow().ToString(TEXT("%Y%m%d-%H%M%S-"))+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT(".png"));
                FScreenshotRequest::RequestScreenshot(LastPhoto,false,false);Message=TEXT("Bilde forespurt i Fiskelaget/Photos. H skjuler fototeksten før neste bilde.");
                UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PHOTO_REQUEST file=%s"),*LastPhoto);
            }
            else { Message=TEXT("Kunne ikke opprette bildemappen."); }
        }return true;
    }
    const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    if(Page==2)
    {
        const auto Days=FiskTripBook::Days(GI->GetJournal());if(K==EKeys::Tab) { Tab=(Tab+1)%4;DetailPage=0; }
        if(K==EKeys::Up) { DayRow=FMath::Max(0,DayRow-1);DetailPage=0; }if(K==EKeys::Down) { DayRow=FMath::Min(FMath::Max(0,Days.Num()-1),DayRow+1);DetailPage=0; }
        if(K==EKeys::PageDown) { DetailPage=FMath::Min(DetailPage+1,1024); }if(K==EKeys::PageUp) { DetailPage=FMath::Max(0,DetailPage-1); }
        if(K==EKeys::F10 && A.Event==IE_Pressed && Days.IsValidIndex(DayRow))
        { FString File;const bool OK=FiskTripBook::Export(FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Exports")),GI->GetJournal(),Days[DayRow].Id,File);Message=OK?TEXT("Valgt tur er eksportert til Fiskelaget/Exports som HTML."):TEXT("Rapport kunne ikke lagres.");UE_LOG(LogTemp,Display,TEXT("FISKELAGET_TRIP_REPORT success=%d file=%s"),OK,*File); }return true;
    }
    TActorIterator<AFiskVoyagePlan> P(GetWorld());if(!P) { return true; }const auto R=P->GetRoute();
    if(K==EKeys::Up) { Mark=(Mark+8)%9; }if(K==EKeys::Down) { Mark=(Mark+1)%9; }
    if(K==EKeys::One || K==EKeys::Two || K==EKeys::Three) { Slot=K==EKeys::One?0:K==EKeys::Two?1:2; }
    if(A.Event!=IE_Pressed) { return true; }
    if(K==EKeys::A) { auto Points=R.Points;Points.Add(FiskAtlas::Find(Mark)->Point);PC->ServerVoyageReplace(R.Day,R.Revision,Points); }
    if(K==EKeys::BackSpace && !R.Points.IsEmpty()) { auto Points=R.Points;Points.Pop();PC->ServerVoyageReplace(R.Day,R.Revision,Points); }
    if(K==EKeys::Delete) { PC->ServerVoyageReplace(R.Day,R.Revision,{}); }
    if(K==EKeys::Enter) { if(const auto* B=PC->FindStarterBoat()) { PC->ServerVoyageNext(R.Day,R.Revision,B->GetRouteRevision()); } }
    if(K==EKeys::F2) { bRenaming=true;EditName=Presets.Slots[Slot].Name; }
    if(K==EKeys::F5)
    {
        auto Candidate=Presets;Candidate.Slots[Slot].Points=R.Points;
        if(Candidate.Save(PresetFile(),LoadedProfile)) { Presets=Candidate;Message=TEXT("Felles plan er lagret som ditt eget ruteoppsett."); }
        else { Message=TEXT("Ruten kunne ikke lagres. Tidligere filer er bevart."); }
    }
    if(K==EKeys::F8) { if(Presets.Slots[Slot].Points.IsEmpty()) { Message=TEXT("Det valgte oppsettet har ingen stopp."); }else { PC->ServerVoyageReplace(R.Day,R.Revision,Presets.Slots[Slot].Points); } }
    return true;
}
void UFiskSkipperPanel::Draw(AHUD& HUD,UCanvas& C)
{
    if(Page==1) { DrawPlan(HUD,C);return; }if(Page==2) { DrawBook(HUD,C);return; }if(Page!=3 || !bPhotoHud) { return; }
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const auto* D=GetWorld()->GetGameState<AFiskDayState>();const int32 Ms=D?D->GetElapsedMilliseconds():0;
    HUD.DrawRect(FLinearColor(.01f,.04f,.06f,.94f),24*S,548*S,1232*S,146*S);
    HUD.DrawText(FString::Printf(TEXT("FOTOMODUS | %02d:%02d | Klokken går videre | %.0f graders utsnitt"),Ms/60000,Ms/1000%60,Photo.Fov),FLinearColor::White,48*S,568*S,nullptr,1.1f*S);
    HUD.DrawText(TEXT("Piler: vinkel | PgUp/PgDn: avstand | Musehjul: bildeutsnitt | Home: nullstill"),FLinearColor(.5f,.9f,.84f),48*S,607*S,nullptr,1.f*S);
    HUD.DrawText(TEXT("F12: ta PNG-bilde | H: skjul/vis denne teksten | F9 / Escape: tilbake til fiske"),FLinearColor::White,48*S,645*S,nullptr,1.f*S);
}
void UFiskSkipperPanel::DrawPlan(AHUD& H,UCanvas& C)
{
    TActorIterator<AFiskVoyagePlan> P(GetWorld());if(!P) { return; }const auto& R=P->GetRoute();const auto* PC=CastChecked<AFiskBoardingController>(GetOwner());const auto* B=PC->FindStarterBoat();
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){H.DrawText(V,Color,X*S,Y*S,nullptr,Size*S);};
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);T(TEXT("SKIPPERENS TURPLAN"),48,55,2.f);
    T(FString::Printf(TEXT("Dag %d | %d av %d stopp besøkt | %s"),R.Day,R.Next,R.Points.Num(),B && PC->PlayerState && B->GetCaptainId()==PC->PlayerState->GetPlayerId()?TEXT("Du er kaptein"):TEXT("Mannskapet følger kapteinens plan")),48,105,1.1f,Teal);
    T(TEXT("Piler: atlasmerke | A: legg til | Backspace: fjern siste | Delete: tøm | Enter: velg neste stopp"),48,143,.94f);
    for(int32 I=0;I<9;++I) { const float Y=185+35*I;H.DrawRect(I==Mark?FLinearColor(.14f,.34f,.36f):FLinearColor(.03f,.12f,.15f),48*S,Y*S,305*S,31*S);T(FiskAtlas::Find(I)->Name,60,Y+6,.98f,I==Mark?Gold:Teal); }
    for(int32 I=0;I<5;++I)
    {
        FString Name=TEXT("Ledig stopp");if(R.Points.IsValidIndex(I)) { const int32 A=FiskAtlas::Nearest(R.Points[I]);Name=A>=0?FiskAtlas::Find(A)->Name:TEXT("Eget sjøpunkt"); }
        T(FString::Printf(TEXT("%d. %s %s"),I+1,*Name,I<R.Next?TEXT("[BESØKT]"):I==R.Active?TEXT("[VALGT]"):TEXT("")),385,187+44*I,1.1f,I==R.Next?Gold:Teal);
    }
    auto Point=[&](FVector V){return FVector2D((875+V.X/150000*330)*S,(330-V.Y/100000*130)*S);};
    H.DrawRect(FLinearColor(.025f,.19f,.25f),850*S,175*S,365*S,315*S);
    for(int32 I=0;I<FiskRouteGeometry::IslandCount;++I) { C.K2_DrawPolygon(nullptr,Point(FiskRouteGeometry::Island(I)),FVector2D(FiskRouteGeometry::Radius(I)/150000*330*S,FiskRouteGeometry::Radius(I)/100000*130*S),24,FLinearColor(.52f,.43f,.24f)); }
    FVector From=FVector::ZeroVector;double Length=0;
    for(int32 I=0;I<R.Points.Num();++I) { const auto Path=FiskRouteGeometry::Between(From,R.Points[I]);Length+=FiskRouteGeometry::Length(Path);for(int32 J=1;J<Path.Num();++J) { const auto A=Point(Path[J-1]),Z=Point(Path[J]);H.DrawLine(A.X,A.Y,Z.X,Z.Y,Teal,2*S); }const auto Q=Point(R.Points[I]);T(FString::FromInt(I+1),Q.X/S,Q.Y/S,.9f,Gold);From=R.Points[I]; }
    Length+=FiskRouteGeometry::Length(FiskRouteGeometry::Between(From,FVector::ZeroVector));T(FString::Printf(TEXT("Hele planlagte ruten: %.2f km inkl. hjemreise"),Length/100000),385,438,.95f);
    if(B) { const auto Q=Point(B->GetActorLocation());H.DrawRect(FLinearColor::White,Q.X-4*S,Q.Y-4*S,8*S,8*S); }
    const auto& Saved=Presets.Slots[Slot];T(FString::Printf(TEXT("EGEN RUTE %d: %s | %d stopp"),Slot+1,*Saved.Name,Saved.Points.Num()),48,525,1.1f,Gold);
    T(bRenaming?TEXT("Nytt navn: ")+EditName+TEXT(" | Enter: lagre | Escape: avbryt"):TEXT("1/2/3: oppsett | F5: lagre felles plan | F8: hent til server | F2: navn"),48,561,.98f,Teal);
    const FString Feedback=PC->GetActionFeedback();T(Feedback.IsEmpty()?Message:Feedback,48,599,.9f,Gold);
    T(TEXT("Rediger ved brygga. Enter ved fiskeplass flytter bare når alle stenger er trygge. Lukk O før R."),48,642,.91f);
}
