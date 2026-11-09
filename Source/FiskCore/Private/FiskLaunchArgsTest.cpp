#include "FiskLaunchArgs.h"
#include "FiskCareerProfile.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskSpilLanArgumentsTest,"Fiskelaget.Launcher.SpilLanArguments",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskSpilLanArgumentsTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("SpilLan start detected"),FiskLaunchArgs::FromSpilLan(TEXT("-SaveToUserDir --spillan --navn HAJ")));
    TestFalse(TEXT("Plain start is not SpilLan"),FiskLaunchArgs::FromSpilLan(TEXT("-log")));
    TestFalse(TEXT("Empty command line"),FiskLaunchArgs::FromSpilLan(TEXT("")));
    TestFalse(TEXT("Null command line"),FiskLaunchArgs::FromSpilLan(nullptr));
    TestEqual(TEXT("Simple name"),FiskLaunchArgs::PlayerName(TEXT("--spillan --navn HAJ")),FString(TEXT("HAJ")));
    TestEqual(TEXT("Quoted name with space"),FiskLaunchArgs::PlayerName(TEXT("--spillan --navn \"Ola Nordmann\" -log")),FString(TEXT("Ola Nordmann")));
    TestEqual(TEXT("Flag case ignored"),FiskLaunchArgs::PlayerName(TEXT("--NAVN Kari")),FString(TEXT("Kari")));
    TestEqual(TEXT("Invalid characters removed"),FiskLaunchArgs::PlayerName(TEXT("--navn \"Per!! (gutt)\"")),FString(TEXT("Per gutt")));
    TestEqual(TEXT("Cut to 24 characters"),FiskLaunchArgs::PlayerName(TEXT("--navn ABCDEFGHIJKLMNOPQRSTUVWXYZ")),FString(TEXT("ABCDEFGHIJKLMNOPQRSTUVWX")));
    TestEqual(TEXT("No trailing space after cut"),FiskLaunchArgs::PlayerName(TEXT("--navn \"ABCDEFGHIJKLMNOPQRSTUVW XYZ\"")),FString(TEXT("ABCDEFGHIJKLMNOPQRSTUVW")));
    TestTrue(TEXT("Missing name"),FiskLaunchArgs::PlayerName(TEXT("--spillan --navn")).IsEmpty());
    TestTrue(TEXT("Nothing valid"),FiskLaunchArgs::PlayerName(TEXT("--navn \"!!!\"")).IsEmpty());
    TestTrue(TEXT("No arguments"),FiskLaunchArgs::PlayerName(TEXT("")).IsEmpty());
    TestTrue(TEXT("Result is always a valid name"),FFiskCareerProfile::ValidName(FiskLaunchArgs::PlayerName(TEXT("--navn \"  Bølge-Ola_2  \""))));
    return true;
}
#endif
