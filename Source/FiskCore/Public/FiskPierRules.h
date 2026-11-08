#pragma once
#include "CoreMinimal.h"
#include "FiskPierLayout.h"
namespace FiskPier
{
    struct FArea { double X0,Y0,X1,Y1; };
    inline constexpr FArea Areas[]={ {-1800,-3200,-900,10300},{-9000,3600,-1200,4100},{-9000,5900,-1200,6400},{-9000,8200,-1200,8700},{-5500,-3200,-1200,-2800},{-5400,-3000,-3000,-2050},{-1200,-350,-450,350} };
    inline FVector Spawn() { return FVector(-1300,0,130); }
    inline bool Inside(FVector P) { if(P.ContainsNaN() || !FMath::IsFinite(P.X) || !FMath::IsFinite(P.Y) || FMath::Abs(P.Z-130)>1 || !FurnitureClear(P)) { return false; }for(const auto& A:Areas) { if(P.X>=A.X0+BodyRadius && P.X<=A.X1-BodyRadius && P.Y>=A.Y0+BodyRadius && P.Y<=A.Y1-BodyRadius) { return true; } }return false; }
    inline bool ValidAxis(FVector2D A) { return FMath::IsFinite(A.X) && FMath::IsFinite(A.Y) && A.SizeSquared()<=2.001; }
    inline bool CanStart(bool Ready,bool Own,bool Dock,bool Aboard) { return Ready && Own && Dock && !Aboard; }
    inline bool AcceptInput(bool Enabled,bool Own,int32 Generation,int32 Expected,int32 Serial,int32 Last,FVector2D A) { return Enabled && Own && Generation==Expected && Serial>0 && Serial>Last && ValidAxis(A); }
    inline FVector Step(FVector P,FVector2D Axis,double Delta,bool Sprint)
    {
        if(!Inside(P) || !ValidAxis(Axis) || !FMath::IsFinite(Delta) || Delta<=0) { return P; }
        const FVector2D A=Axis.GetClampedToMaxSize(1.);const double D=FMath::Min(Delta,.1)*(Sprint?350:220);
        // Short swept substeps prevent thin furniture from being crossed by a large frame.
        const int32 N=FMath::Max(1,FMath::CeilToInt(D/10));const FVector V(A.X*D/N,A.Y*D/N,0);
        for(int32 I=0;I<N;++I) { FVector Q=P+V;if(Inside(Q)) { P=Q;continue; }Q=P+FVector(V.X,0,0);if(Inside(Q)) { P=Q; }Q=P+FVector(0,V.Y,0);if(Inside(Q)) { P=Q; } }return P;
    }
    inline FVector HouseDoor(int32 I) { return FVector(-3000-1700*(FMath::Clamp(I,0,11)%4),3850+2300*(FMath::Clamp(I,0,11)/4),130); }
    inline bool NearBar(FVector P) { return FVector::Dist2D(P,FVector(-4200,-2700,130))<650; }
    inline bool NearBoat(FVector P) { return FVector::Dist2D(P,FVector(-550,0,130))<550; }
}
