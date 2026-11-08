#pragma once
#include "CoreMinimal.h"
#include "FiskCargoModel.generated.h"

enum class EFiskCargoMode : uint8 { Empty, Stowed, Full, Overload, Towing };
enum class EFiskCargoResult : uint8 { Rejected, Stowed, ReturnFull, ReturnOverload, Tow, Uncarryable };
struct FFiskCargoLimits { int32 Hold=0,Overload=0,Tow=0; };
USTRUCT()
struct FFiskCargoView
{
    GENERATED_BODY()
    UPROPERTY() uint8 Mode=0;
    UPROPERTY() bool bDiscarded=false;
    UPROPERTY() bool bReturning=false;
    // Exact weight is never replicated before the common 17:00 reveal.
    UPROPERTY() int32 RevealedGrams=-1;
};
class FFiskCargoModel
{
public:
    static FFiskCargoLimits Limits(int32 Kind);
    static int32 ReturnDuration(int32 NominalMs,bool Towing);
    EFiskCargoResult Add(int32 Kind,const FGuid& CatchId,int32 Grams);
    FFiskCargoView View(bool Revealed=false) const;
    int32 HoldGrams=0,TowedGrams=0;
    bool bMustReturn=false,bLastDiscarded=false;
    EFiskCargoMode Mode=EFiskCargoMode::Empty;
    int32 Count() const { return Seen.Num(); }
private:
    TSet<FGuid> Seen;
};
