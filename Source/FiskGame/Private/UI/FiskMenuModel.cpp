#include "UI/FiskMenuModel.h"

int32 FFiskMenuModel::Rows() const
{
    switch (Page)
    {
    case EFiskMenuPage::Home: case EFiskMenuPage::Session: return 4;
    case EFiskMenuPage::Join: case EFiskMenuPage::Settings: return 3;
    case EFiskMenuPage::Connecting: return 1;
    default: return 0;
    }
}
void FFiskMenuModel::Move(int32 Direction)
{
    const int32 Count = Rows();
    if (Count > 0) { Selected = (Selected + Direction + Count) % Count; }
}
void FFiskMenuModel::Edit(TCHAR Character)
{
    if (Page == EFiskMenuPage::Join && Selected == 0 && Address.Len() < 21
        && ((Character >= TEXT('0') && Character <= TEXT('9')) || Character == TEXT('.') || Character == TEXT(':')))
    { Address.AppendChar(Character); Message.Reset(); }
}
void FFiskMenuModel::Backspace()
{
    if (Page == EFiskMenuPage::Join && Selected == 0 && !Address.IsEmpty()) { Address.LeftChopInline(1); Message.Reset(); }
}
bool FFiskMenuModel::ParseAddress(const FString& Text, FString& Endpoint)
{
    Endpoint.Reset();
    if (Text.IsEmpty() || Text.Len() > 21) { return false; }
    for (TCHAR C : Text) { if (!(C >= TEXT('0') && C <= TEXT('9')) && C != TEXT('.') && C != TEXT(':')) { return false; } }
    TArray<FString> Sides; Text.ParseIntoArray(Sides, TEXT(":"), false);
    if (Sides.Num() < 1 || Sides.Num() > 2) { return false; }
    int32 Port = 7777;
    if (Sides.Num() == 2)
    {
        if (Sides[1].IsEmpty() || Sides[1].Len() > 5) { return false; }
        for (TCHAR C : Sides[1]) { if (C < TEXT('0') || C > TEXT('9')) { return false; } }
        Port = FCString::Atoi(*Sides[1]);
        if (Port < 1 || Port > 65535) { return false; }
    }
    TArray<FString> Octets; Sides[0].ParseIntoArray(Octets, TEXT("."), false);
    if (Octets.Num() != 4) { return false; }
    TArray<int32> Numbers;
    for (const FString& Part : Octets)
    {
        if (Part.IsEmpty() || Part.Len() > 3) { return false; }
        for (TCHAR C : Part) { if (C < TEXT('0') || C > TEXT('9')) { return false; } }
        const int32 Number = FCString::Atoi(*Part);
        if (Number > 255) { return false; }
        Numbers.Add(Number);
    }
    if (Numbers[0] == 0 || Numbers[0] >= 224) { return false; }
    Endpoint = FString::Printf(TEXT("%d.%d.%d.%d:%d"), Numbers[0], Numbers[1], Numbers[2], Numbers[3], Port);
    return true;
}
EFiskMenuAction FFiskMenuModel::Activate()
{
    if (Page == EFiskMenuPage::Home)
    {
        if (Selected == 0) { return EFiskMenuAction::Host; }
        if (Selected == 1) { Show(EFiskMenuPage::Join); }
        else if (Selected == 2) { SettingsParent = Page; Show(EFiskMenuPage::Settings); }
        else if (Selected == 3) { return EFiskMenuAction::Quit; }
    }
    else if (Page == EFiskMenuPage::Session)
    {
        if (Selected == 0) { Show(EFiskMenuPage::Closed); return EFiskMenuAction::Resume; }
        if (Selected == 1) { SettingsParent = Page; Show(EFiskMenuPage::Settings); }
        else if (Selected == 2) { return EFiskMenuAction::Leave; }
        else if (Selected == 3) { return EFiskMenuAction::Quit; }
    }
    else if (Page == EFiskMenuPage::Join)
    {
        if (Selected == 2) { Show(EFiskMenuPage::Home); }
        else
        {
            FString Endpoint;
            if (!ParseAddress(Address, Endpoint)) { Message = TEXT("Skriv en gyldig IPv4-adresse. Eksempel: 192.168.0.10"); }
            else { return EFiskMenuAction::Join; }
        }
    }
    else if (Page == EFiskMenuPage::Settings)
    {
        if (Selected == 0) { bSound = !bSound; return EFiskMenuAction::Sound; }
        if (Selected == 1) { FrameRate = FrameRate == 30 ? 60 : 30; return EFiskMenuAction::FrameRate; }
        Show(SettingsParent);
    }
    else if (Page == EFiskMenuPage::Connecting) { return EFiskMenuAction::Cancel; }
    return EFiskMenuAction::None;
}
EFiskMenuAction FFiskMenuModel::Back()
{
    if (Page == EFiskMenuPage::Connecting) { return EFiskMenuAction::Cancel; }
    if (Page == EFiskMenuPage::Settings) { Show(SettingsParent); }
    else if (Page == EFiskMenuPage::Join) { Show(EFiskMenuPage::Home); }
    else if (Page == EFiskMenuPage::Session) { Show(EFiskMenuPage::Closed); return EFiskMenuAction::Resume; }
    return EFiskMenuAction::None;
}
