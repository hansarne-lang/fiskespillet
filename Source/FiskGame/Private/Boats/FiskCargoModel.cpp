#include "Boats/FiskCargoModel.h"
FFiskCargoLimits FFiskCargoModel::Limits(int32 Kind)
{ return Kind==0?FFiskCargoLimits{100000,10000,200000}:Kind==1?FFiskCargoLimits{300000,30000,500000}:FFiskCargoLimits{}; }
int32 FFiskCargoModel::ReturnDuration(int32 Ms,bool Towing)
{ if(Ms<=0) { return 0; }return static_cast<int32>(FMath::Min<int64>(MAX_int32,Towing?FMath::DivideAndRoundUp<int64>(static_cast<int64>(Ms)*100,65):Ms)); }
EFiskCargoResult FFiskCargoModel::Add(int32 Kind,const FGuid& Id,int32 Grams)
{
    const auto L=Limits(Kind);if(!L.Hold || !Id.IsValid() || Grams<=0 || Seen.Contains(Id) || Seen.Num()>=10000 || bMustReturn) { return EFiskCargoResult::Rejected; }
    Seen.Add(Id);bLastDiscarded=false;
    if(static_cast<int64>(HoldGrams)+Grams<=L.Hold+L.Overload)
    {
        HoldGrams+=Grams;bMustReturn=HoldGrams>=L.Hold;Mode=HoldGrams>L.Hold?EFiskCargoMode::Overload:bMustReturn?EFiskCargoMode::Full:EFiskCargoMode::Stowed;
        return Mode==EFiskCargoMode::Overload?EFiskCargoResult::ReturnOverload:bMustReturn?EFiskCargoResult::ReturnFull:EFiskCargoResult::Stowed;
    }
    if(Grams>L.Hold/2 && Grams<=L.Tow && HoldGrams<=L.Hold)
    { TowedGrams=Grams;bMustReturn=true;Mode=EFiskCargoMode::Towing;return EFiskCargoResult::Tow; }
    bLastDiscarded=true;bMustReturn=HoldGrams>0;return EFiskCargoResult::Uncarryable;
}
FFiskCargoView FFiskCargoModel::View(bool Revealed) const
{ FFiskCargoView V;V.Mode=static_cast<uint8>(Mode);V.bDiscarded=bLastDiscarded;V.bReturning=bMustReturn;V.RevealedGrams=Revealed?HoldGrams+TowedGrams:-1;return V; }
