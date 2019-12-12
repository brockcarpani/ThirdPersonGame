// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSGameMode.h"
#include "TimerManager.h"
#include "TPSHealthComponent.h"
#include "AI/TPSTrackerBot.h" //

ATPSGameMode::ATPSGameMode()
{
	TimeBetweenWaves = 2.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

void ATPSGameMode::StartWave()
{
	WaveCount = 4;//WaveCount++;
	NumOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ATPSGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

void ATPSGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}

void ATPSGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ATPSGameMode::StartWave, TimeBetweenWaves, false);
}

void ATPSGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NumOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}

	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		APawn* TestPawn = It->Get();
		ATPSTrackerBot* TrackerTestPawn = Cast<ATPSTrackerBot>(TestPawn); //
		if (TrackerTestPawn == nullptr || TrackerTestPawn->IsPlayerControlled()) // Test Pawn
		{
			continue;
		}

		UTPSHealthComponent* HealthComp = Cast<UTPSHealthComponent>(TrackerTestPawn->GetComponentByClass(UTPSHealthComponent::StaticClass())); // Test Pawn
		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		PrepareForNextWave();
	}
}

void ATPSGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			UTPSHealthComponent* HealthComp = Cast<UTPSHealthComponent>(MyPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				return;
			}
		}
	}

	GameOver();
}

void ATPSGameMode::GameOver()
{
	EndWave();

	UE_LOG(LogTemp, Log, TEXT("Game Over"));
}

void ATPSGameMode::RestartDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}
}

void ATPSGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ATPSGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
	CheckAnyPlayerAlive();
	RestartDeadPlayers();
}

void ATPSGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NumOfBotsToSpawn--;

	if (NumOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}