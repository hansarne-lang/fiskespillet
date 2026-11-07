#include "World/FiskMarinaHub.h"
#include "World/FiskVisualPrimitives.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"

void AFiskMarinaHub::BuildHouseDetails(int32 Slot,const FFiskMarinaIdentity& V)
{
    const FVector P=HousePosition(Slot);const FLinearColor Wood(.33f,.19f,.09f),Trim(.85f,.81f,.67f),Brass(.65f,.43f,.12f),Dark(.07f,.1f,.12f);
    auto Part=[&](UStaticMesh* Mesh,FVector Q,FVector Scale,FLinearColor Color)
    { auto* C=FiskVisual::AddPart(this,RootComponent,Mesh,Material,*FString::Printf(TEXT("HomeDetail%d_%d"),BuildRevision,DetailParts.Num()),P+Q,Scale,Color);DetailParts.Add(C);return C; };
    auto Line=[&](FVector A,FVector B,double Radius,FLinearColor Color)
    { auto* C=Part(Cylinder,FVector::ZeroVector,FVector::OneVector,Color);FiskVisual::SetCylinder(C,P+A,P+B,Radius); };
    // Thin separated boards and trim make existing free styles read as buildings.
    for(int32 I=0;I<22;++I) { Part(Cube,FVector(-610+I*58,-150,124),FVector(.54,8.6,.08),Wood); }
    for(int32 Z=0;Z<12;++Z) { Part(Cube,FVector(0,239,145+Z*39),FVector(9.6,.07,.06),Trim); }
    for(int32 Side:{-1,1})
    {
        Part(Cube,FVector(Side*476,-40,365),FVector(.12,2.6,2.5),Dark);
        for(int32 Z:{242,365,488}) { Part(Cube,FVector(Side*463,-40,Z),FVector(.2,2.9,.12),Trim); }
        Part(Cube,FVector(Side*463,-40,365),FVector(.2,.12,2.6),Trim);
        Part(Cube,FVector(Side*488,-300,366),FVector(.28,.28,5.1),Trim);
        Part(Cube,FVector(Side*488,260,366),FVector(.28,.28,5.1),Trim);
        Part(Cube,FVector(Side*305,-70,137),FVector(2,1.9,.22),Wood);
        for(int32 Leg:{-1,1}) { Part(Cube,FVector(Side*305+Leg*75,-70,111),FVector(.14,1.7,.5),Dark); }
        Part(Cube,FVector(Side*240,230,367),FVector(.1,.12,3.2),Brass);
        Part(Sphere,FVector(Side*240,219,524),FVector(.24,.12,.28),Brass);
    }
    Part(Cube,FVector(0,-304,613),FVector(10.8,.3,.3),Trim);
    Part(Cube,FVector(0,260,650),FVector(11,.18,.18),Trim);
    // Purchases change only presentation; no route, capacity or fishing stats.
    if(V.Pier>0)
    {
        const int32 Boards=V.Pier==3?26:20;
        for(int32 I=0;I<Boards;++I) { Part(Cube,FVector(0,-620-I*43,110),FVector(V.Pier==3?5.4:3.5,.4,.13),Wood); }
        const double End=-620-(Boards-1)*43;
        for(int32 Side:{-1,1}) { for(int32 I=0;I<4;++I)
        {
            const FVector Q(Side*(V.Pier==3?255:165),-670-I*260,70);Part(Cylinder,Q,FVector(.24,.24,1.6),Wood);
            if(V.Pier>=2) { Part(Cylinder,Q+FVector(0,0,145),FVector(.13,.13,1.5),Trim);if(I>0) { Line(Q+FVector(0,0,205),Q+FVector(0,260,205),3,Brass); } }
        } }
        if(V.Pier==3)
        {
            for(int32 Side:{-1,1})
            {
                Part(Cube,FVector(Side*190,End+170,166),FVector(.8,2.6,.18),Trim);
                Part(Cube,FVector(Side*225,End+170,223),FVector(.15,2.6,1),Wood);
                Part(Cylinder,FVector(Side*210,End+430,155),FVector(.55,.55,.8),Brass);
                Part(Sphere,FVector(Side*210,End+430,227),FVector(.8,.8,1.05),FLinearColor(.1f,.32f,.13f));
            }
        }
    }
    if(V.Lighting>0)
    {
        const int32 Count=V.Lighting==1?2:V.Lighting==2?4:8;
        for(int32 I=0;I<Count;++I)
        {
            const FVector Q=I<2?FVector(I?450:-450,-315,470):FVector(-440+(I-2)*176,-520,535);
            Part(Cylinder,Q,FVector(.3,.3,.65),Dark);Part(Sphere,Q+FVector(0,0,10),FVector(.23,.23,.38),FLinearColor(1,.69f,.24f));
            Part(Cylinder,Q+FVector(0,0,43),FVector(.44,.44,.1),Brass);
            if(I<2)
            {
                auto* L=NewObject<UPointLightComponent>(this,*FString::Printf(TEXT("HomeLight%d_%d"),BuildRevision,DetailParts.Num()));AddInstanceComponent(L);L->SetupAttachment(RootComponent);L->SetRelativeLocation(P+Q+FVector(0,-35,10));
                L->SetMobility(EComponentMobility::Movable);L->SetIntensity(850);L->SetAttenuationRadius(750);L->SetLightColor(FLinearColor(1,.61f,.26f));L->SetCastShadows(false);L->RegisterComponent();DetailParts.Add(L);
            }
        }
        if(V.Lighting==3) { Line(FVector(-520,-520,540),FVector(520,-520,540),2.2,Dark); }
    }
    if(V.Display>0)
    {
        const FLinearColor Frame=V.Display==3?FLinearColor(.82f,.57f,.16f):Trim;
        for(int32 Row=0;Row<4;++Row) { Part(Cube,FVector(0,169,170+Row*105),FVector(8.7,.6,.14),Frame); }
        for(int32 Side:{-1,1}) { Part(Cube,FVector(Side*430,169,330),FVector(.16,.6,3.6),Frame); }
        if(V.Display>=2)
        {
            Part(Cube,FVector(0,201,340),FVector(8.4,.05,3.3),Dark);
            for(int32 Side:{-1,1}) { Part(Cube,FVector(Side*218,146,340),FVector(.1,.12,3.3),Frame); }
        }
        if(V.Display==3)
        { for(int32 Side:{-1,1}) { Part(Cylinder,FVector(Side*450,165,155),FVector(.42,.42,.5),Brass);Part(Sphere,FVector(Side*450,165,520),FVector(.4,.4,.4),Brass); } }
    }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HOME_RENDER slot=%d lighting=%d pier=%d display=%d parts=%d"),Slot,V.Lighting,V.Pier,V.Display,DetailParts.Num());
}
