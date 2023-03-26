// Fill out your copyright notice in the Description page of Project Settings.

#include "Grid3d.h"
#include "Cell.h"
#include "WorldBorder.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include <WaterSimGameInstance.h>

Grid3d* Grid3d::grid3dInstance = nullptr;

Grid3d::Grid3d(const UWaterSimGameInstance& waterSimGameInstance) {
	xNCells = waterSimGameInstance.XCells;
	UE_LOG(LogTemp, Warning, TEXT("GridxNCells %d"), waterSimGameInstance.XCells);
	yNCells = waterSimGameInstance.YCells;
	zNCells = waterSimGameInstance.ZCells;
	xyNCells = xNCells * yNCells;
	XLeftBound = -xNCells / 2 - 1;
	XRightBound = XLeftBound + xNCells;
	YLeftBound = -yNCells / 2 - 1;
	YRightBound = YLeftBound + yNCells;
	ZLeftBound = -zNCells / 2 - 1;
	ZRightBound = ZLeftBound + zNCells;
	NCells = xNCells * yNCells * zNCells;

	CellSize = waterSimGameInstance.CellSize;
	grid3d = new Cell[NCells];
	for (int i = 0; i < NCells; i++) {
		grid3d[i].CalculatePosition(i, CellSize, XLeftBound, XRightBound, YLeftBound, YRightBound, ZLeftBound, ZRightBound);
	}
	UE_LOG(LogTemp, Warning, TEXT("Grid created")); 
}

Grid3d::~Grid3d() {
	delete[] grid3d;
}

Grid3d* Grid3d::GetInstance(const UWaterSimGameInstance& waterSimGameInstance)
{
	if (grid3dInstance == nullptr) {
		grid3dInstance = new Grid3d(waterSimGameInstance);
	}
	return grid3dInstance;
}


int Grid3d::GetTopNeighborIndex(const int& index) {
	int resultIndex = index + Grid3d::xyNCells;
	return resultIndex;
}

int Grid3d::GetFrontNeighborIndex(const int& index) {
	int resultIndex = index + xNCells;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / xyNCells > index / xyNCells)
		return -1;
	return resultIndex;
}
int Grid3d::GetRightNeighborIndex(const int& index) {
	int resultIndex = index - 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells < index / yNCells)
		return -1;
	return resultIndex;
}

int Grid3d::GetBehindNeighborIndex(const int& index) {
	int resultIndex = index - xNCells;
	if (resultIndex / xyNCells < index / xyNCells)
		return -1;
	return resultIndex;
}

int Grid3d::GetLeftNeighborIndex(const int& index) {
	int resultIndex = index + 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells > index / yNCells)
		return -1;
	return resultIndex;
}

int Grid3d::GetBottomNeighborIndex(const int& index) {
	int resultIndex = index - xyNCells;
	return resultIndex;
}

int Grid3d::GetCellIndexAtSnappedPosition(std::unique_ptr<FIntVector> cellPosition) {
	if (!CheckIfInBoundaries(cellPosition->X, cellPosition->Y, cellPosition->Z))
	{
		return -1;
	}
	const std::unique_ptr<FIntVector> localCoordinates = TranslateCellCoordinatesToLocal(std::move(cellPosition));
	return localCoordinates->X + xNCells * localCoordinates->Y + xyNCells * localCoordinates->Z;
}
const std::unique_ptr<FIntVector> Grid3d::TranslateCellCoordinatesToLocal(std::unique_ptr<FIntVector> cellPosition) {

	return std::make_unique<FIntVector>(cellPosition->X - XLeftBound, cellPosition->Y - YLeftBound, cellPosition->Z - ZLeftBound);
}

bool Grid3d::CheckIfInBoundaries(const int& x, const int& y, const int& z) {
	if (x < XLeftBound || x > XRightBound - 1)
		return false;
	else if (y < YLeftBound || y > YRightBound - 1)
		return false;
	else if (z < ZLeftBound || z > ZRightBound - 1)
		return false;
	return true;
}


int Grid3d::GetCellIndexAtFloatPosition(std::shared_ptr<FVector> position) {

	std::unique_ptr<FIntVector> gridTranslatedPosition = std::make_unique<FIntVector>((const int)(position->X / CellSize), (const int)(position->Y / CellSize), (const int)(position->Z / CellSize));

	//UE_LOG(LogTemp, Warning, TEXT("gridTranslatedPosition: %d, %d, %d"), gridTranslatedPosition->X, gridTranslatedPosition->Y, gridTranslatedPosition->Z);
	//UE_LOG(LogTemp, Warning, TEXT("spawnPositionPosition: %f, %f, %f"), spawnPosition->X, spawnPosition->Y, spawnPosition->Z);

	return GetCellIndexAtSnappedPosition(std::move(gridTranslatedPosition));
}


bool Grid3d::CheckIfCellFree(const int& cellIndex) {
	if (!CheckIfCellWIthinBounds(cellIndex)) {
		//UE_LOG(LogTemp, Warning, TEXT("Cell index %d out of bounds! Something went wrong!"), cellIndex);
		return false;
	}
	if (CheckIfBlockCell(cellIndex))
		return false;
	if (GetWaterCubeIfVisible(cellIndex) != nullptr)
		return false;
	return true;
}

bool Grid3d::CheckIfCellWIthinBounds(const int& index) {
	return (index > -1 && index < NCells);
}

bool Grid3d::CheckIfBlockCell(const int& index) {
	return grid3d[index].blockCell;
}

AWaterCube* Grid3d::GetWaterCubeIfPresent(const int& index) {
	if (grid3d[index].WaterCube == nullptr) {
		return nullptr;
	}
	return grid3d[index].WaterCube;
}

AWaterCube* Grid3d::GetWaterCubeIfVisible(const int& index) {
	if (grid3d[index].WaterCube == nullptr) {
		return nullptr;
	}
	if (IsWaterCubeHiddenHere(index))
		return nullptr;
	return grid3d[index].WaterCube;
}

bool Grid3d::IsWaterCubeHiddenHere(const int& index) {
	if (grid3d[index].WaterCube == nullptr) {
		return false;
	}
	if (!grid3d[index].WaterCube->IsHidden())
		return false;
	return true;
}

void Grid3d::SetWaterCubeVisibility(const int& index, bool state) {
	if (grid3d[index].WaterCube == nullptr)
		return;
	grid3d[index].WaterCube->SetActorHiddenInGame(!state);
}

float Grid3d::GetCurrentWaterLevel(const int& index) {
	return grid3d[index].currentWaterLevel;
}

void Grid3d::SetNextIterationWaterLevel(const int& index, const float& waterLevel) {
	grid3d[index].nextIterationWaterLevel = waterLevel;
}

float Grid3d::GetNextIterationWaterLevel(const int& index) {
	return grid3d[index].nextIterationWaterLevel;
}

void Grid3d::SetWaterLevel(const int& index, const float& waterLevel) {
	grid3d[index].currentWaterLevel = waterLevel;
}

bool Grid3d::CheckIfFullCapacityReached(const int& index, const float& level) {
	if (level > GetWaterCapacity(index) - PRECISION_OFFSET)
		return true;
	return false;
}

void Grid3d::SetNextIterationCapacity(const int& index, const float& level) {
	grid3d[index].WaterCube->nextIterationWaterCapacity = level;
}

float Grid3d::GetNextIterationCapacity(const int& index) {
	return grid3d[index].WaterCube->nextIterationWaterCapacity;
}

void Grid3d::SetCapacityDetermined(const int& index, bool state) {
	grid3d[index].WaterCube->isCapacityUndetermined = true;
}

bool Grid3d::GetCapacityDetermined(const int& index) {
	return grid3d[index].WaterCube->isCapacityUndetermined;
}

float Grid3d::CalculateWaterOverload(const int& index) {
	float amount = GetCurrentWaterLevel(index) + GetWaterSpilt(index);
	if (amount < 0.1f)
		return 0.1f;
	return amount / GetWaterCapacity(index);
}

float Grid3d::GetWaterSpilt(const int& index) {
	return grid3d[index].waterSpilt;
}

void Grid3d::AddWaterSpilt(const int& index, const float& amount) {
	grid3d[index].waterSpilt += amount;
}

void Grid3d::SetWaterSpilt(const int& index, const float& amount) {
	grid3d[index].waterSpilt = amount;
}

float Grid3d::GetWaterCapacity(const int& index) {
	//UE_LOG(LogTemp, Warning, TEXT("grid3d[index].WaterCube->currentWaterCapacity: %f"), grid3d[index].WaterCube->currentWaterCapacity)
	return grid3d[index].WaterCube->currentWaterCapacity;
}

void Grid3d::SetWaterCubeInTheGrid(AWaterCube* newWaterCube, const int& cellIndex) {
	grid3d[cellIndex].WaterCube = std::move(newWaterCube);
	newWaterCube->SetCurrentGridIndex(cellIndex);
}

void Grid3d::SetBlockCubeInTheGrid(int cellIndex) {
	grid3d[cellIndex].blockCell = true;
}

const UE::Math::TVector<double>* Grid3d::GetCellPosition(const int& index)
{
	return grid3d[index].GetPosition();
}

void Grid3d::UpdateCubesTransform() {
	for (int i = 0; i < NCells; i++) {
		grid3d[i].AdjustWaterCubesTransformIfPresent(CellSize);
	}
}