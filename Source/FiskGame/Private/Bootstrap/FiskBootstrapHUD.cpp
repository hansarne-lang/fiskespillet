#include "Bootstrap/FiskBootstrapHUD.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Boats/FiskStarterBoat.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "DayCycle/FiskDayState.h"

void AFiskBootstrapHUD::DrawHUD()
{
    Super::DrawHUD();
    if (Canvas == nullptr)
    {
        return;
    }

    const float Left = Canvas->ClipX * 0.08f;
    const float Top = Canvas->ClipY * 0.15f;
    DrawText(TEXT("FISKELAGET"), FLinearColor::White, Left, Top, nullptr, 2.0f);
    DrawText(TEXT("Teknisk grunnstamme"), FLinearColor::White, Left, Top + 65.0f, nullptr, 1.0f);

    if (!StarterBoat.IsValid())
    {
        TActorIterator<AFiskStarterBoat> Boats(GetWorld());
        if (Boats)
        {
            StarterBoat = *Boats;
        }
    }

    if (const AFiskStarterBoat* Boat = StarterBoat.Get())
    {
        const APlayerController* PC = GetOwningPlayerController();
        const int32 PlayerId = PC && PC->PlayerState ? PC->PlayerState->GetPlayerId() : INDEX_NONE;
        const TCHAR* Location = Boat->IsAboard(PlayerId)
            ? TEXT("Du er om bord")
            : TEXT("Ved brygga: Trykk E for å gå om bord");
        const int32 CaptainId = Boat->GetCaptainId();
        const TCHAR* Captain = CaptainId == INDEX_NONE ? TEXT("Ingen")
            : CaptainId == PlayerId ? TEXT("Deg") : TEXT("Medspiller");
        DrawText(FString::Printf(TEXT("Startbåt  %d/%d   Kaptein: %s"), Boat->GetCrewCount(),
            AFiskStarterBoat::Capacity, Captain), FLinearColor::White, Left, Top + 135.0f);
        DrawText(Location, FLinearColor::White, Left, Top + 160.0f);
    }

    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (Day == nullptr)
    {
        return;
    }

    if (!Day->HasStarted())
    {
        const TCHAR* Message = GetNetMode() == NM_Standalone
            ? TEXT("Start en LAN-vert for å spille.")
            : TEXT("Venter på to spillere ...");
        DrawText(Message, FLinearColor::White, Left, Top + 100.0f);
        return;
    }

    const int32 Elapsed = Day->GetElapsedSeconds();
    const TCHAR* Phase = TEXT("Forberedelser");
    switch (AFiskDayState::GetPhase(Elapsed))
    {
    case EFiskDayPhase::Preparation:
        break;
    case EFiskDayPhase::FishingAndReturn:
        Phase = TEXT("Fiske og retur");
        break;
    case EFiskDayPhase::Settlement:
        Phase = TEXT("Dagsoppgjør");
        break;
    case EFiskDayPhase::Finished:
        Phase = TEXT("Fiskedagen er ferdig");
        break;
    }

    DrawText(FString::Printf(TEXT("%02d:%02d  %s"), Elapsed / 60, Elapsed % 60, Phase),
        FLinearColor::White, Left, Top + 100.0f);
}
