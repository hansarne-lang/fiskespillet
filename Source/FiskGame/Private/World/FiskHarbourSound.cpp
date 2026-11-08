#include "World/FiskHarbourSound.h"
#include "Boats/FiskBoardingController.h"
#include "UI/FiskMarinaPanel.h"
#include "World/FiskPierWalk.h"
#include "FiskPierRules.h"
#include "UI/FiskGameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
float FiskHarbourAudio::Sample(uint64 Offset,bool Music)
{
    const double T=Offset/24000.;uint32 Noise=static_cast<uint32>(Offset)*747796405u+2891336453u;Noise=((Noise>>((Noise>>28)+4))^Noise)*277803737u;Noise=(Noise>>22)^Noise;
    const double Surf=(static_cast<int32>(Noise>>16)-32768)/32768.*(.027+.008*FMath::Sin(T*.8));
    const double BirdPhase=FMath::Fmod(T,13.),Bird=BirdPhase<.7?FMath::Sin(BirdPhase/.7*PI)*.018*FMath::Sin(2*PI*(1400*T+55*FMath::Sin(T*15))):0;
    if(!Music) { return Surf+Bird; }
    // Original sixteen-note pentatonic phrase, synthesized here. No recording or song source.
    constexpr int32 Notes[]={60,64,67,69,67,64,62,60,64,67,72,69,67,62,64,60};
    const double Beat=T/.45,Phase=FMath::Frac(Beat),Hz=440*FMath::Pow(2.,(Notes[static_cast<int32>(Beat)%16]-69)/12.);
    const double Pan=.085*FMath::Exp(-Phase*6)*(FMath::Sin(2*PI*Hz*T)+.28*FMath::Sin(2*PI*Hz*2.76*T)+.14*FMath::Sin(2*PI*Hz*4.1*T));
    const double Bass=.035*FMath::Exp(-FMath::Frac(Beat*.5)*9)*FMath::Sin(2*PI*65.406*T);
    return FMath::Clamp(Surf+Bird+Pan+Bass,-.4,.4);
}
UFiskHarbourSound::UFiskHarbourSound() { PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=.05f; }
void UFiskHarbourSound::BeginPlay()
{
    Super::BeginPlay();if(FParse::Param(FCommandLine::Get(),TEXT("nosound"))) { SetComponentTickEnabled(false);return; }
    Wave=NewObject<USoundWaveProcedural>(this);Wave->SetSampleRate(24000);Wave->NumChannels=1;Wave->Duration=INDEFINITELY_LOOPING_DURATION;Wave->VirtualizationMode=EVirtualizationMode::PlayWhenSilent;
    Audio=NewObject<UAudioComponent>(GetOwner(),TEXT("HarbourOriginalAudio"));GetOwner()->AddInstanceComponent(Audio);Audio->bAutoActivate=false;Audio->bAllowSpatialization=false;Audio->bIsUISound=true;Audio->SetVolumeMultiplier(.4f);Audio->SetSound(Wave);Audio->RegisterComponent();
}
void UFiskHarbourSound::EndPlay(const EEndPlayReason::Type Reason) { if(Audio) { Audio->Stop(); }Super::EndPlay(Reason); }
void UFiskHarbourSound::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);const auto* PC=Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController());const auto* GI=Cast<UFiskGameInstance>(GetWorld()->GetGameInstance());
    if(!Wave || !Audio || !PC || !PC->IsLocalController()) { return; }const auto* Panel=PC->GetMarinaPanel();
    const bool Walking=PC->GetPierWalk()->IsEnabled(),Bar=Panel->GetPage()==2 || (Walking && FiskPier::NearBar(PC->GetPierWalk()->GetPosition()));
    if(!GI || !GI->bSound || (!Walking && Panel->GetPage()!=2)) { if(Audio->IsPlaying()) { Audio->Stop();Wave->ResetAudio();Queued=0; }return; }
    if(bMusic!=(Bar && Panel->IsHarbourMusic())) { bMusic=Bar && Panel->IsHarbourMusic();Wave->ResetAudio();Queued=0;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HARBOUR_MUSIC enabled=%d"),bMusic); }
    if(!bReported && Audio->IsPlaying() && Queued>static_cast<uint64>(Wave->GetAvailableAudioByteCount())+4096) { bReported=true;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HARBOUR_AUDIO_CONSUMED")); }
    while(Wave->GetAvailableAudioByteCount()<24576)
    { TArray<int16> Samples;Samples.SetNumUninitialized(1024);for(int32 I=0;I<1024;++I,++Offset) { Samples[I]=static_cast<int16>(FiskHarbourAudio::Sample(Offset,bMusic)*32767); }Wave->QueueAudio(reinterpret_cast<uint8*>(Samples.GetData()),Samples.Num()*sizeof(int16));Queued+=Samples.Num()*sizeof(int16); }
    if(!Audio->IsPlaying()) { Audio->Play(); }
}
