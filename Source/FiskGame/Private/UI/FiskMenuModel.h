#pragma once
#include "CoreMinimal.h"

enum class EFiskMenuPage : uint8 { Closed, Home, Join, Settings, Connecting, Session };
enum class EFiskMenuAction : uint8 { None, Host, Join, Quit, Sound, FrameRate, Resume, Leave, Cancel, RodControl };

class FFiskMenuModel
{
public:
    EFiskMenuPage Page = EFiskMenuPage::Closed;
    int32 Selected = 0;
    FString Address = TEXT("127.0.0.1"), Message;
    bool bSound = true;
    int32 FrameRate = 60;
    bool bMouseRod = true;
    int32 RodSensitivity = 2;
    void Show(EFiskMenuPage NewPage) { Page = NewPage; Selected = 0; Message.Reset(); }
    int32 Rows() const;
    void Move(int32 Direction);
    void Edit(TCHAR Character);
    void Backspace();
    EFiskMenuAction Activate();
    EFiskMenuAction Back();
    static bool ParseAddress(const FString& Text, FString& Endpoint);
private:
    EFiskMenuPage SettingsParent = EFiskMenuPage::Home;
};
