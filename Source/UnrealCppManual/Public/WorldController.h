// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cell.h"
#include "WorldController.generated.h"

UCLASS()
class UNREALCPPMANUAL_API AWorldController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldController();
	~AWorldController();

	// Called every frame
	int GetCellIndexAtFloatPosition(std::shared_ptr<FVector> position);
	bool CheckIfCellFree(const int& cellIndex);
	void SetWaterCubeInTheGrid(AWaterCube* newWaterCube, const int& cellIndex);
	void SetBlockCubeInTheGrid(int cellIndex);
	const UE::Math::TVector<double>* GetCellPosition(const int& index);
	void DetachWaterCubeFromTheCell(const int& index);
	void DestroyWaterCubeActor(const int& index);
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:

	int XLeftBound; 
	int XRightBound;
	int YLeftBound; 
	int YRightBound;
	int ZLeftBound; 
	int ZRightBound;

	// the grid consists of cells that are within 3-dimensional bounds. Those bounds define the piece of space for simulation.
	// Those boundaries are in the following ranges:
	// x: [XLeftBound, XRightBound - 1],
	// y: [YLeftBound, YRightBound - 1],
	// z: [ZLeftBound, ZRightBound - 1]

	Cell* grid3d;
	float gameTimeElapsed;
	
	bool CheckIfInBoundaries(const int& x, const int& y, const int& z);
	bool CheckIfCellWIthinBounds(const int& index);
	bool CheckIfBlockCell(const int& index);
	AWaterCube* GetWaterCubeIfPresent(const int& index);
	float& GetCurrentWaterLevel(const int& index);
	void SetNextIterationWaterLevel(const int& index, const float& waterLevel);
	void SetWaterLevel(const int& index, const float& waterLevel);
	float& GetWaterSpilt(const int& index);
	void AddWaterSpilt(const int& index, const float& amount);
	void SetWaterSpilt(const int& index, const float& amount);
	int GetCellIndexAtSnappedPosition(std::unique_ptr<FIntVector> cellPosition);
	const std::unique_ptr<FIntVector> TranslateCellCoordinatesToLocal(std::unique_ptr<FIntVector> cellPosition);
	FVector* GetCellPositionFromIndex(int index);
	int GetXCoordFromCellIndex(int index);
	int GetYCoordFromCellIndex(int index);
	int GetZCoordFromCellIndex(int index);

	int GetTopNeighborIndex(const int& index);
	int GetFrontNeighborIndex(const int& index);
	int GetRightNeighborIndex(const int& index);
	int GetBehindNeighborIndex(const int& index);
	int GetLeftNeighborIndex(const int& index);
	int GetBottomNeighborIndex(const int& index);
	void UpdateWaterCubePosition(const int& index);
	void MoveTheWaterCube(const int& fromIndex, const int& toIndex);
	void Gravity(const int& index);
	void SpillAround(const int& index);
	bool IsNeighbourFreeToBeSpilledTo(const int& currentIndex, const int& neighbourIndex);
	void HandleSpiltWater();
	void SpreadOverwateredCell(const int& index);
	void ApplySimulationProccesses();
};
