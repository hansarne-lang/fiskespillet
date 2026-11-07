#pragma once
#include "CoreMinimal.h"
#include "FiskCareerProfile.generated.h"

USTRUCT()
struct FISKCORE_API FFiskCareerReceipt
{
    GENERATED_BODY()
    UPROPERTY() FGuid Id;
    UPROPERTY() int32 Kind=0; // 0 initial grant, 1 purchase, 2 settlement
    UPROPERTY() int32 Item=-1;
    UPROPERTY() int64 Amount=0;
    bool operator==(const FFiskCareerReceipt& Other) const
    { return Id==Other.Id && Kind==Other.Kind && Item==Other.Item && Amount==Other.Amount; }
};

// Full bounded receipt chain, private to the owning connection. No current-day
// catch data enters this contract before the server's settlement barrier.
USTRUCT()
struct FISKCORE_API FFiskCareerProfile
{
    GENERATED_BODY()
    static constexpr int32 MaxReceipts=512;
    static constexpr int32 ItemCount=13;
    static constexpr int32 OwnedMask=(1<<ItemCount)-1;
    static constexpr int64 InitialCash=3000000;
    static constexpr int64 MaxCash=1000000000000LL;
    UPROPERTY() FGuid Id;
    UPROPERTY() FString Name;
    UPROPERTY() int64 Cash=0;
    UPROPERTY() int32 Revision=0;
    UPROPERTY() int32 Owned=0;
    UPROPERTY() TArray<FFiskCareerReceipt> Receipts;
    static FFiskCareerProfile Create(const FString& Name);
    static int64 Price(int32 Item);
    static FString ItemName(int32 Item);
    static bool ValidName(const FString& Name);
    bool IsValid() const;
    bool Extends(const FFiskCareerProfile& Older) const;
    bool Buy(const FGuid& Command,int32 ExpectedRevision,int32 Item);
    bool Credit(const FGuid& Settlement,int64 Amount);
    bool Has(int32 Item) const { return Item>=0 && Item<ItemCount && (Owned&(1<<Item))!=0; }
    FString Money() const;
};
