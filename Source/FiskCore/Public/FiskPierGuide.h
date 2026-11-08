#pragma once
#include "FiskPierRules.h"
#include "Algo/Reverse.h"
namespace FiskPier
{
    inline constexpr int32 DestinationCount=14;
    inline FVector Destination(int32 I) { return I==0?FVector(-650,0,130):I==1?FVector(-4200,-2900,130):I>=2 && I<DestinationCount?HouseDoor(I-2):FVector::ZeroVector; }
    inline bool ClearSegment(FVector A,FVector B)
    {
        if(!Inside(A) || !Inside(B)) { return false; }
        const double Distance=FVector::Dist2D(A,B);if(Distance>20000) { return false; }
        const int32 N=FMath::Max(1,FMath::CeilToInt(Distance/8));
        for(int32 I=1;I<N;++I) { if(!Inside(FMath::Lerp(A,B,static_cast<double>(I)/N))) { return false; } }return true;
    }
    // Bounded, deterministic local presentation path. The server still receives only ordinary axes.
    inline TArray<FVector> GuidePath(FVector Start,FVector Goal)
    {
        if(!Inside(Start) || !Inside(Goal)) { return {}; }
        if(ClearSegment(Start,Goal)) { return {Start,Goal}; }
        constexpr int32 Width=87,Height=135,Count=Width*Height;
        auto Point=[](int32 I){return FVector(-9000+(I%Width)*100,-3200+(I/Width)*100,130);};
        auto Nearest=[&](FVector P){int32 Best=-1;double Distance=1.e30;const int32 X=FMath::Clamp(FMath::RoundToInt((P.X+9000)/100),0,Width-1),Y=FMath::Clamp(FMath::RoundToInt((P.Y+3200)/100),0,Height-1);for(int32 DY=-2;DY<=2;++DY) { for(int32 DX=-2;DX<=2;++DX) { if(X+DX<0 || X+DX>=Width || Y+DY<0 || Y+DY>=Height) { continue; }const int32 I=(Y+DY)*Width+X+DX;const double D=FVector::DistSquared2D(P,Point(I));if(D<Distance && ClearSegment(P,Point(I))) { Best=I;Distance=D; } } }return Best;};
        const int32 S=Nearest(Start),G=Nearest(Goal);if(S<0 || G<0) { return {}; }
        TArray<int32> Previous,Queue;Previous.Init(-1,Count);Previous[S]=S;Queue.Reserve(Count);Queue.Add(S);
        for(int32 Head=0;Head<Queue.Num() && Previous[G]<0;++Head)
        {
            const int32 U=Queue[Head],X=U%Width,Y=U/Width;
            const FIntPoint Steps[]={{1,0},{0,1},{-1,0},{0,-1}};
            for(const auto& D:Steps) { const int32 NX=X+D.X,NY=Y+D.Y;if(NX<0 || NX>=Width || NY<0 || NY>=Height) { continue; }const int32 V=NY*Width+NX;if(Previous[V]<0 && ClearSegment(Point(U),Point(V))) { Previous[V]=U;Queue.Add(V); } }
        }
        if(Previous[G]<0) { return {}; }TArray<FVector> Raw={Goal};int32 V=G;
        for(int32 N=0;N<Count;++N) { Raw.Add(Point(V));if(V==S) { break; }V=Previous[V]; }Raw.Add(Start);Algo::Reverse(Raw);
        TArray<FVector> Path={Start};for(int32 I=0;I<Raw.Num()-1;) { int32 J=Raw.Num()-1;while(J>I+1 && !ClearSegment(Raw[I],Raw[J])) { --J; }Path.Add(Raw[J]);I=J; }return Path;
    }
    inline FRotator NameFacing(FVector TowardCamera,FVector CameraUp)
    { return FRotationMatrix::MakeFromXZ(TowardCamera.GetSafeNormal(.0001,FVector::ForwardVector),CameraUp).Rotator(); }
    inline FVector WalkCamera(int32 Mode) { return Mode==1?FVector(-380,-550,390):Mode==2?FVector(-1200,-1650,1750):FVector(-750,-1050,720); }
}
