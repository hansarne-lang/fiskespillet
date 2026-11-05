#include "FiskCareerProfile.h"

bool FFiskCareerProfile::ValidName(const FString& Value)
{
    if(Value.IsEmpty() || Value.Len()>24 || Value!=Value.TrimStartAndEnd()) { return false; }
    for(TCHAR C:Value) { if(!FChar::IsAlnum(C) && C!=' ' && C!='-' && C!='_') { return false; } }
    return true;
}
int64 FFiskCareerProfile::Price(int32 Item)
{ static constexpr int64 Prices[]={25000,100000,150000,2500000};return Item>=0 && Item<4?Prices[Item]:-1; }
FString FFiskCareerProfile::ItemName(int32 Item)
{
    static const TCHAR* Names[]={TEXT("Bedre snøre"),TEXT("Bedre snelle"),TEXT("Bedre stang"),TEXT("Offshorebåt")};
    return Item>=0 && Item<4?Names[Item]:TEXT("Ukjent vare");
}
FFiskCareerProfile FFiskCareerProfile::Create(const FString& Value)
{
    FFiskCareerProfile P;if(!ValidName(Value)) { return P; }
    P.Id=FGuid::NewGuid();P.Name=Value;P.Cash=InitialCash;P.Revision=1;
    FFiskCareerReceipt R;R.Id=P.Id;R.Amount=InitialCash;P.Receipts.Add(R);return P;
}
bool FFiskCareerProfile::IsValid() const
{
    if(!Id.IsValid() || !ValidName(Name) || Revision!=Receipts.Num() || Revision<1 || Revision>MaxReceipts
        || Cash<0 || Cash>MaxCash || Owned<0 || Owned>15) { return false; }
    int64 Balance=0;int32 Items=0;TSet<FGuid> Seen;
    for(int32 I=0;I<Receipts.Num();++I)
    {
        const FFiskCareerReceipt& R=Receipts[I];
        if(!R.Id.IsValid() || Seen.Contains(R.Id)) { return false; }Seen.Add(R.Id);
        if(I==0)
        { if(R.Id!=Id || R.Kind!=0 || R.Item!=-1 || R.Amount!=InitialCash) { return false; } }
        else if(R.Kind==1)
        {
            if(Price(R.Item)<0 || (Items&(1<<R.Item))!=0 || R.Amount!=-Price(R.Item)) { return false; }
            Items|=1<<R.Item;
        }
        else if(R.Kind!=2 || R.Item!=-1 || R.Amount<0 || R.Amount>MaxCash) { return false; }
        if(R.Amount>0 && Balance>MaxCash-R.Amount) { return false; }
        Balance+=R.Amount;if(Balance<0 || Balance>MaxCash) { return false; }
    }
    return Balance==Cash && Items==Owned;
}
bool FFiskCareerProfile::Extends(const FFiskCareerProfile& Older) const
{
    if(!IsValid() || !Older.IsValid() || Id!=Older.Id || Name!=Older.Name || Revision<Older.Revision) { return false; }
    for(int32 I=0;I<Older.Receipts.Num();++I) { if(!(Receipts[I]==Older.Receipts[I])) { return false; } }
    return true;
}
bool FFiskCareerProfile::Buy(const FGuid& Command,int32 ExpectedRevision,int32 Item)
{
    if(!IsValid() || !Command.IsValid() || Price(Item)<0) { return false; }
    for(const FFiskCareerReceipt& R:Receipts) { if(R.Id==Command) { return R.Kind==1 && R.Item==Item; } }
    if(Revision!=ExpectedRevision || Revision>=MaxReceipts || Has(Item) || Cash<Price(Item)) { return false; }
    FFiskCareerReceipt R;R.Id=Command;R.Kind=1;R.Item=Item;R.Amount=-Price(Item);
    Receipts.Add(R);Cash+=R.Amount;Owned|=1<<Item;++Revision;return true;
}
bool FFiskCareerProfile::Credit(const FGuid& Settlement,int64 Amount)
{
    if(!IsValid() || !Settlement.IsValid() || Amount<0 || Amount>MaxCash) { return false; }
    for(const FFiskCareerReceipt& R:Receipts) { if(R.Id==Settlement) { return R.Kind==2 && R.Amount==Amount; } }
    if(Revision>=MaxReceipts || Cash>MaxCash-Amount) { return false; }
    FFiskCareerReceipt R;R.Id=Settlement;R.Kind=2;R.Amount=Amount;Receipts.Add(R);Cash+=Amount;++Revision;return true;
}
FString FFiskCareerProfile::Money() const
{ return FString::Printf(TEXT("$%lld.%02lld"),static_cast<long long>(Cash/100),static_cast<long long>(Cash%100)); }
