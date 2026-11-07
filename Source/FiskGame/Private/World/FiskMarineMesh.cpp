#include "World/FiskMarineMesh.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
namespace
{
constexpr int32 MarineRibs=18;
double MarineWidth(double T) { return 128*FMath::Pow(FMath::Max(.001,1-FMath::Pow(T,3.6)),.65); }
int32 MarineVertex(FFiskMarineMesh& M,FVector V) { const int32 I=M.Vertices.Add(V);M.UV.Add(FVector2D(V.X/700,V.Y/260));return I; }
void MarineTri(FFiskMarineMesh& M,int32 A,int32 B,int32 C) { M.Triangles.Append({A,B,C}); }
void MarineQuad(FFiskMarineMesh& M,int32 A,int32 B,int32 C,int32 D) { MarineTri(M,A,B,C);MarineTri(M,A,C,D); }
void MarineFin(FFiskMarineMesh& M,FVector A,FVector B,FVector C)
{ for(int32 Side=0;Side<2;++Side) { const int32 I=MarineVertex(M,A),J=MarineVertex(M,Side?C:B),K=MarineVertex(M,Side?B:C);MarineTri(M,I,J,K); } }
}
bool FFiskMarineMesh::Valid() const
{
    if(Vertices.IsEmpty() || Normals.Num()!=Vertices.Num() || UV.Num()!=Vertices.Num() || Triangles.IsEmpty() || Triangles.Num()%3) { return false; }
    for(int32 I=0;I<Vertices.Num();++I) { if(Vertices[I].ContainsNaN() || Normals[I].ContainsNaN() || !FMath::IsNearlyEqual(Normals[I].SizeSquared(),1.,.02) || !FMath::IsFinite(UV[I].X) || !FMath::IsFinite(UV[I].Y)) { return false; } }
    for(int32 I:Triangles) { if(!Vertices.IsValidIndex(I)) { return false; } }return true;
}
void FFiskMarineMesh::Finish()
{
    Normals.Init(FVector::ZeroVector,Vertices.Num());for(int32 I=0;I<Triangles.Num();I+=3) { const int32 A=Triangles[I],B=Triangles[I+1],C=Triangles[I+2];const FVector N=FVector::CrossProduct(Vertices[B]-Vertices[A],Vertices[C]-Vertices[A]);Normals[A]+=N;Normals[B]+=N;Normals[C]+=N; }
    for(auto& N:Normals) { N=N.GetSafeNormal(.00001,FVector::UpVector); }
}
FFiskMarineMesh FiskMarine::Hull()
{
    FFiskMarineMesh M;for(int32 I=0;I<=MarineRibs;++I)
    {
        const double T=I/static_cast<double>(MarineRibs),X=-335+690*T,W=MarineWidth(T),Lift=FMath::Pow(T,5)*35;
        for(auto V:{FVector(X,-W,115+Lift),FVector(X,-W*.84,8+Lift),FVector(X,0,-62+Lift),FVector(X,W*.84,8+Lift),FVector(X,W,115+Lift)}) { MarineVertex(M,V); }
        if(I) { for(int32 J=0;J<4;++J) { const int32 A=(I-1)*5+J;MarineQuad(M,A,A+1,I*5+J+1,I*5+J); } }
    }
    for(int32 End=0;End<2;++End) { const int32 Base=End?MarineRibs*5:0,Center=MarineVertex(M,FVector(End?355:-335,0,50+(End?35:0)));for(int32 J=0;J<5;++J) { if(End) { MarineTri(M,Center,Base+J,Base+(J+1)%5); }else { MarineTri(M,Center,Base+(J+1)%5,Base+J); } } }
    M.Finish();return M;
}
FFiskMarineMesh FiskMarine::Deck()
{
    FFiskMarineMesh M;for(int32 I=0;I<=MarineRibs;++I) { const double T=I/static_cast<double>(MarineRibs),W=FMath::Max(1.,MarineWidth(T)-4);MarineVertex(M,FVector(-335+690*T,-W,85));MarineVertex(M,FVector(-335+690*T,W,85));if(I) { MarineQuad(M,(I-1)*2,I*2,I*2+1,(I-1)*2+1); } }M.Finish();return M;
}
FFiskMarineMesh FiskMarine::Gunwale()
{
    FFiskMarineMesh M;for(int32 Side:{-1,1}) { const int32 Base=M.Vertices.Num();for(int32 I=0;I<=MarineRibs;++I) { const double T=I/static_cast<double>(MarineRibs),W=MarineWidth(T),Z=114+FMath::Pow(T,5)*35;MarineVertex(M,FVector(-335+690*T,Side*(W+.5),Z));MarineVertex(M,FVector(-335+690*T,Side*(W+1),Z-17));if(I) { const int32 A=Base+(I-1)*2,B=Base+I*2;if(Side<0) { MarineQuad(M,A,A+1,B+1,B); }else { MarineQuad(M,A,B,B+1,A+1); } } } }M.Finish();return M;
}
FFiskMarineMesh FiskMarine::Fish()
{
    FFiskMarineMesh M;constexpr int32 Rings=14,Sides=12;
    for(int32 I=0;I<=Rings;++I) { const double T=I/static_cast<double>(Rings),Radius=FMath::Max(.5,FMath::Sin(PI*T)*42)*(T<.2?.45:1);for(int32 J=0;J<Sides;++J) { const double A=2*PI*J/Sides;MarineVertex(M,FVector(-210+420*T,FMath::Cos(A)*Radius,FMath::Sin(A)*Radius*.85)); }
        if(I) { for(int32 J=0;J<Sides;++J) { MarineQuad(M,(I-1)*Sides+J,(I-1)*Sides+(J+1)%Sides,I*Sides+(J+1)%Sides,I*Sides+J); } } }
    MarineFin(M,FVector(-185,0,0),FVector(-265,0,105),FVector(-235,0,0));MarineFin(M,FVector(-185,0,0),FVector(-235,0,0),FVector(-265,0,-105));MarineFin(M,FVector(55,0,30),FVector(-40,0,105),FVector(-80,0,30));
    for(int32 Side:{-1,1}) { MarineFin(M,FVector(75,Side*28,-5),FVector(-60,Side*140,-25),FVector(-15,Side*35,-5)); }M.Finish();return M;
}
double FiskMarine::Wave(double X,double Y,double Time,double Strength)
{
    const double Edge=FMath::Clamp(FMath::Max(FMath::Abs(X),FMath::Abs(Y))/12000.,0.,1.);
    return -8-57*FMath::Pow(Edge,8)+(1-FMath::Pow(Edge,4))*Strength*(FMath::Sin(X*.0011+Time*.8)*7+FMath::Sin(Y*.0018+Time*1.3)*4+FMath::Sin((X+Y)*.0007-Time)*3);
}
FFiskMarineMesh FiskMarine::Water(double Time,double Strength)
{
    FFiskMarineMesh M;constexpr int32 Cells=48;for(int32 Y=0;Y<=Cells;++Y) { for(int32 X=0;X<=Cells;++X)
    { const double PX=-12000+24000.*X/Cells,PY=-12000+24000.*Y/Cells,Z=Wave(PX,PY,Time,Strength);MarineVertex(M,FVector(PX,PY,Z));const double DX=(Wave(PX+1,PY,Time,Strength)-Wave(PX-1,PY,Time,Strength))*.5,DY=(Wave(PX,PY+1,Time,Strength)-Wave(PX,PY-1,Time,Strength))*.5;M.Normals.Add(FVector(-DX,-DY,1).GetSafeNormal());if(X && Y) { const int32 A=(Y-1)*(Cells+1)+X-1,B=A+1,C=Y*(Cells+1)+X,D=C-1;MarineQuad(M,A,B,C,D); } } }return M;
}
UProceduralMeshComponent* FiskMarine::Add(AActor* Owner,USceneComponent* Parent,FName Name,const FFiskMarineMesh& M,UMaterialInterface* Material,FLinearColor Color,float Roughness)
{
    check(M.Valid());UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MARINE_MESH name=%s vertices=%d triangles=%d"),*Name.ToString(),M.Vertices.Num(),M.Triangles.Num()/3);auto* C=NewObject<UProceduralMeshComponent>(Owner,Name);Owner->AddInstanceComponent(C);C->SetupAttachment(Parent);C->SetMobility(EComponentMobility::Movable);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->bUseAsyncCooking=true;
    if(Material) { auto* Tint=UMaterialInstanceDynamic::Create(Material,C);TArray<FMaterialParameterInfo> Params;TArray<FGuid> Ids;Material->GetAllVectorParameterInfo(Params,Ids);for(const auto& P:Params) { Tint->SetVectorParameterValue(P.Name,Color); }Params.Reset();Ids.Reset();Material->GetAllScalarParameterInfo(Params,Ids);for(const auto& P:Params) { Tint->SetScalarParameterValue(P.Name,Roughness); }C->SetMaterial(0,Tint); }
    C->RegisterComponent();C->CreateMeshSection_LinearColor(0,M.Vertices,M.Triangles,M.Normals,M.UV,{}, {},false);return C;
}
void FiskMarine::UpdateWater(UProceduralMeshComponent* C,double Time,double Strength)
{ if(C) { const auto M=Water(Time,Strength);C->UpdateMeshSection_LinearColor(0,M.Vertices,M.Normals,M.UV,{},{}); } }
