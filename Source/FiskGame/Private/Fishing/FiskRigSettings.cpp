#include "Fishing/FiskRigSettings.h"
bool FFiskRigRules::IsValid(int32 Bait, int32 Depth, int32 Program)
{
    return Bait >= 0 && Bait < 3 && Program >= 0 && Program < 3
        && Depth >= 10 && Depth <= 60 && Depth % 10 == 0;
}
bool FFiskRigRules::TryApply(FFiskRigSettings& Rig, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program)
{
    if (ExpectedRevision != Rig.Revision || Rig.Revision == MAX_int32 || !IsValid(Bait, Depth, Program)) { return false; }
    Rig.Bait = Bait; Rig.Depth = Depth; Rig.Program = Program; ++Rig.Revision;
    return true;
}
int32 FFiskRigRules::Quality(const FFiskRigSettings& Rig)
{
    if (!IsValid(Rig.Bait, Rig.Depth, Rig.Program)) { return 0; }
    // One fixed prototype spot: bottom 60 m, fish signs at 20-40 m.
    const bool bDepth = Rig.Depth >= 20 && Rig.Depth <= 40;
    const bool bMovement = Rig.Bait == Rig.Program;
    return bDepth && bMovement ? 2 : bDepth || bMovement ? 1 : 0;
}
double FFiskRigRules::BiteDelay(const FFiskRigSettings& Rig, FRandomStream& Random)
{
    const int32 Q = Quality(Rig);
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
const TCHAR* FFiskRigRules::QualityName(const FFiskRigSettings& Rig)
{
    const TCHAR* Names[] = {TEXT("Dårlig tilpasning"),TEXT("Middels tilpasning"),TEXT("God tilpasning")};
    return Names[Quality(Rig)];
}
