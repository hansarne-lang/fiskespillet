#include "Fishing/FiskFightModel.h"
#include "FiskCareerProfile.h"

void FFiskFightModel::Reset(int32 Seed, float InitialLine,int32 Species,int32 Equipment)
{
    *this = FFiskFightModel();
    Profile=FFiskFightProfile::ForSpecies(Species);
    Gear=Equipment&FFiskCareerProfile::FightGearMask;
    Profile.Pump*=HasGear(Gear,FFiskCareerProfile::ItemRod)?PumpGearBonus:1.0f;
    Line = FMath::IsFinite(InitialLine) ? FMath::Clamp(InitialLine, 2.0f, 1000.0f) : 30.0f;
    Random.Initialize(Seed);
    NextRun = Random.FRandRange(5.0f, 8.0f);
}

void FFiskFightModel::AdjustDrag(int32 Steps)
{
    DragPercent = FMath::Clamp(DragPercent + FMath::Clamp(Steps, -1, 1) * 10, 10, 100);
}

void FFiskFightModel::Advance(double Seconds)
{
    if (!FMath::IsFinite(Seconds) || Seconds <= 0.0) { return; }
    Pending += Seconds;
    // Fixed real-time steps make input/physics independent of render rate and null RHI.
    while (Pending + 1.e-9 >= 0.05 && Outcome == EFiskFightOutcome::Fighting)
    {
        Pending -= 0.05;
        Step();
    }
}

void FFiskFightModel::Step()
{
    constexpr float Dt = 0.05f;
    Age += Dt;
    if (RunLeft <= 0.0 && Age >= NextRun && Fatigue > 0.25f)
    {
        RunLeft = Random.FRandRange(2.0f, 3.0f)*Profile.RunDuration;
        NextRun = Age + Random.FRandRange(7.0f, 11.0f);
    }
    const bool bRun = IsSurging();
    Idle = bRaised || bReeling || bRun ? 0.0f : Idle + Dt;
    Load = FMath::Max(0.0f, 12.0f + DragPercent * 0.55f
        + (bRaised ? 22.0f : 0.0f) + (bReeling ? 18.0f : 0.0f)
        + (bRun ? 40.0f : 0.0f) - FMath::Max(0.0f, Idle - 2.0f) * 4.0f);
    Overload = Load > OverloadLoad(Gear,FFiskCareerProfile::ItemLine) ? Overload + Dt : 0.0f;
    SlackTime = Load < 20.0f ? SlackTime + Dt : 0.0f;
    if (Overload >= 1.0f) { Outcome = EFiskFightOutcome::LineBreak; return; }
    if (SlackTime >= 3.0f) { Outcome = EFiskFightOutcome::Slack; return; }
    if (bRun)
    {
        Line += 5.0f * Profile.RunSpeed * (1.0f - DragPercent * 0.005f) * Dt;
        RunLeft = FMath::Max(0.0, RunLeft - Dt);
        PumpCredit = 0.0f;
    }
    else if (bRaised && !bReeling)
    {
        PumpCredit = FMath::Min(6.0f*Profile.Pump, PumpCredit + 4.0f * Profile.Pump * Dt);
    }
    else if (!bRaised && bReeling)
    {
        const float Gain = FMath::Min(PumpCredit, 4.0f * Profile.Pump * (HasGear(Gear,FFiskCareerProfile::ItemReel) ? ReelGearBonus : 1.0f) * Dt);
        Line = FMath::Max(0.0f, Line - Gain);
        PumpCredit -= Gain;
    }
    Fatigue = FMath::Max(0.0f, Fatigue - Dt * Profile.FatigueDrain * (0.02f + (bRaised ? 0.01f : 0.0f) + (bReeling ? 0.012f : 0.0f)));
    if (Fatigue <= 0.2f && Line <= 2.0f && !bRun) { Outcome = EFiskFightOutcome::ReadyToLand; }
}
