// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorSpawner.h"
#include "Components/BoxComponent.h"
#include "WaterCube.h"
#include "BlockCube.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include <WorldBorder.h>

// Sets default values
AActorSpawner::AActorSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));

	SpawnVolume->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	defaultRotation = new FRotator3d();
}
AActorSpawner::~AActorSpawner() {
	delete defaultRotation;
}
void AActorSpawner::SpawnWaterCube()
{
	const std::shared_ptr<FVector> SpawnerLocation = std::make_shared<FVector>(spawner->GetActorLocation());
	const int& cellIndex = worldController->GetCellIndexAtFloatPosition(SpawnerLocation);

	if (worldController->CheckIfCellFree(cellIndex))
	{
		AWaterCube* newCube = GetWorld()->SpawnActor<AWaterCube>((FVector) *worldController->GetCellPosition(cellIndex), *defaultRotation);
		worldController->SetWaterCubeInTheGrid(newCube, cellIndex);
		worldController->SetWaterLevel(cellIndex, 1.0f);
	}
}

void AActorSpawner::SpawnBlockCube()
{
	const std::shared_ptr<FVector> SpawnerLocation = std::make_shared<FVector>(spawner->GetActorLocation());
	const int& cellIndex = worldController->GetCellIndexAtFloatPosition(SpawnerLocation);

	if (worldController->CheckIfCellFree(cellIndex))
	{
		ABlockCube* newCube = GetWorld()->SpawnActor<ABlockCube>((FVector)*worldController->GetCellPosition(cellIndex), *defaultRotation);
		worldController->SetBlockCubeInTheGrid(cellIndex);
	}
}


