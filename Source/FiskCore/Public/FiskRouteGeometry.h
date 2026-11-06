#pragma once
#include "CoreMinimal.h"
// Shared centimetre geometry for chart, sea validation, islands and navigation.
namespace FiskRouteGeometry
{
    inline constexpr int32 IslandCount=3,NodeCount=2+IslandCount*4;
    inline FVector Island(int32 I) { return I==0?FVector(45000,55000,0):I==1?FVector(110000,45000,0):FVector(42000,-66000,0); }
    inline double Radius(int32 I) { return I==0?9000:I==1?12000:4500; }
    inline bool Sea(FVector P)
    {
        if(P.ContainsNaN() || P.X<6000 || P.X>150000 || FMath::Abs(P.Y)>100000 || FMath::Abs(P.Z)>1 || P.Size2D()<6000) { return false; }
        for(int32 I=0;I<IslandCount;++I) { if(FVector::Dist2D(P,Island(I))<=Radius(I)+2000) { return false; } }return true;
    }
    inline bool Clear(FVector A,FVector B)
    {
        for(int32 I=0;I<IslandCount;++I) { if(FMath::PointDistToSegment(Island(I),A,B)<=Radius(I)+1500) { return false; } }return true;
    }
    inline bool Navigable(FVector P)
    {
        if(P.ContainsNaN() || P.X<0 || P.X>150000 || FMath::Abs(P.Y)>100000 || FMath::Abs(P.Z)>1) { return false; }
        for(int32 I=0;I<IslandCount;++I) { if(FVector::Dist2D(P,Island(I))<=Radius(I)+1500) { return false; } }return true;
    }
    inline TArray<FVector> Between(FVector Origin,FVector Destination)
    {
        if(!Navigable(Origin) || !Navigable(Destination)) { return {}; }TArray<FVector> Nodes={Origin,Destination};
        for(int32 I=0;I<IslandCount;++I) { for(int32 X=-1;X<=1;X+=2) { for(int32 Y=-1;Y<=1;Y+=2) { Nodes.Add(Island(I)+FVector(X,Y,0)*(Radius(I)+3500)); } } }
        double Dist[NodeCount];int32 Previous[NodeCount];bool Used[NodeCount]={};
        for(int32 I=0;I<NodeCount;++I) { Dist[I]=1.e30;Previous[I]=-1; }Dist[0]=0;
        for(int32 Step=0;Step<NodeCount;++Step)
        {
            int32 U=-1;for(int32 I=0;I<NodeCount;++I) { if(!Used[I] && (U<0 || Dist[I]<Dist[U])) { U=I; } }
            if(U<0 || U==1 || Dist[U]>1.e29) { break; }Used[U]=true;
            for(int32 V=0;V<NodeCount;++V) { if(!Used[V] && Clear(Nodes[U],Nodes[V]))
                { const double D=Dist[U]+FVector::Dist(Nodes[U],Nodes[V]);if(D<Dist[V]) { Dist[V]=D;Previous[V]=U; } } }
        }
        if(Previous[1]<0) { return {}; }TArray<FVector> Result;int32 V=1;
        while(V>=0) { Result.Insert(Nodes[V],0);V=Previous[V]; }return Result;
    }
    inline TArray<FVector> Path(FVector Destination)
    { return Sea(Destination)?Between(FVector::ZeroVector,Destination):TArray<FVector>(); }
    inline double Length(const TArray<FVector>& P)
    { double N=0;for(int32 I=1;I<P.Num();++I) { N+=FVector::Dist(P[I-1],P[I]); }return N; }
    inline FVector Position(const TArray<FVector>& P,double Fraction)
    {
        if(P.IsEmpty()) { return FVector::ZeroVector; }
        if(Fraction<=0) { return P[0]; }if(Fraction>=1) { return P.Last(); }
        double Left=Length(P)*FMath::Clamp(Fraction,0.0,1.0);
        for(int32 I=1;I<P.Num();++I) { const double D=FVector::Dist(P[I-1],P[I]);if(Left<=D) { return FMath::Lerp(P[I-1],P[I],D>0?Left/D:0); }Left-=D; }
        return P.IsEmpty()?FVector::ZeroVector:P.Last();
    }
}
