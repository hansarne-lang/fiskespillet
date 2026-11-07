#include "World/FiskSpeciesArt.h"
namespace
{
const FFiskSpeciesArt Art[]={
 {390,42,76,70,65,0,8,{.75f,.22f,.15f},{1,.53f,.32f}},
 {470,40,60,85,50,0,8,{.35f,.48f,.49f},{.91f,.67f,.17f}},
 {470,58,72,105,92,0,8,{.05f,.24f,.42f},{1,.83f,.08f}},
 {380,68,94,66,61,0,9,{.31f,.28f,.19f},{.55f,.44f,.26f}},
 {460,39,105,100,118,0,8,{.06f,.61f,.39f},{.14f,.47f,.91f}},
 {560,30,43,90,42,0,7,{.09f,.32f,.53f},{.63f,.81f,.87f}},
 {470,40,64,125,107,155,8,{.03f,.19f,.39f},{.18f,.47f,.74f}},
 {590,25,37,70,40,28,7,{.39f,.48f,.49f},{.14f,.21f,.24f}},
 {470,42,82,92,66,0,11,{.64f,.72f,.72f},{.22f,.33f,.38f}},
 {300,23,134,86,124,0,8,{.55f,.61f,.57f},{.13f,.21f,.25f}},
 {460,35,65,78,57,0,9,{.54f,.59f,.42f},{.13f,.18f,.12f}},
 {440,67,88,110,79,0,16,{.09f,.25f,.38f},{.68f,.73f,.57f}},
 {470,29,51,115,178,145,7,{.05f,.26f,.47f},{.12f,.42f,.76f}},
 {510,44,72,125,100,190,9,{.19f,.28f,.35f},{.55f,.61f,.64f}},
 {460,51,73,70,54,0,11,{.055f,.09f,.11f},{.23f,.29f,.29f}}
};
int32 V(FFiskMarineMesh& M,FVector P) { const int32 I=M.Vertices.Add(P);M.UV.Add(FVector2D(P.X/600,P.Z/300));return I; }
void Tri(FFiskMarineMesh& M,int32 A,int32 B,int32 C) { M.Triangles.Append({A,B,C}); }
void Fin(FFiskMarineMesh& M,FVector A,FVector B,FVector C)
{ for(int32 S=0;S<2;++S) { const int32 I=V(M,A),J=V(M,S?C:B),K=V(M,S?B:C);Tri(M,I,J,K); } }
}
const FFiskSpeciesArt* FiskSpeciesArt::Find(int32 Species) { return Species>=0 && Species<Count?&Art[Species]:nullptr; }
FFiskMarineMesh FiskSpeciesArt::Body(int32 Species)
{
    FFiskMarineMesh M;const auto* P=Find(Species);if(!P) { return M; }constexpr int32 Rings=22,Sides=20;
    for(int32 I=0;I<=Rings;++I)
    {
        const double T=I/static_cast<double>(Rings),Radius=FMath::Max(.015,FMath::Pow(FMath::Sin(PI*T),.7));
        const double TailNarrow=T<.2?.28+T*3.6:1,Forehead=Species==4 && T>.65?1.2:1;
        for(int32 J=0;J<Sides;++J) { const double A=2*PI*J/Sides;V(M,FVector((T-.5)*P->Length,FMath::Cos(A)*P->Width*Radius*TailNarrow,FMath::Sin(A)*P->Height*Radius*TailNarrow*Forehead)); }
        if(I) { for(int32 J=0;J<Sides;++J) { const int32 A=(I-1)*Sides+J,B=(I-1)*Sides+(J+1)%Sides,C=I*Sides+(J+1)%Sides,D=I*Sides+J;Tri(M,A,B,C);Tri(M,A,C,D); } }
    }
    for(int32 End=0;End<2;++End) { const int32 C=V(M,FVector((End?.5:-.5)*P->Length,0,0)),Base=End?Rings*Sides:0;for(int32 J=0;J<Sides;++J) { if(End) { Tri(M,C,Base+J,Base+(J+1)%Sides); }else { Tri(M,C,Base+(J+1)%Sides,Base+J); } } }
    if(P->Bill>0) { const int32 Base=M.Vertices.Num();for(int32 J=0;J<12;++J) { const double A=J*2*PI/12;V(M,FVector(P->Length*.45,FMath::Cos(A)*7,FMath::Sin(A)*(Species==13?3:7))); }const int32 Tip=V(M,FVector(P->Length*.5+P->Bill,0,0));for(int32 J=0;J<12;++J) { Tri(M,Base+J,Tip,Base+(J+1)%12); } }
    M.Finish();return M;
}
FFiskMarineMesh FiskSpeciesArt::Fins(int32 Species)
{
    FFiskMarineMesh M;const auto* P=Find(Species);if(!P) { return M; }
    const double X=-P->Length*.48;
    Fin(M,FVector(X,0,0),FVector(X-80,0,P->Tail),FVector(X-42,0,0));Fin(M,FVector(X,0,0),FVector(X-42,0,0),FVector(X-80,0,-P->Tail));
    const int32 Segments=Species==12?10:Species==4?8:1;
    for(int32 I=0;I<Segments;++I)
    {
        const double T=I/static_cast<double>(Segments),U=(I+1.)/Segments;
        const FVector A(P->Length*(.25-.6*T),0,P->Height*.65),B(P->Length*(.25-.6*U),0,P->Height*.65);
        if(Segments==1) { Fin(M,A,FVector(-P->Length*.12,0,P->Dorsal),FVector(-P->Length*.27,0,P->Height*.65)); }
        else { const FVector C(A.X,0,P->Height*.65+P->Dorsal*FMath::Sin((T*.75+.12)*PI)),D(B.X,0,P->Height*.65+P->Dorsal*FMath::Sin((U*.75+.12)*PI));Fin(M,A,C,D);Fin(M,A,D,B); }
    }
    for(int32 Side:{-1,1}) { Fin(M,FVector(P->Length*.2,Side*P->Width*.8,0),FVector(-P->Length*.17,Side*(P->Width+70),-P->Height*.6),FVector(0,Side*P->Width,-P->Height*.3)); }
    Fin(M,FVector(-P->Length*.1,0,-P->Height*.7),FVector(-P->Length*.28,0,-P->Dorsal*.72),FVector(-P->Length*.35,0,-P->Height*.5));
    if(Species==2 || Species==11) { for(int32 I=0;I<6;++I) { const double PX=-P->Length*(.22+.04*I);for(int32 S:{-1,1}) { Fin(M,FVector(PX,0,S*P->Height*.6),FVector(PX-12,0,S*P->Height*.84),FVector(PX-20,0,S*P->Height*.4)); } } }
    M.Finish();return M;
}
FVector FiskSpeciesArt::Eye(int32 Species,int32 Side)
{ const auto* P=Find(Species);return P?FVector(P->Length*.32,(Side<0?-1:1)*P->Width*.71,P->Height*.24):FVector::ZeroVector; }
