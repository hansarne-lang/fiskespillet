#pragma once
#include "CoreMinimal.h"

enum class EFiskFightOutcome : uint8 { Fighting, ReadyToLand, LineBreak, Slack };

// Server-only prototype physics. Fatigue and the random stream never replicate.
class FFiskFightModel
{
public:
    void Reset(int32 Seed, float InitialLine = 30.0f);
    void SetRaised(bool Value) { bRaised = Value; }
    void SetReeling(bool Value) { bReeling = Value; }
    void AdjustDrag(int32 Steps);
    void Advance(double Seconds);
    float GetLine() const { return Line; }
    int32 GetDragPercent() const { return DragPercent; }
    int32 GetLoadPercent() const { return FMath::RoundToInt(Load); }
    bool IsSurging() const { return RunLeft > 0.0; }
    bool IsRaised() const { return bRaised; }
    bool IsReeling() const { return bReeling; }
    EFiskFightOutcome GetOutcome() const { return Outcome; }
private:
    void Step();
    FRandomStream Random;
    double Pending = 0.0, Age = 0.0, NextRun = 0.0, RunLeft = 0.0;
    float Line = 30.0f, Load = 34.0f, Fatigue = 1.0f, PumpCredit = 0.0f;
    float Idle = 0.0f, Overload = 0.0f, SlackTime = 0.0f;
    int32 DragPercent = 40;
    bool bRaised = false, bReeling = false;
    EFiskFightOutcome Outcome = EFiskFightOutcome::Fighting;
};
