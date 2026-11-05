#pragma once
#include "CoreMinimal.h"

// Server-only lifecycle gate. Reliable retries retain the identity they were issued for.
class FFiskRodCycle
{
public:
    static constexpr int32 LandingDurationMs = 3000;
    void ResetForDay()
    {
        Id = FGuid::NewGuid(); AvailableAtMs = 0; bOpen = false;
    }
    bool TryStart(const FGuid& ExpectedId, int32 NowMs)
    {
        if (bOpen || ExpectedId != Id || NowMs < AvailableAtMs) { return false; }
        Id = FGuid::NewGuid();
        bOpen = true;
        AvailableAtMs = 0;
        return true;
    }
    bool CanAct(const FGuid& ExpectedId) const { return bOpen && ExpectedId.IsValid() && ExpectedId == Id; }
    bool FinishLanding(const FGuid& ExpectedId, int32 NowMs)
    {
        if (!CanAct(ExpectedId)) { return false; }
        bOpen = false;
        AvailableAtMs = NowMs + LandingDurationMs;
        return true;
    }
    bool Withdraw(const FGuid& ExpectedId)
    {
        if (!CanAct(ExpectedId)) { return false; }
        bOpen = false;
        AvailableAtMs = 0;
        return true;
    }
    const FGuid& GetId() const { return Id; }
    int32 GetAvailableAtMs() const { return AvailableAtMs; }
private:
    FGuid Id;
    int32 AvailableAtMs = 0;
    bool bOpen = false;
};
