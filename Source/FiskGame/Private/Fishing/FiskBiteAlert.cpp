#include "Fishing/FiskBiteAlert.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Boats/FiskBoardingController.h"
#include "UI/FiskGameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

int16 FiskBiteSound::Sample(uint64 Offset,int32 Slot)
{
    const double Time=static_cast<double>(Offset)/SampleRate;
    if(Time>=Duration) { return 0; }
    const double Frequencies[]={880.0,1108.73,1318.51};
    const double Frequency=Frequencies[FMath::Clamp(Slot,0,2)];
    double Value=0;
    // Two original bell strikes, separate pitches identify the three own rods.
    for(double Strike:{0.0,.24})
    {
        const double T=Time-Strike;if(T<0) { continue; }
        const double Envelope=FMath::Min(1.0,T/.008)*FMath::Exp(-T*8.5);
        Value+=Envelope*(FMath::Sin(2*PI*Frequency*T)+.25*FMath::Sin(2*PI*Frequency*2.76*T));
    }
    return static_cast<int16>(FMath::Clamp(Value*.32,-.8,.8)*32767);
}
UFiskBiteAlert::UFiskBiteAlert()
{
    PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=.05f;
}
void UFiskBiteAlert::BeginPlay()
{
    Super::BeginPlay();const auto* PC=Cast<AFiskBoardingController>(GetOwner());
    if(!PC || !PC->IsLocalController()) { SetComponentTickEnabled(false);return; }
    if(FParse::Param(FCommandLine::Get(),TEXT("nosound"))) { return; }
    Wave=NewObject<USoundWaveProcedural>(this);Wave->SetSampleRate(FiskBiteSound::SampleRate);
    Wave->NumChannels=1;Wave->Duration=INDEFINITELY_LOOPING_DURATION;Wave->VirtualizationMode=EVirtualizationMode::PlayWhenSilent;
    Audio=NewObject<UAudioComponent>(GetOwner(),TEXT("LocalBiteBell"));GetOwner()->AddInstanceComponent(Audio);
    Audio->bAutoActivate=false;Audio->bAllowSpatialization=false;Audio->bIsUISound=true;
    Audio->SetSound(Wave);Audio->RegisterComponent();
}
void UFiskBiteAlert::EndPlay(const EEndPlayReason::Type Reason)
{
    if(Audio) { Audio->Stop(); }Super::EndPlay(Reason);
}
void UFiskBiteAlert::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);const auto* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    for(int32 Slot=0;Slot<PC->GetRodCapacity();++Slot)
    {
        const auto* Rod=PC->GetRodAt(Slot);const auto State=Rod->GetState();
        const bool Bite=State==EFiskRodState::InitialRun || State==EFiskRodState::Fighting || State==EFiskRodState::ReadyToLand;
        if(!Bite || !Rod->GetCastId().IsValid() || Seen[Slot]==Rod->GetCastId()) { continue; }
        Seen[Slot]=Rod->GetCastId();++AlertCount;
        const bool Audible=GI->bSound && Wave && Audio;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BITE_ALERT slot=%d cast=%s audible=%d"),Slot,*Seen[Slot].ToString(EGuidFormats::Digits),Audible);
        if(Audible)
        {
            Pulses.Add(FPulse{0,Slot});
            PendingPlayback.Add(FPlayback{QueuedBytes+2048,Slot,Seen[Slot]});
        }
    }
    if(!Wave || !Audio) { return; }
    Audio->SetVolumeMultiplier(GI->bSound?.75f:0.f);
    if(!GI->bSound) { Pulses.Reset();PendingPlayback.Reset(); }
    const uint64 Consumed=QueuedBytes-static_cast<uint64>(Wave->GetAvailableAudioByteCount());
    for(int32 I=PendingPlayback.Num()-1;I>=0;--I)
    {
        if(Consumed>=PendingPlayback[I].Marker)
        {
            ++ConsumedCount;const auto& P=PendingPlayback[I];
            UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BITE_ALERT_CONSUMED slot=%d cast=%s"),P.Slot,*P.Cast.ToString(EGuidFormats::Digits));
            PendingPlayback.RemoveAt(I);
        }
    }
    while(Wave->GetAvailableAudioByteCount()<4096)
    {
        TArray<int16> Samples;Samples.SetNumUninitialized(1024);
        for(auto& Value:Samples)
        {
            int32 Mixed=0;for(auto& Pulse:Pulses) { Mixed+=FiskBiteSound::Sample(Pulse.Offset++,Pulse.Slot); }
            Value=static_cast<int16>(FMath::Clamp(Mixed,-30000,30000));
        }
        Pulses.RemoveAll([](const FPulse& P){return P.Offset>=FiskBiteSound::SampleRate*FiskBiteSound::Duration;});
        Wave->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()),Samples.Num()*sizeof(int16));
        QueuedBytes+=Samples.Num()*sizeof(int16);
    }
    if(!Audio->IsPlaying()) { Audio->Play(); }
}
