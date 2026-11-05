#include "Fishing/FiskCombatFeedback.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Boats/FiskBoardingController.h"
#include "Components/AudioComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Sound/SoundWaveProcedural.h"

void FiskReelSound::Fill(TArray<int16>& Samples, int32 Count, bool bActive, bool bRun, int32 Load, uint64& Offset)
{
    Samples.SetNumUninitialized(Count);
    for (int32 I = 0; I < Count; ++I, ++Offset)
    {
        const double T = static_cast<double>(Offset) / SampleRate;
        const double Rattle = FMath::Pow(FMath::Max(0.0, FMath::Sin(T * 2.0 * PI * (bRun ? 65.0 : 24.0))), 8.0);
        const double Tone = FMath::Sin(T * 2.0 * PI * (bRun ? 920.0 : 360.0));
        const double Level = bActive ? (0.10 + FMath::Clamp(Load, 0, 140) * 0.0006) : 0.0;
        Samples[I] = static_cast<int16>(Tone * Rattle * Level * 32767.0);
    }
}

UFiskCombatFeedback::UFiskCombatFeedback()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f / 30.0f;
}
void UFiskCombatFeedback::BeginPlay()
{
    Super::BeginPlay();
    const AFiskBoardingController* PC = Cast<AFiskBoardingController>(GetOwner());
    if (!PC || !PC->IsLocalController() || FParse::Param(FCommandLine::Get(), TEXT("nosound")))
    {
        SetComponentTickEnabled(false);
        return;
    }
    Wave = NewObject<USoundWaveProcedural>(this);
    Wave->SetSampleRate(FiskReelSound::SampleRate);
    Wave->NumChannels = 1;
    Wave->Duration = INDEFINITELY_LOOPING_DURATION;
    Wave->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
    Audio = NewObject<UAudioComponent>(GetOwner(), TEXT("LocalReelAudio"));
    GetOwner()->AddInstanceComponent(Audio);
    Audio->bAutoActivate = false;
    Audio->bAllowSpatialization = false;
    Audio->bIsUISound = true;
    Audio->SetVolumeMultiplier(0.35f);
    Audio->SetSound(Wave);
    Audio->RegisterComponent();
}
void UFiskCombatFeedback::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Audio) { Audio->Stop(); }
    Super::EndPlay(Reason);
}
void UFiskCombatFeedback::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!Wave || !Audio) { return; }
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const UFiskPrototypeRod* Rod=PC->GetRod();
    // A passive rod's first run must be audible even while the other rod is selected.
    for(int32 Slot=0;Slot<PC->GetRodCapacity();++Slot)
    { if(PC->GetRodAt(Slot)->GetState()==EFiskRodState::InitialRun) { Rod=PC->GetRodAt(Slot);break; } }
    const bool bRun = Rod->GetState() == EFiskRodState::InitialRun || Rod->IsSurging();
    const bool bActive = bRun || (Rod->GetState() == EFiskRodState::Fighting && Rod->IsReeling());
    if (bActive && !bReportedPlayback && Audio->IsPlaying()
        && QueuedBytes > static_cast<uint64>(Wave->GetAvailableAudioByteCount()) + 4096)
    {
        bReportedPlayback = true;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REEL_AUDIO_CONSUMED"));
    }
    while (Wave->GetAvailableAudioByteCount() < 8192)
    {
        TArray<int16> Samples;
        FiskReelSound::Fill(Samples, 1024, bActive, bRun, Rod->GetLoadPercent(), SampleOffset);
        Wave->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));
        QueuedBytes += Samples.Num() * sizeof(int16);
    }
    if (!Audio->IsPlaying()) { Audio->Play(); }
}
