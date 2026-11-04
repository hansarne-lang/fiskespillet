#include "Boats/FiskStarterBoat.h"
bool FFiskRouteView::Authorizes(int32 Requester,int32 ExpectedRevision,const TArray<int32>& Crew) const
{
    return Revision<MAX_int32 && ExpectedRevision==Revision && Requester>=0
        && Requester==CaptainId && Crew.Contains(Requester);
}
bool FFiskRouteView::Transfer(int32 Requester,int32 ExpectedRevision,int32 Target,const TArray<int32>& Crew)
{
    if(!Authorizes(Requester,ExpectedRevision,Crew) || Target<0 || Target==Requester || !Crew.Contains(Target)) { return false; }
    CaptainId=Target;++Revision;return true;
}
bool FFiskRouteView::RefreshCaptain(const TArray<int32>& Crew,int32 OwnerId,bool bPreferOwner)
{
    int32 Next=CaptainId;
    if(bPreferOwner && OwnerId>=0 && Crew.Contains(OwnerId)) { Next=OwnerId; }
    else if(!Crew.Contains(CaptainId)) { Next=Crew.IsEmpty()?INDEX_NONE:Crew[0]; }
    if(Next==CaptainId) { return false; }
    // Initial assignment has no older valid captain commands. Later role changes invalidate them.
    if((CaptainId!=INDEX_NONE || Revision>0) && Revision<MAX_int32) { ++Revision; }
    CaptainId=Next;return true;
}
