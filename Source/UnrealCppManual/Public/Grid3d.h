// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cell.h"
#include "WorldBorder.h"
#include "WaterSimGameInstance.h"

class UNREALCPPMANUAL_API Grid3d
{
private:

	Grid3d(const UWaterSimGameInstance& waterSimGameInstance);
	~Grid3d();
	Grid3d(Grid3d& other) = delete;
	void operator=(const Grid3d&) = delete;
	static Grid3d* grid3dInstance;
	Cell* grid3d;

public:

	static Grid3d* Grid3d::GetInstance(const UWaterSimGameInstance &waterSimGameInstance);

	int xNCells;
	int yNCells;
	int zNCells;
	int xyNCells;
	int NCells;

	int XLeftBound;
	int XRightBound;
	int YLeftBound;
	int YRightBound;
	int ZLeftBound;
	int ZRightBound;

	int CellSize;

	int GetTopNeighborIndex(const int& index);
	int GetFrontNeighborIndex(const int& index);
	int GetRightNeighborIndex(const int& index);
	int GetBehindNeighborIndex(const int& index);
	int GetLeftNeighborIndex(const int& index);
	int GetBottomNeighborIndex(const int& index);
	int GetCellIndexAtSnappedPosition(std::unique_ptr<FIntVector> cellPosition);
	const std::unique_ptr<FIntVector> TranslateCellCoordinatesToLocal(std::unique_ptr<FIntVector> cellPosition);
	bool CheckIfInBoundaries(const int& x, const int& y, const int& z);
	int GetCellIndexAtFloatPosition(std::shared_ptr<FVector> position);
	bool CheckIfCellFree(const int& cellIndex);
	bool IsWaterCubeHiddenHere(const int& index);
	AWaterCube* GetWaterCubeIfPresent(const int& index);
	void SetWaterCubeVisibility(const int& index, bool state);
	void SetWaterCubeInTheGrid(AWaterCube* newWaterCube, const int& cellIndex);
	void SetBlockCubeInTheGrid(int cellIndex);
	void SetWaterLevel(const int& index, const float& waterLevel);
	const UE::Math::TVector<double>* GetCellPosition(const int& index);
	bool CheckIfCellWIthinBounds(const int& index);
	bool CheckIfBlockCell(const int& index);
	AWaterCube* GetWaterCubeIfVisible(const int& index);
	float GetCurrentWaterLevel(const int& index);
	void SetNextIterationWaterLevel(const int& index, const float& waterLevel);
	float GetNextIterationWaterLevel(const int& index);
	void SetNextIterationCapacity(const int& index, const float& level);
	float GetNextIterationCapacity(const int& index);
	bool CheckIfFullCapacityReached(const int& index, const float& level);
	void SetCapacityDetermined(const int& index, bool state);
	bool GetCapacityDetermined(const int& index);
	float CalculateWaterOverload(const int& index);
	float GetWaterSpilt(const int& index);
	void AddWaterSpilt(const int& index, const float& amount);
	void SetWaterSpilt(const int& index, const float& amount);
	float GetWaterCapacity(const int& index);
	void UpdateCubesTransform();
};

