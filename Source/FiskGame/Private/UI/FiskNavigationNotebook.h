#pragma once
#include "CoreMinimal.h"

struct FFiskSavedPlace
{
    int32 Ground = INDEX_NONE;
    FString Name;
};

// Preferences only: every use still goes through the captain's server command.
class FFiskNavigationNotebook
{
public:
    static constexpr int32 Count=6;
    static constexpr int32 NameLimit=24;
    bool Load(const FString& File);
    bool Save(int32 Slot,int32 Ground,const FString& Name,const FString& File);
    bool Clear(int32 Slot,const FString& File);
    const FFiskSavedPlace* Get(int32 Slot) const;
    bool IsBlocked() const { return bBlocked; }
    static bool ValidName(const FString& Name);
private:
    FFiskSavedPlace Slots[Count];
    bool bBlocked=false;
    static bool Read(const FString& File,FFiskNavigationNotebook& Result);
    bool Commit(int32 Slot,const FFiskSavedPlace& Place,const FString& File);
};
