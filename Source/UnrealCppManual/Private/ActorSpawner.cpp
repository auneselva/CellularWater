// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorSpawner.h"
#include "Components/BoxComponent.h"
#include "WaterCell.h"

// Sets default values
AActorSpawner::AActorSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));

	SpawnVolume->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}
void AActorSpawner::SpawnActor()
{
	FVector SpawnerLocation = mainCameraPawn->StaticMeshComp->GetComponentLocation();
	auto[cellPosition, cellIndex] = worldController->GetCellIndexAndNewPosition(SpawnerLocation);

	if (worldController->CheckIfCellFree(cellIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Water cell spawned! Cell index: %d "), cellIndex);
		FRotator SpawnRotation = FRotator3d();
		AWaterCell* newWaterCell = GetWorld()->SpawnActor<AWaterCell>(cellPosition, SpawnRotation);
		worldController->SetCellInTheGrid(newWaterCell, cellIndex);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Not spawned! Cell index: %d "), cellIndex);
}

// Called when the game starts or when spawned
void AActorSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AActorSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

