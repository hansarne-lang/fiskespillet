#include "UI/FiskMenuModel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskAddressTest,"Fiskelaget.Menu.AddressValidation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskAddressTest::RunTest(const FString& Parameters)
{
    FString Endpoint;
    TestTrue(TEXT("LAN IPv4"),FFiskMenuModel::ParseAddress(TEXT("192.168.0.10"),Endpoint));
    TestEqual(TEXT("Default port"),Endpoint,FString(TEXT("192.168.0.10:7777")));
    TestTrue(TEXT("Local test and port"),FFiskMenuModel::ParseAddress(TEXT("127.0.0.1:17784"),Endpoint));
    TestTrue(TEXT("Normalize octets"),FFiskMenuModel::ParseAddress(TEXT("010.001.002.003:1"),Endpoint));
    TestEqual(TEXT("Canonical address"),Endpoint,FString(TEXT("10.1.2.3:1")));
    for (const TCHAR* Bad : {TEXT(""),TEXT("999.1.1.1"),TEXT("1.2.3"),TEXT("1..2.3"),TEXT("1.2.3.4:"),TEXT("1.2.3.4:0"),TEXT("1.2.3.4:65536"),TEXT("1.2.3.4:7.7"),TEXT("1.2.3.4::77"),TEXT("127.0.0.1?listen"),TEXT("/Engine/Maps/Entry"),TEXT("0.0.0.0"),TEXT("224.0.0.1"),TEXT("localhost")})
    {
        TestFalse(FString::Printf(TEXT("Reject %s"),Bad),FFiskMenuModel::ParseAddress(Bad,Endpoint));
        TestTrue(TEXT("Failed parse clears endpoint"),Endpoint.IsEmpty());
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMenuNavigationTest,"Fiskelaget.Menu.Navigation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMenuNavigationTest::RunTest(const FString& Parameters)
{
    FFiskMenuModel M; M.Show(EFiskMenuPage::Home);
    M.Move(-1); TestEqual(TEXT("Wrap up"),M.Selected,3); M.Move(1); TestEqual(TEXT("Wrap down"),M.Selected,0);
    TestTrue(TEXT("Play hosts"),M.Activate()==EFiskMenuAction::Host);
    M.Move(1); M.Activate(); TestTrue(TEXT("Join page"),M.Page==EFiskMenuPage::Join);
    M.Address=TEXT("999.0.0.1"); TestTrue(TEXT("Invalid join remains local"),M.Activate()==EFiskMenuAction::None && M.Page==EFiskMenuPage::Join && !M.Message.IsEmpty());
    M.Address=TEXT("127.0.0.1"); TestTrue(TEXT("Valid join"),M.Activate()==EFiskMenuAction::Join);
    M.Back(); M.Selected=2; M.Activate(); TestTrue(TEXT("Settings"),M.Page==EFiskMenuPage::Settings);
    M.Activate(); TestFalse(TEXT("Mute"),M.bSound); M.Move(1); M.Activate(); TestEqual(TEXT("30 FPS"),M.FrameRate,30);
    TestEqual(TEXT("Five settings rows"),M.Rows(),5);
    M.Move(1); TestTrue(TEXT("Rod control toggles off"),M.Activate()==EFiskMenuAction::RodControl && !M.bMouseRod);
    M.Activate(); TestTrue(TEXT("Rod control toggles back on"),M.bMouseRod);
    M.Move(1); M.Activate(); TestEqual(TEXT("Sensitivity medium to high"),M.RodSensitivity,3);
    M.Activate(); TestEqual(TEXT("Sensitivity wraps to low"),M.RodSensitivity,1);
    M.Move(1); TestTrue(TEXT("Last row returns"),M.Activate()==EFiskMenuAction::None && M.Page==EFiskMenuPage::Home);
    M.Selected=2; M.Activate(); TestTrue(TEXT("Reopen settings"),M.Page==EFiskMenuPage::Settings);
    M.Back(); TestTrue(TEXT("Back to home"),M.Page==EFiskMenuPage::Home);
    M.Show(EFiskMenuPage::Session); M.Selected=1; M.Activate(); M.Back(); TestTrue(TEXT("Back to session"),M.Page==EFiskMenuPage::Session);
    TestTrue(TEXT("Resume without pausing world"),M.Back()==EFiskMenuAction::Resume && M.Page==EFiskMenuPage::Closed);
    M.Show(EFiskMenuPage::Connecting); TestTrue(TEXT("Cancel connecting"),M.Back()==EFiskMenuAction::Cancel);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMenuEditingTest,"Fiskelaget.Menu.AddressEditing",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMenuEditingTest::RunTest(const FString& Parameters)
{
    FFiskMenuModel M; M.Show(EFiskMenuPage::Join); M.Address.Reset();
    for (TCHAR C : FString(TEXT("127.0.0.1:7777"))) { M.Edit(C); }
    TestEqual(TEXT("Address entered"),M.Address,FString(TEXT("127.0.0.1:7777")));
    M.Edit(TEXT('?')); TestEqual(TEXT("Options cannot be typed"),M.Address,FString(TEXT("127.0.0.1:7777")));
    M.Selected=1; M.Backspace(); TestEqual(TEXT("Only address field edits"),M.Address,FString(TEXT("127.0.0.1:7777")));
    M.Selected=0; M.Backspace(); TestEqual(TEXT("Backspace"),M.Address,FString(TEXT("127.0.0.1:777")));
    for (int32 I=0;I<100;++I) { M.Edit(TEXT('1')); } TestEqual(TEXT("Bounded input"),M.Address.Len(),21);
    return true;
}
#endif
