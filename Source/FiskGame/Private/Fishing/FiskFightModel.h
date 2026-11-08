#pragma once
#include "CoreMinimal.h"
#include "Fishing/FiskFightProfile.h"

enum class EFiskFightOutcome : uint8 { Fighting, ReadyToLand, LineBreak, Slack };

// Server-only prototype physics. Fatigue and the random stream never replicate.
class FFiskFightModel
{
public:
    // Line load (percent) that breaks the line after one second; Bedre snøre raises it.
    static constexpr float BaseOverloadLoad = 100.0f;
    static constexpr float StrongLineOverloadLoad = 115.0f;
    static constexpr float ReelGearBonus = 1.15f;  // Bedre snelle
    static constexpr float PumpGearBonus = 1.10f;  // Bedre stang
    // Equipment is FFiskCareerProfile::Owned; Item is one of its Item* ids.
    static bool HasGear(int32 Equipment, int32 Item) { return Item >= 0 && Item < 31 && (Equipment & (1 << Item)) != 0; }
    static float OverloadLoad(int32 Equipment, int32 LineItem) { return HasGear(Equipment, LineItem) ? StrongLineOverloadLoad : BaseOverloadLoad; }
    void Reset(int32 Seed, float InitialLine = 30.0f,int32 Species=1,int32 Equipment=0);
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
    FFiskFightProfile Profile;
    int32 Gear=0;
    void Step();
    FRandomStream Random;
    double Pending = 0.0, Age = 0.0, NextRun = 0.0, RunLeft = 0.0;
    float Line = 30.0f, Load = 34.0f, Fatigue = 1.0f, PumpCredit = 0.0f;
    float Idle = 0.0f, Overload = 0.0f, SlackTime = 0.0f;
    int32 DragPercent = 40;
    bool bRaised = false, bReeling = false;
    EFiskFightOutcome Outcome = EFiskFightOutcome::Fighting;
};
