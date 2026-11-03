#include "Boats/FiskStarterBoat.h"
#include "Net/UnrealNetwork.h"

AFiskStarterBoat::AFiskStarterBoat()
{
    bReplicates = true;
    bAlwaysRelevant = true;
}

void AFiskStarterBoat::SetOwnerPlayer(int32 PlayerId)
{
    if (HasAuthority() && OwnerId == INDEX_NONE)
    {
        OwnerId = PlayerId;
    }
}

void AFiskStarterBoat::Board(int32 PlayerId)
{
    if (!HasAuthority() || PlayerId == INDEX_NONE || IsAboard(PlayerId) || Crew.Num() >= Capacity)
    {
        return;
    }

    Crew.Add(PlayerId);
    OnRep_Crew();
    ForceNetUpdate();
}

void AFiskStarterBoat::Leave(int32 PlayerId)
{
    if (HasAuthority() && Crew.Remove(PlayerId) > 0)
    {
        OnRep_Crew();
        ForceNetUpdate();
    }
}

int32 AFiskStarterBoat::GetCaptainId() const
{
    if (IsAboard(OwnerId))
    {
        return OwnerId;
    }
    return Crew.IsEmpty() ? INDEX_NONE : Crew[0];
}

void AFiskStarterBoat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFiskStarterBoat, OwnerId);
    DOREPLIFETIME(AFiskStarterBoat, Crew);
}

void AFiskStarterBoat::OnRep_Crew()
{
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_STARTER_BOAT_CREW %d"), Crew.Num());
}
