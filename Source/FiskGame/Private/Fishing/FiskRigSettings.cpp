#include "Fishing/FiskRigSettings.h"
#include "FiskFishingGround.h"
bool FFiskRigRules::IsValid(int32 Bait, int32 Depth, int32 Program, int32 BottomDepth)
{
    return Bait >= 0 && Bait < 3 && Program >= 0 && Program < 3
        && Depth >= 10 && Depth <= FMath::Min(120, BottomDepth) && Depth % 10 == 0;
}
bool FFiskRigRules::TryApply(FFiskRigSettings& Rig, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program, int32 BottomDepth)
{
    if (ExpectedRevision != Rig.Revision || Rig.Revision == MAX_int32 || !IsValid(Bait, Depth, Program, BottomDepth)) { return false; }
    Rig.Bait = Bait; Rig.Depth = Depth; Rig.Program = Program; ++Rig.Revision;
    return true;
}
int32 FFiskRigRules::Quality(const FFiskRigSettings& Rig, const FFiskFishingGround* Ground)
{
    if (!IsValid(Rig.Bait, Rig.Depth, Rig.Program, Ground?Ground->BottomDepth:60)) { return 0; }
    const bool bDepth = Rig.Depth >= (Ground?Ground->FishDepthMin:20) && Rig.Depth <= (Ground?Ground->FishDepthMax:40);
    const bool bMovement = Rig.Bait == Rig.Program;
    return bDepth && bMovement ? 2 : bDepth || bMovement ? 1 : 0;
}
double FFiskRigRules::BiteDelay(const FFiskRigSettings& Rig, FRandomStream& Random, const FFiskFishingGround* Ground)
{
    const int32 Q = Quality(Rig, Ground);
    const float Minimum[] = {300,180,120}, Maximum[] = {480,300,180};
    return Random.FRandRange(Minimum[Q], Maximum[Q]);
}
const TCHAR* FFiskRigRules::BaitName(int32 Bait)
{
    const TCHAR* Names[] = {TEXT("Naturlig agnfisk"),TEXT("Blekksprut"),TEXT("Metalljigg")};
    return Bait >= 0 && Bait < 3 ? Names[Bait] : TEXT("Ukjent");
}
const TCHAR* FFiskRigRules::ProgramName(int32 Program)
{
    const TCHAR* Names[] = {TEXT("Rolig"),TEXT("Pulserende"),TEXT("Aktiv")};
    return Program >= 0 && Program < 3 ? Names[Program] : TEXT("Ukjent");
}
const TCHAR* FFiskRigRules::QualityName(const FFiskRigSettings& Rig, const FFiskFishingGround* Ground)
{
    const TCHAR* Names[] = {TEXT("Dårlig tilpasning"),TEXT("Middels tilpasning"),TEXT("God tilpasning")};
    return Names[Quality(Rig, Ground)];
}
