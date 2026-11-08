#include "World/FiskOceanAtlas.h"
const FFiskAtlasMark* FiskAtlas::Find(int32 Index)
{
    static const FFiskAtlasMark Marks[]={
        {TEXT("Korallrevet"),FVector(20000,0,0),TEXT("Grunt rev. Begynn ved 20–40 m.")},
        {TEXT("Undervannsskrenten"),FVector(60000,24000,0),TEXT("Bratt kant. Prøv 60–90 m.")},
        {TEXT("Åpent hav"),FVector(100000,-75000,0),TEXT("Dyp bunn, fisketegn ofte 20–60 m.")},
        {TEXT("Lagunen"),FVector(18000,-19000,0),TEXT("Kort tur nær marinaen, med revhabitat.")},
        {TEXT("Pelikanodden"),FVector(30000,18000,0),TEXT("Rev utenfor odden. Bruk ekkoloddet.")},
        {TEXT("Blårenna"),FVector(72000,10000,0),TEXT("Skrenthabitat med dypere fiske.")},
        {TEXT("Fregattpasset"),FVector(70000,85000,0),TEXT("Ruten går rundt øya. Beregn returen.")},
        {TEXT("Vestbanken"),FVector(94000,-42000,0),TEXT("Åpent hav. Lang tur gir mindre fisketid.")},
        {TEXT("Marlinryggen"),FVector(140000,-65000,0),TEXT("Ytterste punkt. Ingen garantifangst.")},
        {TEXT("Mangrovebanken"),FVector(35000,-60000,0),TEXT("Nytt habitat. Fire mangrovearter, fiske ved 20–50 m.")},
        {TEXT("Rotbukta"),FVector(52000,-82000,0),TEXT("Mangroveøyas utside. Ruten holder avstand til røttene.")},
        {TEXT("Tarponrenna"),FVector(20000,-85000,0),TEXT("Mangrovehabitat. Lange utras krever rolig snørearbeid.")},
        {TEXT("Dyphavsbanken"),FVector(140000,85000,0),TEXT("400 m bunn. Prøv 80–120 m med oppgradert utstyr.")},
        {TEXT("Tunfiskkanten"),FVector(140000,10000,0),TEXT("Dyphavshabitat øst for øya. Planlegg hjemreisen.")},
        {TEXT("Sverdfiskryggen"),FVector(145000,65000,0),TEXT("Lang tur. Dype oppsett gir mye line ved første utras.")},
        {TEXT("Norddjupet"),FVector(130000,95000,0),TEXT("Dyphavshabitat. Artsvalg er fortsatt uforutsigbart.")}
    };return Index>=0 && Index<Count?&Marks[Index]:nullptr;
}
int32 FiskAtlas::Nearest(FVector Point,double Radius)
{
    int32 Best=-1;double Distance=Radius;
    for(int32 I=0;I<Count;++I) { const double D=FVector::Dist2D(Point,Find(I)->Point);if(D<=Distance) { Best=I;Distance=D; } }return Best;
}
