#include "FiskRodStick.h"

void FFiskRodStick::Reset(float NewAngle)
{
    Angle = FMath::Clamp(NewAngle, 0.0f, 1.0f); Pending = 0.0f; Velocity = 0.0f; Direction = 0;
    bRaised = Angle >= HoldHigh; bReeling = false;
}
void FFiskRodStick::Move(float DeltaAngle)
{
    // Only movement the rod can actually make counts, so pushing past the ends does nothing.
    const float Before = Angle;
    Angle = FMath::Clamp(Angle + DeltaAngle, 0.0f, 1.0f);
    Pending += Angle - Before;
}
void FFiskRodStick::Advance(float Seconds, bool bSurging)
{
    if (Seconds <= 0.0f) { return; }
    const float Instant = Pending / Seconds; Pending = 0.0f;
    Velocity += (Instant - Velocity) * (1.0f - FMath::Exp(-Seconds / Smoothing));
    if (Velocity > MoveThreshold) { Direction = 1; }
    else if (Velocity < -MoveThreshold) { Direction = -1; }
    else if (FMath::Abs(Velocity) < MoveThreshold * 0.5f) { Direction = 0; }
    // During a run the reel slips against the drag instead of winding in.
    bRaised = Direction > 0 || (Direction == 0 && Angle >= HoldHigh);
    bReeling = Direction < 0 && !bSurging;
}
