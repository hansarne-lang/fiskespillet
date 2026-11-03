#include "Bootstrap/FiskBootstrapHUD.h"
#include "Engine/Canvas.h"

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
}
