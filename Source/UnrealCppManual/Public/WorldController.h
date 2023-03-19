// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Cell.h"
#include "GameFramework/Actor.h"
#include "WorldController.generated.h"

#define SIMULATION_SPEED 0.2
#define BASE_CAPACITY 1.0
#define EXCEED_MODIFIER 0.02
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
	virtual void Tick(float DeltaTime) override;
	void CreateWorldBorders();
	void SpawnWorldBorder(FVector spawn, UE::Math::TVector<double> scale, FRotator3d rotator);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:

	// the grid consists of cells that are within 3-dimensional bounds. Those bounds define the piece of space for simulation.
	// Those boundaries are in the following ranges:
	// x: [XLEFTBOUND, XRIGHTBOUND - 1],
	// y: [YLEFTBOUND, YRIGHTBOUND - 1],
	// z: [ZLEFTBOUND, ZRIGHTBOUND - 1]

	float gameTimeElapsed;
	int simCounter;
#pragma region Physics
	void Gravity(const int& index);
	void SpillAround(const int& index);
	bool IsNeighbourFreeToBeSpilledTo(const int& currentIndex, const int& neighbourIndex);
	bool CanWaterFallDown(const int& currentIndex);
	bool CanWaterSpillAround(const int& index);
	void HandleSpiltWater();
	void ApplyNextIterWaterToCurrent();
	void ApplySimulationProccesses();
	void CalculateWaterCubeCapacity();
	void GetHigherCapacity(float& currCapacity, const int& index, bool& isWaterAround);
	void ApplyCalculatedCapacities();
	bool SetByNeighbourWaterCapacityIfPresent(const int& index);
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
