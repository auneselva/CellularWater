// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorSpawner.h"
#include "Components/BoxComponent.h"
#include "WaterCube.h"
#include "BlockCube.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include <WorldBorder.h>
#include "Grid3d.h"
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
	const int& cellIndex = Grid3d::GetInstance()->GetCellIndexAtFloatPosition(SpawnerLocation);

	if (Grid3d::GetInstance()->CheckIfCellFree(cellIndex))
	{
		if (Grid3d::GetInstance()->GetWaterCubeIfPresent(cellIndex) != nullptr)
			Grid3d::GetInstance()->SetWaterCubeVisibility(cellIndex, true);
		else
		{
			AWaterCube* newCube = GetWorld()->SpawnActor<AWaterCube>((FVector)*Grid3d::GetInstance()->GetCellPosition(cellIndex), *defaultRotation);
			Grid3d::GetInstance()->SetWaterCubeInTheGrid(newCube, cellIndex);
		}
		Grid3d::GetInstance()->SetWaterLevel(cellIndex, 1.0f);
	}
}

void AActorSpawner::SpawnBlockCube()
{
	const std::shared_ptr<FVector> SpawnerLocation = std::make_shared<FVector>(spawner->GetActorLocation());
	const int& cellIndex = Grid3d::GetInstance()->GetCellIndexAtFloatPosition(SpawnerLocation);

	if (Grid3d::GetInstance()->CheckIfCellFree(cellIndex))
	{
		ABlockCube* newCube = GetWorld()->SpawnActor<ABlockCube>((FVector)*Grid3d::GetInstance()->GetCellPosition(cellIndex), *defaultRotation);
		Grid3d::GetInstance()->SetBlockCubeInTheGrid(cellIndex);
	}
}
void AActorSpawner::SpawnWorldBorder(FVector spawn, UE::Math::TVector<double> scale, FRotator3d rotator) {

	AWorldBorder* worldBorder = GetWorld()->SpawnActor<AWorldBorder>(spawn, rotator);
	worldBorder->SetActorScale3D(std::move(scale));
}

