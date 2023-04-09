// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Cell.h"
#include "GameFramework/Actor.h"
#include "WaterSimGameInstance.h"
#include "WorldController.generated.h"

#define BASE_CAPACITY 1.0
#define EXCEED_MODIFIER 0.04
#define MAX_PRESSURED_AMOUNT_ALLOWED_TO_SPREAD 1.0

UCLASS()
class UNREALCPPMANUAL_API AWorldController : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AWorldController();
	~AWorldController();
	class UWaterSimGameInstance* wsgi;
	float simThreshold;

	float moreOftenSimThreshold;

	FRotator3d* defaultRotation;
	virtual void Tick(float DeltaTime) override;
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
	float moreOftenGameTimeElapsed;
	int simCounter;
#pragma region Physics
	void Gravity(const int& index);
	void SpillAround(const int& index);
	bool IsNeighbourFreeToBeSpilledTo(const int& currentIndex, const int& neighbourIndex);
	bool CanWaterFallDown(const int& currentIndex);
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
	void FlowPressurizedWaterUpwards();
	float GetFreeAmountInCell(const int& index);
#pragma endregion Physics
	std::vector<int> Get4HorizontalNeighbourIndices(const int& index);
	std::vector<int> Get6AdjacentNeighbourIndices(const int& index);
};
