#pragma once
#include "CoreMinimal.h"
#include "UI/FiskJournalModel.h"

enum class EFiskJournalLoad : uint8 { Empty,Loaded,Recovered,Unavailable };
class FFiskJournalStore
{
public:
    static constexpr int64 MaxBytes=4*1024*1024;
    EFiskJournalLoad Load(const FString& File,FFiskJournalModel& Model);
    bool Save(const FString& File,FFiskJournalModel& Model);
private:
    static bool Read(const FString& File,FFiskJournalModel& Model,int32& Generation);
    static bool Write(const FString& File,const FFiskJournalModel& Model,int32 Generation);
    static FString LockName(const FString& File);
};
