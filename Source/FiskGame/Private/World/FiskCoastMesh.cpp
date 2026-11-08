#include "World/FiskCoastMesh.h"
#include "FiskRouteGeometry.h"
namespace
{
double Shore(double A,int32 I) { return .88+.045*FMath::Sin(A*3+I)+.025*FMath::Cos(A*7-I); }
double Height(double T,double A,int32 I)
{
    const double H=I==2?520:2400;
    return -35+100*(1-T)+H*FMath::Pow(1-T,1.5)*(1+.18*FMath::Sin(A*3+I)*T);
}
FFiskMarineMesh Strip(int32 Island,double Inner,double Outer,bool Foam)
{
    FFiskMarineMesh M;if(Island<0 || Island>=FiskRouteGeometry::IslandCount) { return M; }
    const double R=FiskRouteGeometry::Radius(Island);const int32 Rings=Foam?1:FiskCoast::Rings;
    for(int32 Row=0;Row<=Rings;++Row) { const double T=FMath::Lerp(Inner,Outer,static_cast<double>(Row)/Rings);for(int32 J=0;J<=FiskCoast::Sectors;++J)
    {
        const double A=2*PI*J/FiskCoast::Sectors,Radius=R*Shore(A,Island)*T;
        M.Vertices.Add(FVector(Radius*FMath::Cos(A),Radius*FMath::Sin(A),Foam?-1+2*FMath::Sin(A*9):Height(T,A,Island)));M.UV.Add(FVector2D(J/static_cast<double>(FiskCoast::Sectors),T));
        if(Row && J) { const int32 B=Row*(FiskCoast::Sectors+1)+J,A0=B-FiskCoast::Sectors-2;M.Triangles.Append({A0,B,A0+1,A0,B-1,B}); }
    } }M.Finish();
    // UE renders clockwise front faces. Keep the upward shading normals computed above,
    // then reverse the index order so the terrain is visible from above the sea.
    for(int32 T=0;T<M.Triangles.Num();T+=3) { Swap(M.Triangles[T+1],M.Triangles[T+2]); }
    return M;
}
}
FFiskMarineMesh FiskCoast::Terrain(int32 I,int32 Band)
{ return Band<0 || Band>2?FFiskMarineMesh():Strip(I,Band==0?.78:Band==1?.38:.001,Band==0?1.:Band==1?.78:.38,false); }
FFiskMarineMesh FiskCoast::Foam(int32 I) { return Strip(I,.982,1.012,true); }
FVector FiskCoast::Decoration(int32 I,int32 Index,double Fraction)
{
    if(I<0 || I>=FiskRouteGeometry::IslandCount || !FMath::IsFinite(Fraction) || Fraction<0 || Fraction>1) { return FVector::ZeroVector; }
    const double A=Index*2.399963+I*.7,T=.25+.6*Fraction,R=FiskRouteGeometry::Radius(I)*Shore(A,I)*T;
    return FiskRouteGeometry::Island(I)+FVector(R*FMath::Cos(A),R*FMath::Sin(A),Height(T,A,I));
}
