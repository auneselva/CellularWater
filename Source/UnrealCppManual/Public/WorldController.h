// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cell.h"
#include "WorldController.generated.h"

#define BOUND 3
#define N_CELLS (BOUND + BOUND) * (BOUND + BOUND) * (BOUND + BOUND)
#define CELL_SIZE 100.0
#define SIMULATION_SPEED 0.2
#define BASE_CAPACITY 1.0
#define EXCEED_MODIFIER 0.02
#define PRECISION_OFFSET 0.00001
#define MAX_PRESSURED_AMOUNT_ALLOWED_TO_SPREAD 1.0

UCLASS()
class UNREALCPPMANUAL_API AWorldController : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AWorldController();
	~AWorldController();
	FRotator3d* defaultRotation;
	int GetCellIndexAtFloatPosition(std::shared_ptr<FVector> position);
	bool CheckIfCellFree(const int& cellIndex);
	bool IsWaterCubeHiddenHere(const int& index);
	AWaterCube* GetWaterCubeIfPresent(const int& index);
	void SetWaterCubeVisibility(const int& index, bool state);
	void SetWaterCubeInTheGrid(AWaterCube* newWaterCube, const int& cellIndex);
	void SetBlockCubeInTheGrid(int cellIndex);
	void SetWaterLevel(const int& index, const float& waterLevel);
	const UE::Math::TVector<double>* GetCellPosition(const int& index);
	void DetachWaterCubeFromTheCell(const int& index);
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


	int xNCells;
	int yNCells;
	int zNCells;
	int xyNCells;
	// the grid consists of cells that are within 3-dimensional bounds. Those bounds define the piece of space for simulation.
	// Those boundaries are in the following ranges:
	// x: [XLeftBound, XRightBound - 1],
	// y: [YLeftBound, YRightBound - 1],
	// z: [ZLeftBound, ZRightBound - 1]

	Cell* grid3d;
	float gameTimeElapsed;
	int simCounter;
	void CreateWorldBorders();
	void SpawnWorldBorder(FVector spawn, UE::Math::TVector<double> scale, FRotator3d rotator);
	bool CheckIfInBoundaries(const int& x, const int& y, const int& z);
	bool CheckIfCellWIthinBounds(const int& index);
	bool CheckIfBlockCell(const int& index);
	AWaterCube* GetWaterCubeIfVisible(const int& index);
	float GetCurrentWaterLevel(const int& index);
	void SetNextIterationWaterLevel(const int& index, const float& waterLevel);
	bool CheckIfFullCapacityReached(const int& index, const float& level);
	float CalculateWaterOverload(const int& index);
	bool SetByNeighbourWaterCapacityIfPresent(const int& index);
	float GetWaterSpilt(const int& index);
	void AddWaterSpilt(const int& index, const float& amount);
	void SetWaterSpilt(const int& index, const float& amount);
	float GetWaterCapacity(const int& index);
	int GetCellIndexAtSnappedPosition(std::unique_ptr<FIntVector> cellPosition);
	const std::unique_ptr<FIntVector> TranslateCellCoordinatesToLocal(std::unique_ptr<FIntVector> cellPosition);
	FVector* GetCellPositionFromIndex(int index);
	int GetXCoordFromCellIndex(int index);
	int GetYCoordFromCellIndex(int index);
	int GetZCoordFromCellIndex(int index);

	int GetTopNeighborIndex(const int& index);
	int GetFrontNeighborIndex(const int& index);
	int GetFrontRightNeighborIndex(const int& index); //by diagonal
	int GetRightNeighborIndex(const int& index);
	int GetRightBehindNeighborIndex(const int& index); //by diagonal
	int GetBehindNeighborIndex(const int& index);
	int GetBehindLeftNeighborIndex(const int& index); //by diagonal
	int GetLeftNeighborIndex(const int& index);
	int GetLeftFrontNeighborIndex(const int& index); //by diagonal
	int GetBottomNeighborIndex(const int& index);
	void UpdateWaterCubePosition(const int& index);
	void MoveTheWaterCube(const int& fromIndex, const int& toIndex);
#pragma region Physics
	void Gravity(const int& index);
	void SpillAround(const int& index);
	bool IsNeighbourFreeToBeSpilledTo(const int& currentIndex, const int& neighbourIndex);
	bool CanWaterFallDown(const int& currentIndex);
	bool CanWaterSpillAround(const int& index);
	void HandleSpiltWater();
	void ApplyNextIterWaterToCurrent();
	void SpreadOverwateredCell(const int& index);
	void ApplySimulationProccesses();
	void CalculateWaterCubeCapacity();
	void GetHigherCapacity(float& currCapacity, const int& index, bool& isWaterAround);
	void ApplyCalculatedCapacities();
	void ClusterizeWaterGroupsOnLevels();
	void TraverseAdjacentWaters(const int& currentCluster, const int& startIdx);
	void SetAllCapacitiesInClusterToHighest(const int& nClusters, const int& firstIdx, const int& lastIdx);
	void ResetClusters();
	void DetermineWaterFlow();
	void EvaluateFlowFromNeighbours(const int& index);
	void FlowAccordingToPressure(const int& index);
	void FlowPressurizedWaterUpwards();
	float GetWaterOverloadInCell(const int& index);
	float GetWaterAmountDiff(const int& index, const int& neighbourIndex);
#pragma endregion Physics
};
