#pragma once
#include "CoreMinimal.h"

// Mouse-steered rod (issue #132). Turns rod movement into the two fight inputs the server already
// understands, so no button has to be held: moving up lifts (pump), moving down reels in, and a
// still rod keeps tension when it is held high. The fight rules and limits are unchanged.
class FISKCORE_API FFiskRodStick
{
public:
    static constexpr float MoveThreshold = 0.15f;  // Angle per second before a movement counts
    static constexpr float HoldHigh = 0.5f;         // A still rod at or above this keeps tension
    static constexpr float Smoothing = 0.12f;       // Seconds; hides hand tremor
    static constexpr float KeyStep = 0.12f;         // Arrow keys move the rod in steps
    // Mouse distance for the full rod travel. Sensitivity 1 = low, 2 = medium, 3 = high.
    static float TravelForSensitivity(int32 Sensitivity) { return Sensitivity <= 1 ? 900.0f : Sensitivity >= 3 ? 350.0f : 550.0f; }
    static bool ValidSensitivity(int32 Sensitivity) { return Sensitivity >= 1 && Sensitivity <= 3; }

    void Reset(float NewAngle = 0.3f);
    // Positive moves the rod up. Unit: fraction of the full travel.
    void Move(float DeltaAngle);
    void Advance(float Seconds, bool bSurging);
    float GetAngle() const { return Angle; }
    float GetVelocity() const { return Velocity; }
    int32 GetDirection() const { return Direction; }   // 1 up, -1 down, 0 still
    bool WantsRaised() const { return bRaised; }
    bool WantsReeling() const { return bReeling; }
private:
    float Angle = 0.3f, Pending = 0.0f, Velocity = 0.0f;
    int32 Direction = 0;
    bool bRaised = false, bReeling = false;
};
