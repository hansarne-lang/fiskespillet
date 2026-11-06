#pragma once
#include "FiskCareerProfile.h"
enum class EFiskCareerLoad : uint8 { Empty,Loaded,Recovered,Blocked };
class FFiskCareerStore
{
public:
    static EFiskCareerLoad Load(const FString& File,FFiskCareerProfile& Profile);
    static bool Save(const FString& File,const FFiskCareerProfile& Profile);
private:
    static bool Read(const FString& File,FFiskCareerProfile& Profile);
    static bool Write(const FString& File,const FFiskCareerProfile& Profile);
};
