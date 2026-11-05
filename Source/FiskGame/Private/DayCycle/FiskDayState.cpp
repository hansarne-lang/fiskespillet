#include "DayCycle/FiskDayState.h"
#include "HAL/PlatformTime.h"
#include "Net/UnrealNetwork.h"
#include "DayCycle/FiskSessionRules.h"

AFiskDayState::AFiskDayState()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
}

void AFiskDayState::StartDay()
{
    if (!HasAuthority() || HasStarted()) { return; }
    StartedMonotonic = FPlatformTime::Seconds();
    Clock.Number = 1;
    Clock.ElapsedMs = 0;
    OnRep_Clock();
    ForceNetUpdate();
}

bool AFiskDayState::AdvanceDay()
{
    if (!HasAuthority() || !HasStarted() || GetElapsedMilliseconds() < DayEnd*1000 || Clock.Number >= FiskSession::MaxDay) { return false; }
    StartedMonotonic = FPlatformTime::Seconds();
    ++Clock.Number; Clock.ElapsedMs = 0; LastMinute = -1;
    OnRep_Clock(); ForceNetUpdate(); return true;
}

int32 AFiskDayState::GetElapsedMilliseconds() const
{
    if (!HasStarted()) { return 0; }
    if (!HasAuthority()) { return Clock.ElapsedMs; }
    const double Delta = FMath::Clamp(FPlatformTime::Seconds() - StartedMonotonic, 0.0, static_cast<double>(DayEnd));
    return FMath::FloorToInt(Delta * 1000.0);
}

void AFiskDayState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!HasAuthority() || !HasStarted()) { return; }
    Clock.ElapsedMs = GetElapsedMilliseconds();
    const int32 Minute = Clock.ElapsedMs / 60000;
    if (Minute != LastMinute)
    {
        LastMinute = Minute;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_CLOCK ms=%d"), Clock.ElapsedMs);
    }
}

int32 AFiskDayState::CalculateElapsedSeconds(double ServerTime, double StartTime)
{
    return FMath::FloorToInt(FMath::Clamp(ServerTime - StartTime, 0.0, static_cast<double>(DayEnd)));
}

EFiskDayPhase AFiskDayState::GetPhase(int32 ElapsedSeconds)
{
    if (ElapsedSeconds >= DayEnd) { return EFiskDayPhase::Finished; }
    if (ElapsedSeconds >= SettlementStart) { return EFiskDayPhase::Settlement; }
    if (ElapsedSeconds >= PreparationEnd) { return EFiskDayPhase::FishingAndReturn; }
    return EFiskDayPhase::Preparation;
}

void AFiskDayState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFiskDayState, Clock);
}

void AFiskDayState::OnRep_Clock()
{
    if (HasStarted() && ReportedDay != Clock.Number)
    {
        ReportedDay = Clock.Number;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_DAY_STARTED ms=%d"), Clock.ElapsedMs);
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_SESSION_DAY number=%d ms=%d"), Clock.Number, Clock.ElapsedMs);
    }
}
