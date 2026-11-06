#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FiskBootstrapGameMode.generated.h"

class AFiskStarterBoat;
class AFiskDaySettlement;
class AFiskExpeditionState;

UCLASS()
class AFiskBootstrapGameMode final : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFiskBootstrapGameMode();
    virtual void StartPlay() override;
    virtual void InitGameState() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    AFiskStarterBoat* GetStarterBoat() const { return StarterBoat; }
    AFiskDaySettlement* GetSettlement() const { return Settlement; }
    AFiskExpeditionState* GetExpedition() const { return Expedition; }
    bool StartNextDay(APlayerController* Requester, int32 ExpectedDay);
    void BindCareer(int32 Player,const FGuid& Profile) { DayProfiles.Add(Player,Profile); }
    int64 CareerIncome(const FGuid& Profile) const;
    bool ArchiveConfirmedDay();
    int32 GetArchiveRevision() const { return ArchiveRevision; }
    const TMap<int32,FGuid>& GetDayProfiles() const { return DayProfiles; }

private:
    UPROPERTY() TObjectPtr<AFiskExpeditionState> Expedition;
    TMap<int32,FGuid> DayProfiles;
    FGuid ArchivedDay;
    int32 ArchiveRevision=0;
    double ArchiveRetryAt=0;
    UPROPERTY()
    TObjectPtr<AFiskStarterBoat> StarterBoat;
    UPROPERTY()
    TObjectPtr<AFiskDaySettlement> Settlement;
};
