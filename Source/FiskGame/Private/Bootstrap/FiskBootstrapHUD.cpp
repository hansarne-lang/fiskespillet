#include "Bootstrap/FiskBootstrapHUD.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Boats/FiskStarterBoat.h"
#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskPrototypeRod.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"

void AFiskBootstrapHUD::DrawHUD()
{
    Super::DrawHUD();
    if (Canvas == nullptr)
    {
        return;
    }

    const float Left = 30.0f;
    const float Top = 24.0f;
    DrawRect(FLinearColor(0.01f, 0.025f, 0.04f, 0.78f), 14.0f, 12.0f, 520.0f, 390.0f);
    DrawText(TEXT("FISKELAGET"), FLinearColor::White, Left, Top, nullptr, 2.0f);
    DrawText(TEXT("LAN-prototype - fiskedag i 3D"), FLinearColor::White, Left, Top + 65.0f, nullptr, 1.0f);

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
        const int32 Progress = Boat->GetTripProgress();
        const bool bAboard = Boat->IsAboard(PlayerId);
        const TCHAR* Location = bAboard
            ? TEXT("Du er om bord")
                        : Progress == AFiskStarterBoat::AtDock
                ? TEXT("Ved brygga: Trykk E for å gå om bord")
                : TEXT("Du er ved brygga");
        const int32 CaptainId = Boat->GetCaptainId();
        const TCHAR* Captain = CaptainId == INDEX_NONE ? TEXT("Ingen")
            : CaptainId == PlayerId ? TEXT("Deg") : TEXT("Medspiller");
        DrawText(FString::Printf(TEXT("Startbåt  %d/%d   Kaptein: %s"), Boat->GetCrewCount(),
            AFiskStarterBoat::Capacity, Captain), FLinearColor::White, Left, Top + 135.0f);
        DrawText(Location, FLinearColor::White, Left, Top + 160.0f);

        FString Route;
        if (Progress == AFiskStarterBoat::AtDock)
        {
            Route = CaptainId != INDEX_NONE && CaptainId == PlayerId
                ? TEXT("Ved brygga. Fra 02:00: Trykk R for å dra til fiskeplassen")
                : TEXT("Ved brygga. Kapteinen starter turen");
        }
        else if (Progress == AFiskStarterBoat::AtMarina)
        {
            Route = TEXT("Tilbake ved brygga");
        }
        else if (Progress > AFiskStarterBoat::AtFishingSpot)
        {
            Route = FString::Printf(TEXT("På vei hjem: %d%%"), Progress - AFiskStarterBoat::AtFishingSpot);
        }
        else if (Progress == AFiskStarterBoat::AtFishingSpot)
        {
            Route = CaptainId != INDEX_NONE && CaptainId == PlayerId
                ? TEXT("Framme ved fiskeplassen. Trykk R for å dra hjem")
                : TEXT("Framme ved fiskeplassen");
        }
        else
        {
            Route = FString::Printf(TEXT("Underveis til fiskeplassen: %d%%"), Progress);
        }
        DrawText(Route, FLinearColor::White, Left, Top + 185.0f);
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


    for (TActorIterator<AFiskDaySettlement> Settlement(GetWorld()); Settlement; ++Settlement)
    {
        const FFiskSettlementView& View = Settlement->GetView();
        if (!View.bRevealed)
        {
            DrawText(TEXT("Dagsresultat ved 17:00. Saldo (økt): 0"), FLinearColor::White, Left, Top + 275.0f);
        }
        else
        {
            float RowY = Top + 275.0f;
            for (const FFiskDayResult& Row : View.Results)
            {
                DrawText(FString::Printf(TEXT("Spiller %d: levert %d, tapt %d, saldo (økt) %d"),
                    Row.PlayerId, Row.DeliveredFish, Row.LostFish, Row.SessionBalance), FLinearColor::White, Left, RowY);
                RowY += 24.0f;
            }
        }
    }

    const AFiskStarterBoat* Boat = StarterBoat.Get();
    const AFiskBoardingController* PC = Cast<AFiskBoardingController>(GetOwningPlayerController());
    if (!Boat || !PC || !PC->PlayerState || !Boat->IsAboard(PC->PlayerState->GetPlayerId())
        || Boat->GetTripProgress() != AFiskStarterBoat::AtFishingSpot
        || AFiskDayState::GetPhase(Elapsed) != EFiskDayPhase::FishingAndReturn)
    {
        return;
    }

    FString RodText = TEXT("Stang 1: Trykk F for å sette ut");
    switch (PC->GetRod()->GetState())
    {
    case EFiskRodState::Ready:
        break;
    case EFiskRodState::Fishing:
        RodText = TEXT("Stang 1: Fisker");
        break;
    case EFiskRodState::InitialRun:
        RodText = TEXT("Stang 1: Napp! Fisken tar line");
        break;
    case EFiskRodState::Fighting:
        RodText = FString::Printf(TEXT("Stang 1: Kamp - %d m line ute"), PC->GetRod()->GetLineOut());
        DrawText(TEXT("Venstreklikk: løft og senk. Høyreklikk: sveiv inn"),
            FLinearColor::White, Left, Top + 240.0f);
        break;
    case EFiskRodState::ReadyToLand:
        RodText = TEXT("Stang 1: Klar til landing. Trykk mellomrom");
        break;
    case EFiskRodState::Landed:
        RodText = TEXT("Stang 1: Fisk landet");
        break;
    }
    DrawText(RodText, FLinearColor::White, Left, Top + 215.0f);
}
