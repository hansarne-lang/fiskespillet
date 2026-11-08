#pragma once
#include "CoreMinimal.h"
struct FFiskGreetingClock
{
    int32 LastSerial=0;TMap<FString,double> Until;
    void Observe(int32 Serial,const FString& Name,double Now) { if(Serial<=LastSerial || Name.IsEmpty() || !FMath::IsFinite(Now)) { return; }LastSerial=Serial;Until.FindOrAdd(Name)=Now+4.; }
    bool IsWaving(const FString& Name,double Now) const { const double* T=Until.Find(Name);return T && FMath::IsFinite(Now) && Now<*T; }
};
