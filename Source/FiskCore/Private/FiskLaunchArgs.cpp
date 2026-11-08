#include "FiskLaunchArgs.h"
#include "FiskCareerProfile.h"
#include "Misc/Parse.h"

namespace
{
    TArray<FString> Tokens(const TCHAR* CommandLine)
    {
        TArray<FString> Result;
        if(!CommandLine) { return Result; }
        const TCHAR* Stream=CommandLine;FString Token;
        while(FParse::Token(Stream,Token,false)) { Result.Add(Token); }
        return Result;
    }
}

bool FiskLaunchArgs::FromSpilLan(const TCHAR* CommandLine)
{
    return Tokens(CommandLine).ContainsByPredicate([](const FString& T){ return T.Equals(TEXT("--spillan"),ESearchCase::IgnoreCase); });
}

FString FiskLaunchArgs::PlayerName(const TCHAR* CommandLine)
{
    const TArray<FString> All=Tokens(CommandLine);
    const int32 Flag=All.IndexOfByPredicate([](const FString& T){ return T.Equals(TEXT("--navn"),ESearchCase::IgnoreCase); });
    if(Flag==INDEX_NONE || !All.IsValidIndex(Flag+1)) { return FString(); }
    FString Clean;
    for(TCHAR C:All[Flag+1])
    {
        if(FChar::IsAlnum(C) || C=='-' || C=='_') { Clean.AppendChar(C); }
        else if(FChar::IsWhitespace(C) && !Clean.IsEmpty() && !Clean.EndsWith(TEXT(" "))) { Clean.AppendChar(' '); }
    }
    Clean.TrimStartAndEndInline();Clean.LeftInline(24);Clean.TrimEndInline();
    return FFiskCareerProfile::ValidName(Clean)?Clean:FString();
}
