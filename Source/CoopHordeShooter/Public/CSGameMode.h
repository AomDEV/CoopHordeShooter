#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CSGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController);

enum class EWaveState: uint8;

UCLASS()
class COOPHORDESHOOTER_API ACSGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ACSGameMode();
protected:
    
    UFUNCTION(BlueprintImplementableEvent, Category="GameMode")
    void SpawnNewBot();

    UPROPERTY(EditDefaultsOnly, Category="GameMode", meta=(ClampMin=0))
    float TimeBetweenWaves;

    void StartWave();
    void EndWave();
    void CheckWaveState();
    void PrepareForNextWave();
    void SpawnBotTimerElapsed();
    
    void CheckAnyPlayerIsAlive();
    void GameOver();
    void SetWaveState(EWaveState NewState);

    void RestartDeadPlayers();

    int BotsToSpawn;
    int WaveCount; 
    FTimerHandle TimerHandle_BotSpawner;
    FTimerHandle TimerHandle_NextWaveStart;
public:
    virtual void StartPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(BlueprintAssignable, Category="GameMode")
    FOnActorKilled OnActorKilled;
};
