#include "UI/FiskSeaChart.h"
#include "World/FiskOceanAtlas.h"
#include "FiskFishingGround.h"
#include "FiskRouteGeometry.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"

bool UFiskSeaChart::HandleAtlas(const FInputKeyEventArgs& A)
{
    if(A.Event!=IE_Pressed && A.Event!=IE_Repeat) { return true; }auto* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if(A.Key==EKeys::Escape) { Show(false);return true; }
    if(A.Key==EKeys::Up) { AtlasRow=(AtlasRow+FiskAtlas::Count-1)%FiskAtlas::Count; }if(A.Key==EKeys::Down) { AtlasRow=(AtlasRow+1)%FiskAtlas::Count; }
    if(A.Key==EKeys::Home) { AtlasRow=0; }if(A.Key==EKeys::End) { AtlasRow=FiskAtlas::Count-1; }
    if(A.Key==EKeys::PageDown) { AtlasRow=FMath::Min(AtlasRow+8,FiskAtlas::Count-1); }if(A.Key==EKeys::PageUp) { AtlasRow=FMath::Max(AtlasRow-8,0); }
    const FKey Keys[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};
    for(int32 I=0;I<9;++I) { if(A.Key==Keys[I]) { AtlasRow=I; } }
    bool Choose=A.Key==EKeys::Enter;
    if(A.Key==EKeys::LeftMouseButton)
    {
        FVector2D M;if(GetWorld()->GetGameViewport()->GetMousePosition(M))
        { for(int32 I=0;I<Buttons.Num();++I) { if(Buttons[I].IsInside(M)) { AtlasRow=(AtlasRow/8)*8+I;Choose=true;break; } }
          for(int32 I=0;I<Points.Num();++I) { if(Points[I].IsInside(M)) { AtlasRow=I;Choose=true;break; } } }
    }
    if(Choose) { if(const auto* B=PC->FindStarterBoat()) { Message=TEXT("Venter på vertens rutevalg ...");PC->GetBoatCommands()->ServerSelectSeaPoint(B->GetRouteRevision(),FiskAtlas::Find(AtlasRow)->Point); } }
    return true;
}
void UFiskSeaChart::DrawAtlas(AHUD& HUD,UCanvas& C)
{
    const auto* PC=CastChecked<AFiskBoardingController>(GetOwner());const auto* B=PC->FindStarterBoat();const auto* D=GetWorld()->GetGameState<AFiskDayState>();if(!B || !D) { return; }
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){HUD.DrawText(V,Color,X*S,Y*S,nullptr,Size*S);};
    HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);T(TEXT("HAV-ATLAS"),48,56,2.1f);
    T(TEXT("Piler / PgUp/PgDn: alle 16 | Home/End: første/siste | Enter / klikk: velg | A: vanlig kart | Tab: favoritter"),48,110,.98f,Teal);
    Buttons.Reset();Points.Reset();
    const int32 First=(AtlasRow/8)*8;
    for(int32 I=First;I<FMath::Min(First+8,FiskAtlas::Count);++I)
    {
        const auto& Mark=*FiskAtlas::Find(I);int32 Habitat=0,Travel=0;FiskGrounds::MakeSeaPoint(Mark.Point,Habitat,Travel);const auto& G=*FiskGrounds::Find(Habitat);
        const float Y=157+43*(I-First);Buttons.Add(FBox2D(FVector2D(48*S,Y*S),FVector2D(575*S,(Y+39)*S)));
        HUD.DrawRect(AtlasRow==I?FLinearColor(.14f,.34f,.36f):FLinearColor(.03f,.12f,.15f),48*S,Y*S,527*S,39*S);
        T(FString::Printf(TEXT("%d  %s"),I+1,Mark.Name),60,Y+4,1.f);T(FString::Printf(TEXT("%s | %d m bunn | Hjem ca. %d s"),G.Name,G.BottomDepth,(B->GetKind()==1?Travel*4/5:Travel)/1000),60,Y+23,.75f,Teal);
    }
    T(FString::Printf(TEXT("Side %d/%d | %d merker"),First/8+1,FMath::DivideAndRoundUp(FiskAtlas::Count,8),FiskAtlas::Count),48,515,.85f,Teal);
    auto Point=[&](FVector P){return FVector2D((638+P.X/150000.0*560)*S,(355-P.Y/100000.0*180)*S);};
    HUD.DrawRect(FLinearColor(.025f,.19f,.25f),610*S,156*S,614*S,398*S);
    for(int32 I=0;I<FiskRouteGeometry::IslandCount;++I) { C.K2_DrawPolygon(nullptr,Point(FiskRouteGeometry::Island(I)),FVector2D(FiskRouteGeometry::Radius(I)/150000*560*S,FiskRouteGeometry::Radius(I)/100000*180*S),32,FLinearColor(.52f,.43f,.24f)); }
    const auto Path=B->GetActivePath();for(int32 I=1;I<Path.Num();++I) { const auto A=Point(Path[I-1]),Z=Point(Path[I]);HUD.DrawLine(A.X,A.Y,Z.X,Z.Y,Teal,2*S); }
    for(int32 I=0;I<FiskAtlas::Count;++I) { const auto P=Point(FiskAtlas::Find(I)->Point);Points.Add(FBox2D(P-FVector2D(12*S,12*S),P+FVector2D(12*S,12*S)));HUD.DrawRect(I==AtlasRow?Gold:Teal,P.X-5*S,P.Y-5*S,10*S,10*S);T(FString::FromInt(I+1),P.X/S+10,P.Y/S-8,.95f); }
    const auto Ship=Point(B->GetActorLocation());HUD.DrawRect(FLinearColor::White,Ship.X-4*S,Ship.Y-4*S,8*S,8*S);T(TEXT("Marina"),614,375,.8f);
    const auto& Mark=*FiskAtlas::Find(AtlasRow);T(Mark.Name,610,575,1.25f,Gold);T(Mark.Tip,610,610,.88f,Teal);
    T(Message.IsEmpty()?TEXT("Ved fiskeplassen starter valg flytting når alle stenger er trygge."):Message,48,566,.82f,Gold);
    T(TEXT("Flytting bruker tid. Trekk inn stengene med X, lukk M før R for retur."),48,602,.86f,Teal);
    T(TEXT("M / Escape: lukk | R underveis: vend hjem | V underveis: stem på reisevisning"),48,645,.9f,Teal);
}
