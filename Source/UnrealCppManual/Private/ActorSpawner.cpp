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

void AActorSpawner::BeginPlay()
{
	waterSimGameInstance = Cast<UWaterSimGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	Super::BeginPlay();
	CreateWorldBorders();

}

void AActorSpawner::SpawnWaterCube()
{
	const std::shared_ptr<FVector> SpawnerLocation = std::make_shared<FVector>(spawner->GetActorLocation());
	const int& cellIndex = Grid3d::GetInstance(*waterSimGameInstance)->GetCellIndexAtFloatPosition(SpawnerLocation);

	if (Grid3d::GetInstance(*waterSimGameInstance)->CheckIfCellFree(cellIndex))
	{
		if (Grid3d::GetInstance(*waterSimGameInstance)->GetWaterCubeIfPresent(cellIndex) != nullptr)
			Grid3d::GetInstance(*waterSimGameInstance)->SetWaterCubeVisibility(cellIndex, true);
		else
		{
			AWaterCube* newCube = GetWorld()->SpawnActor<AWaterCube>((FVector)*Grid3d::GetInstance(*waterSimGameInstance)->GetCellPosition(cellIndex), *defaultRotation);
			Grid3d::GetInstance(*waterSimGameInstance)->SetWaterCubeInTheGrid(newCube, cellIndex);
		}
		Grid3d::GetInstance(*waterSimGameInstance)->SetWaterLevel(cellIndex, 1.0f);
	}
}

void AActorSpawner::SpawnBlockCube()
{
	const std::shared_ptr<FVector> SpawnerLocation = std::make_shared<FVector>(spawner->GetActorLocation());
	const int& cellIndex = Grid3d::GetInstance(*waterSimGameInstance)->GetCellIndexAtFloatPosition(SpawnerLocation);

	if (Grid3d::GetInstance(*waterSimGameInstance)->CheckIfCellFree(cellIndex))
	{
		ABlockCube* newCube = GetWorld()->SpawnActor<ABlockCube>((FVector)*Grid3d::GetInstance(*waterSimGameInstance)->GetCellPosition(cellIndex), *defaultRotation);
		Grid3d::GetInstance(*waterSimGameInstance)->SetBlockCubeInTheGrid(cellIndex);
	}
}
void AActorSpawner::SpawnWorldBorder(FVector spawn, UE::Math::TVector<double> scale, FRotator3d rotator) {

	AWorldBorder* worldBorder = GetWorld()->SpawnActor<AWorldBorder>(spawn, rotator);
	worldBorder->SetActorScale3D(std::move(scale));
}

void AActorSpawner::CreateWorldBorders() {
	// 12 orange lines

	int xNCells = Grid3d::GetInstance(*waterSimGameInstance)->xNCells;
	int yNCells = Grid3d::GetInstance(*waterSimGameInstance)->yNCells;
	int zNCells = Grid3d::GetInstance(*waterSimGameInstance)->zNCells;
	int XLeftBound = Grid3d::GetInstance(*waterSimGameInstance)->XLeftBound;
	int XRightBound = Grid3d::GetInstance(*waterSimGameInstance)->XRightBound;
	int YLeftBound = Grid3d::GetInstance(*waterSimGameInstance)->YLeftBound;
	int YRightBound = Grid3d::GetInstance(*waterSimGameInstance)->YRightBound;
	int ZLeftBound = Grid3d::GetInstance(*waterSimGameInstance)->ZLeftBound;
	int ZRightBound = Grid3d::GetInstance(*waterSimGameInstance)->ZRightBound;


	//down 
	SpawnWorldBorder(FVector(CELL_SIZE * (XLeftBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YRightBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, xNCells), FRotator3d(90.0f, 0.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, xNCells), FRotator3d(90.0f, 0.0f, 0.0f));


	// vertical
	SpawnWorldBorder(FVector(CELL_SIZE * (XLeftBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, zNCells), FRotator3d(0.0f, 90.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, zNCells), FRotator3d(0.0f, 90.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XLeftBound - 0.5f), CELL_SIZE * (YRightBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, zNCells), FRotator3d(0.0f, 90.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YRightBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, zNCells), FRotator3d(0.0f, 90.0f, 0.0f));

	//up
	SpawnWorldBorder(FVector(CELL_SIZE * (XLeftBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZRightBound), UE::Math::TVector<double>(0.1f, 0.1f, yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZRightBound), UE::Math::TVector<double>(0.1f, 0.1f, yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YRightBound - 0.5f), CELL_SIZE * ZRightBound), UE::Math::TVector<double>(0.1f, 0.1f, xNCells), FRotator3d(90.0f, 0.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZRightBound), UE::Math::TVector<double>(0.1f, 0.1f, xNCells), FRotator3d(90.0f, 0.0f, 0.0f));


}