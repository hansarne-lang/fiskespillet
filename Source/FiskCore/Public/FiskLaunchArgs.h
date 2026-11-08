#pragma once
#include "CoreMinimal.h"

/** Arguments from the SpilLan launcher, which starts the game with: --spillan --navn <player name>. */
namespace FiskLaunchArgs
{
    /** True when the command line contains --spillan. */
    FISKCORE_API bool FromSpilLan(const TCHAR* CommandLine);
    /**
     * The name after --navn, cleaned so it is a valid FISKELAGET name (FFiskCareerProfile::ValidName):
     * characters other than letters, digits, space, - and _ are removed, and the name is cut to 24 characters.
     * Empty when --navn is missing or nothing valid remains.
     */
    FISKCORE_API FString PlayerName(const TCHAR* CommandLine);
}
