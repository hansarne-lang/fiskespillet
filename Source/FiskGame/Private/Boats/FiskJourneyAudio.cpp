#include "Boats/FiskJourneyAudio.h"
#include "Boats/FiskStarterBoat.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
UFiskJourneyAudio::UFiskJourneyAudio() { PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=1.f/20; }
void UFiskJourneyAudio::BeginPlay()
{
    Super::BeginPlay();if(FParse::Param(FCommandLine::Get(),TEXT("nosound"))) { SetComponentTickEnabled(false);return; }
    Wave=NewObject<USoundWaveProcedural>(this);Wave->SetSampleRate(24000);Wave->NumChannels=1;Wave->Duration=INDEFINITELY_LOOPING_DURATION;
    Wave->VirtualizationMode=EVirtualizationMode::PlayWhenSilent;
    Audio=NewObject<UAudioComponent>(GetOwner(),TEXT("LocalJourneyAudio"));GetOwner()->AddInstanceComponent(Audio);
    Audio->bAutoActivate=false;Audio->bAllowSpatialization=false;Audio->bIsUISound=true;Audio->SetVolumeMultiplier(.28f);Audio->SetSound(Wave);Audio->RegisterComponent();
}
void UFiskJourneyAudio::EndPlay(const EEndPlayReason::Type Reason) { if(Audio) { Audio->Stop(); }Super::EndPlay(Reason); }
void UFiskJourneyAudio::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);const APlayerController* PC=GetWorld()->GetFirstPlayerController();
    if(!Audio || !Wave || !PC || !PC->IsLocalController()) { return; }
    const AFiskStarterBoat* Boat=CastChecked<AFiskStarterBoat>(GetOwner());const int32 P=Boat->GetTripProgress();
    const bool Moving=P>=0 && P!=100 && P!=200;
    if(!bReported && Moving && Audio->IsPlaying() && Queued>static_cast<uint64>(Wave->GetAvailableAudioByteCount())+4096)
    { bReported=true;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNEY_AUDIO_CONSUMED")); }
    while(Wave->GetAvailableAudioByteCount()<8192)
    {
        TArray<int16> Samples;Samples.SetNumUninitialized(1024);
        for(int32 I=0;I<1024;++I,++Offset)
        {
            const double T=Offset/24000.0;Noise=Noise*1664525u+1013904223u;
            const double Surf=(static_cast<int32>(Noise>>16)-32768)/32768.0*(.018+.012*FMath::Sin(T*.8));
            const double Motor=Moving?(.045*FMath::Sin(T*2*PI*(Boat->GetKind()==1?72:55))+.025*FMath::Sin(T*2*PI*111))*(.8+.2*FMath::Sin(T*9)):0;
            const double BirdPhase=FMath::Fmod(T,11.0),Envelope=BirdPhase<.7?FMath::Sin(BirdPhase/.7*PI):0;
            const double Bird=Envelope*.025*FMath::Sin(2*PI*(1300*T+40*FMath::Sin(T*18)));
            Samples[I]=static_cast<int16>(FMath::Clamp(Surf+Motor+Bird,-1.0,1.0)*32767);
        }
        Wave->QueueAudio(reinterpret_cast<uint8*>(Samples.GetData()),Samples.Num()*sizeof(int16));Queued+=Samples.Num()*sizeof(int16);
    }
    if(!Audio->IsPlaying()) { Audio->Play(); }
}
