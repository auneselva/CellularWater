// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldController.h"
#include "WaterCube.h"
#include "Grid3d.h"
#include "Cell.h"
#include <vector>
#include <algorithm>
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include "ActorSpawner.h"
#include <WorldBorder.h>
AWorldController::AWorldController()
{
	PrimaryActorTick.bCanEverTick = true;
	defaultRotation = new FRotator3d();
	xNCells = XRIGHTBOUND - XLEFTBOUND;
	yNCells = YRIGHTBOUND - YLEFTBOUND;
	zNCells = ZRIGHTBOUND - ZLEFTBOUND;
	xyNCells = (XRIGHTBOUND - XLEFTBOUND) * (YRIGHTBOUND - YLEFTBOUND);

	grid3d = new Cell[N_CELLS];
	for (int i = 0; i < N_CELLS; i++) {
		grid3d[i].CalculatePosition(i, CELL_SIZE, XLEFTBOUND, XRIGHTBOUND, YLEFTBOUND, YRIGHTBOUND, ZLEFTBOUND, ZRIGHTBOUND);
		
	}
	//SpawnWorldBorders();
	//UE_LOG(LogTemp, Warning, TEXT("Grid3d %d"), grid3d[7999].WaterCube );

}
void AWorldController::BeginPlay()
{
	Super::BeginPlay();

	gameTimeElapsed = 0;
	simCounter = 0;
	CreateWorldBorders();
}
AWorldController::~AWorldController()
{
	delete[] grid3d;
	delete defaultRotation;
}

void AWorldController::CreateWorldBorders() {
	// 12 orange lines

	//down 
	SpawnWorldBorder(FVector(CELL_SIZE * (XLEFTBOUND - 0.5f), CELL_SIZE * (YLEFTBOUND - 0.5f), CELL_SIZE * ZLEFTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRIGHTBOUND - 0.5f), CELL_SIZE * (YLEFTBOUND - 0.5f), CELL_SIZE * ZLEFTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRIGHTBOUND - 0.5f), CELL_SIZE * (YRIGHTBOUND - 0.5f), CELL_SIZE * ZLEFTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)xNCells), FRotator3d(90.0f, 0.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRIGHTBOUND - 0.5f), CELL_SIZE * (YLEFTBOUND - 0.5f), CELL_SIZE * ZLEFTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)xNCells), FRotator3d(90.0f, 0.0f, 0.0f));


	// vertical
	SpawnWorldBorder(FVector(CELL_SIZE * (XLEFTBOUND - 0.5f), CELL_SIZE * (YLEFTBOUND - 0.5f), CELL_SIZE * ZLEFTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)zNCells), FRotator3d(0.0f, 90.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRIGHTBOUND - 0.5f), CELL_SIZE * (YLEFTBOUND - 0.5f), CELL_SIZE * ZLEFTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)zNCells), FRotator3d(0.0f, 90.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XLEFTBOUND - 0.5f), CELL_SIZE * (YRIGHTBOUND - 0.5f), CELL_SIZE * ZLEFTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)zNCells), FRotator3d(0.0f, 90.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRIGHTBOUND - 0.5f), CELL_SIZE * (YRIGHTBOUND - 0.5f), CELL_SIZE * ZLEFTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)zNCells), FRotator3d(0.0f, 90.0f, 0.0f));

	//up
	SpawnWorldBorder(FVector(CELL_SIZE * (XLEFTBOUND - 0.5f), CELL_SIZE * (YLEFTBOUND - 0.5f), CELL_SIZE * ZRIGHTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRIGHTBOUND - 0.5f), CELL_SIZE * (YLEFTBOUND - 0.5f), CELL_SIZE * ZRIGHTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRIGHTBOUND - 0.5f), CELL_SIZE * (YRIGHTBOUND - 0.5f), CELL_SIZE * ZRIGHTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)xNCells), FRotator3d(90.0f, 0.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRIGHTBOUND - 0.5f), CELL_SIZE * (YLEFTBOUND - 0.5f), CELL_SIZE * ZRIGHTBOUND), UE::Math::TVector<double>(0.1f, 0.1f, (float)xNCells), FRotator3d(90.0f, 0.0f, 0.0f));


}
void AWorldController::SpawnWorldBorder(FVector spawn, UE::Math::TVector<double> scale, FRotator3d rotator) {

	AWorldBorder* worldBorder = GetWorld()->SpawnActor<AWorldBorder>(spawn, rotator);
	worldBorder->SetActorScale3D(std::move(scale));
}

int AWorldController::GetCellIndexAtSnappedPosition(std::unique_ptr<FIntVector> cellPosition) {
	if (!CheckIfInBoundaries(cellPosition->X, cellPosition->Y, cellPosition->Z))
	{
		return -1;
	}
	const std::unique_ptr<FIntVector> localCoordinates = TranslateCellCoordinatesToLocal(std::move(cellPosition));
	return localCoordinates->X + xNCells * localCoordinates->Y + xyNCells * localCoordinates->Z;
}
const std::unique_ptr<FIntVector> AWorldController::TranslateCellCoordinatesToLocal(std::unique_ptr<FIntVector> cellPosition) {

	return std::make_unique<FIntVector>(cellPosition->X - XLEFTBOUND, cellPosition->Y - YLEFTBOUND, cellPosition->Z - ZLEFTBOUND);
}

bool AWorldController::CheckIfInBoundaries(const int& x, const int& y, const int& z) {
	if (x < XLEFTBOUND || x > XRIGHTBOUND - 1)
		return false;
	else if (y < YLEFTBOUND || y > YRIGHTBOUND - 1)
		return false;
	else if (z < ZLEFTBOUND || z > ZRIGHTBOUND - 1)
		return false;
	return true;
}

int AWorldController::GetCellIndexAtFloatPosition(std::shared_ptr<FVector> position) {

	std::unique_ptr<FIntVector> gridTranslatedPosition = std::make_unique<FIntVector>((const int)(position->X / CELL_SIZE), (const int)(position->Y / CELL_SIZE), (const int)(position->Z / CELL_SIZE));
	
	//UE_LOG(LogTemp, Warning, TEXT("gridTranslatedPosition: %d, %d, %d"), gridTranslatedPosition->X, gridTranslatedPosition->Y, gridTranslatedPosition->Z);
	//UE_LOG(LogTemp, Warning, TEXT("spawnPositionPosition: %f, %f, %f"), spawnPosition->X, spawnPosition->Y, spawnPosition->Z);

	return GetCellIndexAtSnappedPosition(std::move(gridTranslatedPosition));
}

bool AWorldController::CheckIfCellFree(const int& cellIndex) {
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

bool AWorldController::CheckIfCellWIthinBounds(const int& index) {
	return (index > -1 && index < N_CELLS);
}

bool AWorldController::CheckIfBlockCell(const int& index) {
	return grid3d[index].blockCell;
}

AWaterCube* AWorldController::GetWaterCubeIfPresent(const int& index) {
	if (grid3d[index].WaterCube == nullptr) {
		return nullptr;
	}
	return grid3d[index].WaterCube;
}

AWaterCube* AWorldController::GetWaterCubeIfVisible(const int& index) {
	if (grid3d[index].WaterCube == nullptr) {
		return nullptr;
	}
	if (IsWaterCubeHiddenHere(index))
		return nullptr;
	return grid3d[index].WaterCube;
}

bool AWorldController::IsWaterCubeHiddenHere(const int& index) {
	if (grid3d[index].WaterCube == nullptr) {
		return false;
	}
	if (!grid3d[index].WaterCube->IsHidden())
		return false;
	return true;
}
void AWorldController::SetWaterCubeVisibility(const int& index, bool state) {
	if (grid3d[index].WaterCube == nullptr)
		return;
	grid3d[index].WaterCube->SetActorHiddenInGame(!state);
}
float AWorldController::GetCurrentWaterLevel(const int& index) {
	return grid3d[index].currentWaterLevel;
}
void AWorldController::SetNextIterationWaterLevel(const int& index, const float& waterLevel) {
	grid3d[index].nextIterationWaterLevel = waterLevel;
}
void AWorldController::SetWaterLevel(const int& index, const float& waterLevel) {
	grid3d[index].currentWaterLevel = waterLevel;
}

bool AWorldController::CheckIfFullCapacityReached(const int& index, const float& level) {
	if (level > GetWaterCapacity(index) - PRECISION_OFFSET)
		return true;
	return false;
}

float AWorldController::CalculateWaterOverload(const int& index) {
	float amount = GetCurrentWaterLevel(index) + GetWaterSpilt(index);
	if (amount < 0.1f)
		return 0.1f;
	return amount / GetWaterCapacity(index);
}

float AWorldController::GetWaterSpilt(const int& index) {
	return grid3d[index].waterSpilt;
}

void AWorldController::AddWaterSpilt(const int& index, const float& amount) {
	grid3d[index].waterSpilt += amount;
}
void AWorldController::SetWaterSpilt(const int& index, const float& amount) {
	grid3d[index].waterSpilt = amount;
}

float AWorldController::GetWaterCapacity(const int& index) {
	//UE_LOG(LogTemp, Warning, TEXT("grid3d[index].WaterCube->currentWaterCapacity: %f"), grid3d[index].WaterCube->currentWaterCapacity)
	return grid3d[index].WaterCube->currentWaterCapacity;
}

void AWorldController::SetWaterCubeInTheGrid(AWaterCube* newWaterCube, const int& cellIndex) {
	grid3d[cellIndex].WaterCube = std::move(newWaterCube);
	newWaterCube->SetCurrentGridIndex(cellIndex);
}
void AWorldController::SetBlockCubeInTheGrid(int cellIndex) {
	grid3d[cellIndex].blockCell = true;
}
const UE::Math::TVector<double>* AWorldController::GetCellPosition(const int& index)
{
	return grid3d[index].GetPosition();
}

void AWorldController::DetachWaterCubeFromTheCell(const int& index) {
	grid3d[index].WaterCube = nullptr;
}

void AWorldController::Tick(float DeltaTime) //delta time == around 0.02
{
	Super::Tick(DeltaTime);
	gameTimeElapsed += DeltaTime;
	if (gameTimeElapsed > SIMULATION_SPEED) {
		gameTimeElapsed -= SIMULATION_SPEED;
		ApplySimulationProccesses();
		simCounter++;
	}
		
}

void AWorldController::MoveTheWaterCube(const int& fromIndex, const int& toIndex) {
	SetWaterCubeInTheGrid(grid3d[fromIndex].WaterCube, toIndex);
	DetachWaterCubeFromTheCell(fromIndex);
	UpdateWaterCubePosition(toIndex);
}

void AWorldController::UpdateWaterCubePosition(const int& index) {
	grid3d[index].WaterCube->SetActorLocation(*grid3d[index].GetPosition());
}
void AWorldController::Gravity(const int& index) {

	if (CanWaterFallDown(index)) {
		int bottomIndex = Grid3d::GetBottomNeighborIndex(index, xyNCells);
		if (GetWaterCubeIfVisible(bottomIndex) != nullptr) {
			float waterAmountToBeFlown = GetWaterCapacity(bottomIndex) - GetCurrentWaterLevel(bottomIndex) - GetWaterSpilt(bottomIndex);
			
			AddWaterSpilt(bottomIndex, waterAmountToBeFlown);
			AddWaterSpilt(index, -waterAmountToBeFlown);
		}
		else {
			AddWaterSpilt(bottomIndex, GetCurrentWaterLevel(index) + GetWaterSpilt(index));
			AddWaterSpilt(index, -GetCurrentWaterLevel(index) - GetWaterSpilt(index));
		}
	}
}

void AWorldController::ApplySimulationProccesses() {
	
	for (int i = 0; i < N_CELLS; i++) {
		if (GetWaterCubeIfVisible(i) != nullptr) {
			Gravity(i);
			SpillAround(i);
		}
		if (i % xyNCells == xyNCells - 1)
			HandleSpiltWater();
	}
	/*
	UE_LOG(LogTemp, Warning, TEXT("\n\n\n\n\nWater Splitted"));
	for (int i = 0; i < N_CELLS; i++) {
		if (GetWaterSpilt(i) > PRECISION_OFFSET)
			UE_LOG(LogTemp, Warning, TEXT("%d: %f"), i, GetWaterSpilt(i));
	}

	UE_LOG(LogTemp, Warning, TEXT("\n\n\n\n\nWater Levels"));
	for (int i = 0; i < N_CELLS; i++) {
		if (GetCurrentWaterLevel(i) > PRECISION_OFFSET)
			UE_LOG(LogTemp, Warning, TEXT("%d: %f"), i, GetCurrentWaterLevel(i))
	} */
	HandleSpiltWater();
	
	CalculateWaterCubeCapacity();
	ApplyCalculatedCapacities(); //
	ClusterizeWaterGroupsOnLevels();
	for (int i = 0; i < N_CELLS; i++)
		if (GetWaterCubeIfVisible(i) != nullptr)
			UE_LOG(LogTemp, Warning, TEXT("GetCurrentWaterLevel(%d): %f, GetWaterSpilt() %f, GetWaterCapacity(): %f"), i, GetCurrentWaterLevel(i), GetWaterSpilt(i), GetWaterCapacity(i));
	
	DetermineWaterFlow();
	FlowPressurizedWaterUpwards();
	UE_LOG(LogTemp, Warning, TEXT("After Pressurizing"));
	for (int i = 0; i < N_CELLS; i++)
		if (GetWaterCubeIfVisible(i) != nullptr)
			UE_LOG(LogTemp, Warning, TEXT("GetCurrentWaterLevel(%d): %f, GetWaterSpilt() %f, GetWaterCapacity(): %f"), i, GetCurrentWaterLevel(i), GetWaterSpilt(i), GetWaterCapacity(i));

	HandleSpiltWater();
	for (int i = 0; i < N_CELLS; i++)
		grid3d[i].AdjustWaterCubesTransformIfPresent(CELL_SIZE);
}

void AWorldController::SpillAround(const int& index) {
	//only spill water around if the cell below is occupied

	if (!CanWaterFallDown(index)) {
		//UE_LOG(LogTemp, Warning, TEXT("Cannot fall down"));
		std::vector<int> sideNeighbours;
		sideNeighbours.reserve(4);
		std::vector<int> diagonalNeighbours;
		diagonalNeighbours.reserve(4);
		//UE_LOG(LogTemp, Warning, TEXT("Current index: %d"), index);
		int rightIndex = Grid3d::GetRightNeighborIndex(index, yNCells);

		if (IsNeighbourFreeToBeSpilledTo(index, rightIndex)) {
			sideNeighbours.emplace_back(rightIndex);
		}
		int leftIndex = Grid3d::GetLeftNeighborIndex(index, yNCells);
		if (IsNeighbourFreeToBeSpilledTo(index, leftIndex)) {
			sideNeighbours.emplace_back(leftIndex);
		}
		int frontIndex = Grid3d::GetFrontNeighborIndex(index, xNCells, xyNCells);
		if (IsNeighbourFreeToBeSpilledTo(index, frontIndex)) {
			sideNeighbours.emplace_back(frontIndex);
		}
		int behindIndex = Grid3d::GetBehindNeighborIndex(index, xNCells, xyNCells);
		if (IsNeighbourFreeToBeSpilledTo(index, behindIndex)) {
			sideNeighbours.emplace_back(behindIndex);
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), frontIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
				int frontRightIndex = Grid3d::GetRightNeighborIndex(frontIndex, yNCells);
				if (IsNeighbourFreeToBeSpilledTo(index, frontRightIndex))
					diagonalNeighbours.emplace_back(frontRightIndex);
			}	
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), behindIndex) != sideNeighbours.end()) {
				int rightBehindIndex = Grid3d::GetBehindNeighborIndex(rightIndex, xNCells, xyNCells);
				if (IsNeighbourFreeToBeSpilledTo(index, rightBehindIndex))
					diagonalNeighbours.emplace_back(rightBehindIndex);
			}
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), behindIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), leftIndex) != sideNeighbours.end()) {
				int behindLeftIndex = Grid3d::GetLeftNeighborIndex(behindIndex, yNCells);
				if (IsNeighbourFreeToBeSpilledTo(index, behindLeftIndex))
					diagonalNeighbours.emplace_back(behindLeftIndex);
			}
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), frontIndex) != sideNeighbours.end()) {
				int frontRightIndex = Grid3d::GetFrontNeighborIndex(rightIndex, xNCells, xyNCells);
				if (IsNeighbourFreeToBeSpilledTo(index, frontRightIndex))
					diagonalNeighbours.emplace_back(frontRightIndex);
			}
		}
		/*
		UE_LOG(LogTemp, Warning, TEXT("frontIndex: %d"), frontIndex);
		UE_LOG(LogTemp, Warning, TEXT("behindIndex: %d"), behindIndex);
		UE_LOG(LogTemp, Warning, TEXT("leftIndex: %d"), leftIndex);
		UE_LOG(LogTemp, Warning, TEXT("rightIndex: %d"), rightIndex);
		*/
		// weightSideNeighbour = 1.0f;
		// weightDiagNeighbour = 0.5f;
		// CurrentLevel = (sideNeighbours.size() + 1) * 1.0f * x + diagNeighbours.size() * 0.5f * x
		// CurrentLevel = x ( (sideneighbours.size() + 1) + diagNeighbours.size * 0.5)
		// x = CurrentLevel/ ( (sideneighbours.size() + 1) + diagNeighbours.size * 0.5)
		float diagWeight = 0.5f;

		float diagWaterSum = 0.0f;
		float sideWaterSum = 0.0f;
		for (int& i : sideNeighbours)
			sideWaterSum += GetCurrentWaterLevel(i) + GetWaterSpilt(i);
		for (int& i : diagonalNeighbours)
			diagWaterSum += GetCurrentWaterLevel(i) + GetWaterSpilt(i);
		float sumWater = sideWaterSum + diagWaterSum + GetCurrentWaterLevel(index) + GetWaterSpilt(index);
		const int cells = sideNeighbours.size() + diagonalNeighbours.size() + 1;
		float waterAmountForEach = sumWater / cells;
		//UE_LOG(LogTemp, Warning, TEXT("waterAmountForEachNeighbour: %f"), waterAmountForEachNeighbour);
		for (int& i : sideNeighbours)
			AddWaterSpilt(i, waterAmountForEach - (GetCurrentWaterLevel(i) + GetWaterSpilt(i)));
		for (int& i : diagonalNeighbours)
			AddWaterSpilt(i, waterAmountForEach - (GetCurrentWaterLevel(i) + GetWaterSpilt(i)));
		AddWaterSpilt(index, waterAmountForEach - (GetCurrentWaterLevel(index) + GetWaterSpilt(index)));
	}
}
bool AWorldController::IsNeighbourFreeToBeSpilledTo(const int& currentIndex, const int& neighbourIndex) {
	if (CheckIfCellWIthinBounds(neighbourIndex)) {
		if (CheckIfBlockCell(neighbourIndex))
			return false;
		if (GetWaterCubeIfVisible(neighbourIndex) == nullptr)
			return true;
		if (GetCurrentWaterLevel(neighbourIndex) + GetWaterSpilt(neighbourIndex) < GetCurrentWaterLevel(currentIndex) + GetWaterSpilt(currentIndex))
			return true;
	}
	return false;
}
bool AWorldController::CanWaterFallDown(const int& currentIndex) {
	int bottomIndex = Grid3d::GetBottomNeighborIndex(currentIndex, xyNCells);
	if (CheckIfCellWIthinBounds(bottomIndex)) {
		if (CheckIfBlockCell(bottomIndex))
			return false;
		if (GetWaterCubeIfVisible(bottomIndex) == nullptr)
			return true;
		if (!CheckIfFullCapacityReached(bottomIndex, GetWaterSpilt(bottomIndex) + GetCurrentWaterLevel(bottomIndex)))
			return true;
	}
	return false;
}

bool AWorldController::CanWaterSpillAround(const int& index) {
	int bottomIndex = Grid3d::GetBottomNeighborIndex(index, xyNCells);
	if (!CheckIfCellWIthinBounds(bottomIndex))
		return true;
	if (CheckIfBlockCell(bottomIndex))
		return true;
	if (GetWaterCubeIfVisible(bottomIndex) == nullptr)
		return true;
	if (!CheckIfFullCapacityReached(bottomIndex, GetCurrentWaterLevel(bottomIndex)))
		return true;
	if ( 0.2f > GetWaterSpilt(bottomIndex)  && GetWaterSpilt(bottomIndex) > PRECISION_OFFSET)
		return true;
	return false;

}

void AWorldController::HandleSpiltWater() { //or handle pressure?
	for (int i = 0; i < N_CELLS; i++) {
		if (!CheckIfBlockCell(i)) {
			float summedWaterInCell = GetWaterSpilt(i) + GetCurrentWaterLevel(i);
			if (summedWaterInCell < PRECISION_OFFSET) {
				SetWaterCubeVisibility(i, false);
				SetNextIterationWaterLevel(i, 0.0f);
			}
			else if (summedWaterInCell >= PRECISION_OFFSET) {

				if (GetWaterCubeIfPresent(i) != nullptr)
					SetWaterCubeVisibility(i, true);
				else
				{
					AWaterCube* newCube = GetWorld()->SpawnActor<AWaterCube>((FVector)*GetCellPosition(i), *defaultRotation);
					SetWaterCubeInTheGrid(newCube, i);
				}
				SetNextIterationWaterLevel(i, summedWaterInCell);
			}
		}
		SetWaterSpilt(i, 0.0f);
	}

	ApplyNextIterWaterToCurrent();
}
void AWorldController::ApplyNextIterWaterToCurrent() {
	for (int i = 0; i < N_CELLS; i++) {
		SetWaterLevel(i, grid3d[i].nextIterationWaterLevel);
	}
}

void AWorldController::CalculateWaterCubeCapacity() {
	std::vector<int> indicesOfUnknownCapacity;
	std::vector<int> indicesInZStack;
	indicesInZStack.reserve(zNCells);
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* waterCube = GetWaterCubeIfVisible(i);
		if (waterCube == nullptr)
			continue;

		waterCube->nextIterationWaterCapacity = BASE_CAPACITY;

		indicesInZStack.emplace_back(i);

		bool checkingUpwards = true;
		bool blockAbove = false;
		int topIndex = Grid3d::GetTopNeighborIndex(i, xyNCells);
		while (true) {
			if (!CheckIfCellWIthinBounds(topIndex)) {
				break;
			}

			if (CheckIfBlockCell(topIndex)) {
				blockAbove = true;
				break;
			}

			if (GetWaterCubeIfVisible(topIndex) == nullptr) {
				break;
			}

			indicesInZStack.emplace_back(topIndex);
			waterCube->nextIterationWaterCapacity += EXCEED_MODIFIER;
			float currentLevel = GetCurrentWaterLevel(topIndex) + GetWaterSpilt(topIndex);
			if (!CheckIfFullCapacityReached(topIndex, currentLevel)) {
				break;
			}
			topIndex = Grid3d::GetTopNeighborIndex(topIndex, xyNCells);
		}

		if (blockAbove) {
			for (int& idx : indicesInZStack) {
				grid3d[idx].WaterCube->isCapacityUndetermined = true;
				indicesOfUnknownCapacity.emplace_back(idx);
			}
		}
		else {
			for (int& idx : indicesInZStack)
				grid3d[idx].WaterCube->isCapacityUndetermined = false;
		}
		indicesInZStack.clear();
	}

	std::vector<int> toRemove;
	toRemove.reserve(indicesOfUnknownCapacity.size());

	while (!indicesOfUnknownCapacity.empty()) {
		for (int& idx : indicesOfUnknownCapacity) {

			if (SetByNeighbourWaterCapacityIfPresent(idx)) {
				toRemove.emplace_back(idx);
			}
			//UE_LOG(LogTemp, Warning, TEXT("toRemove.size(): %d"), toRemove.size());
		}
		if (toRemove.empty())
			return;

		while (!toRemove.empty()) {
			int idx = toRemove.back();
			toRemove.pop_back();
			indicesOfUnknownCapacity.erase(std::remove(indicesOfUnknownCapacity.begin(), indicesOfUnknownCapacity.end(), idx), indicesOfUnknownCapacity.end());
		}
		//UE_LOG(LogTemp, Warning, TEXT("indicesOfUnknownCapacity.size(): %d"), indicesOfUnknownCapacity.size());
	}


}

bool AWorldController::SetByNeighbourWaterCapacityIfPresent(const int& index) {
	float highestCapacity = 0.0f;
	bool isWaterAround = false;
	int leftIndex = Grid3d::GetLeftNeighborIndex(index, yNCells);
	GetHigherCapacity(highestCapacity, leftIndex, isWaterAround);
	int frontIndex = Grid3d::GetFrontNeighborIndex(index, xNCells, xyNCells);
	GetHigherCapacity(highestCapacity, frontIndex, isWaterAround);
	int rightIndex = Grid3d::GetRightNeighborIndex(index, yNCells);
	GetHigherCapacity(highestCapacity, rightIndex, isWaterAround);
	int behindIndex = Grid3d::GetBehindNeighborIndex(index, xNCells, xyNCells);
	GetHigherCapacity(highestCapacity, behindIndex, isWaterAround);
	if (highestCapacity > BASE_CAPACITY - PRECISION_OFFSET) {
		grid3d[index].WaterCube->nextIterationWaterCapacity = highestCapacity;
		grid3d[index].WaterCube->isCapacityUndetermined = false;
		return true;
	}
	if (!isWaterAround) {
		grid3d[index].WaterCube->nextIterationWaterCapacity = BASE_CAPACITY;
		grid3d[index].WaterCube->isCapacityUndetermined = false;
		return true;
	}
	return false;
}

void AWorldController::GetHigherCapacity(float& currCapacity, const int& index, bool& isWaterAround) {
	if (!CheckIfCellWIthinBounds(index)) {
		return;
	}

	AWaterCube* waterCube = GetWaterCubeIfVisible(index);
	if (waterCube == nullptr)
		return;
	isWaterAround = true;

	if (waterCube->isCapacityUndetermined)
		return;

	if (waterCube->nextIterationWaterCapacity > currCapacity)
		currCapacity = waterCube->nextIterationWaterCapacity;
}

void AWorldController::ApplyCalculatedCapacities() {
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* waterCube = GetWaterCubeIfVisible(i);
		if (waterCube != nullptr) {
			//UE_LOG(LogTemp, Warning, TEXT("waterCube->nextIterationWaterCapacity[%d]: %f"), i, waterCube->nextIterationWaterCapacity);
			waterCube->currentWaterCapacity = waterCube->nextIterationWaterCapacity;
			waterCube->nextIterationWaterCapacity = BASE_CAPACITY;
		}
	}
}
void AWorldController::ClusterizeWaterGroupsOnLevels() {
	//go through the cells from start to beginning
	//if water && not assigned to any cluster
	//do method TraverseAdjacentWaters(), that has to go like this:
	//1. set its cluster to some in the grid data

	//2. check four neighbours
	//3. if water and not assigned to cluster add them to waterCubesToTraverse and assign to the current cluster
	//4. otherwise do nothing

	//5. while (!needTraverseList.empty) {
	//  pop from needTraverselist
	// do step 3 }
	for (int ZLvl = 0; ZLvl < zNCells; ZLvl++) {

		int currClusterNum = 0;
		for (int idx = ZLvl * xyNCells; idx < xyNCells * (ZLvl + 1); idx++)
		{
			AWaterCube* waterCube = GetWaterCubeIfVisible(idx);
			if (waterCube == nullptr || waterCube->clusterNum > 0)
				continue;
			currClusterNum++;
			waterCube->clusterNum = currClusterNum;
			TraverseAdjacentWaters(currClusterNum, idx);

		}

		SetAllCapacitiesInClusterToHighest(currClusterNum, ZLvl * xyNCells, xyNCells * (ZLvl + 1));
	}

	ResetClusters();

}
void AWorldController::ResetClusters() {
	for (int idx = 0; idx < N_CELLS; idx++)
	{
		AWaterCube* waterCube = GetWaterCubeIfVisible(idx);
		if (waterCube == nullptr)
			continue;
		waterCube->clusterNum = 0;
	}
}
void AWorldController::TraverseAdjacentWaters(const int& currentCluster, const int& startIdx) {

	std::vector<int> waterCubesToTraverse;
	waterCubesToTraverse.reserve(xyNCells);
	waterCubesToTraverse.emplace_back(startIdx);

	while (!waterCubesToTraverse.empty()) {
		int currIdx = waterCubesToTraverse.back();
		waterCubesToTraverse.pop_back();

		//check all 4 neighbours
		int leftIndex = Grid3d::GetLeftNeighborIndex(currIdx, yNCells);
		if (CheckIfCellWIthinBounds(leftIndex)) {
			AWaterCube* waterCube = GetWaterCubeIfVisible(leftIndex);
			if (waterCube != nullptr && waterCube->clusterNum == 0) {
				waterCubesToTraverse.emplace_back(leftIndex);
				waterCube->clusterNum = currentCluster;
			}
		}

		int rightIndex = Grid3d::GetRightNeighborIndex(currIdx, yNCells);
		if (CheckIfCellWIthinBounds(rightIndex)) {
			AWaterCube* waterCube = GetWaterCubeIfVisible(rightIndex);
			if (waterCube != nullptr && waterCube->clusterNum == 0) {
				waterCubesToTraverse.emplace_back(rightIndex);
				waterCube->clusterNum = currentCluster;
			}
		}
		int frontIndex = Grid3d::GetFrontNeighborIndex(currIdx, xNCells, xyNCells);
		if (CheckIfCellWIthinBounds(frontIndex)) {
			AWaterCube* waterCube = GetWaterCubeIfVisible(frontIndex);
			if (waterCube != nullptr && waterCube->clusterNum == 0) {
				waterCubesToTraverse.emplace_back(frontIndex);
				waterCube->clusterNum = currentCluster;
			}
		}

		int behindIndex = Grid3d::GetBehindNeighborIndex(currIdx, xNCells, xyNCells);
		if (CheckIfCellWIthinBounds(behindIndex)) {
			AWaterCube* waterCube = GetWaterCubeIfVisible(behindIndex);
			if (waterCube != nullptr && waterCube->clusterNum == 0) {
				waterCubesToTraverse.emplace_back(behindIndex);
				waterCube->clusterNum = currentCluster;
			}
		}

	}

}
void AWorldController::SetAllCapacitiesInClusterToHighest(const int& nClusters, const int& firstIdx, const int& lastIdx) {
	std::vector<float> capacitiesForClusters;
	capacitiesForClusters.reserve(nClusters + 1);
	for (int i = 0; i < nClusters; i++) {
		capacitiesForClusters.emplace_back(BASE_CAPACITY);
	}

	for (int i = firstIdx; i < lastIdx; i++) {
		AWaterCube* waterCube = GetWaterCubeIfVisible(i);
		if (waterCube == nullptr)
			continue;
		if (capacitiesForClusters[waterCube->clusterNum - 1] < waterCube->currentWaterCapacity)
			capacitiesForClusters[waterCube->clusterNum - 1] = waterCube->currentWaterCapacity;
	}
	for (int i = firstIdx; i < lastIdx; i++) {
		AWaterCube* waterCube = GetWaterCubeIfVisible(i);
		if (waterCube == nullptr)
			continue;

		waterCube->currentWaterCapacity = capacitiesForClusters[waterCube->clusterNum - 1];
	}
	capacitiesForClusters.clear();
}

void AWorldController::DetermineWaterFlow() {
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* waterCube = GetWaterCubeIfVisible(i);
		if (waterCube != nullptr) {
			float currentLevel = GetCurrentWaterLevel(i) + GetWaterSpilt(i);
			if (CheckIfFullCapacityReached(i, currentLevel) && !CanWaterFallDown(i)) {
				EvaluateFlowFromNeighbours(i);
				//FlowAccordingToPressure(i);
			}

		}
	}
}

void AWorldController::FlowPressurizedWaterUpwards() {
/*
   * 1. if water in current cell and exceeded base capacity check the capacity of cell above
   * 2. if cell above is block do nothing
   * 3. if it is empty treat it as if it was of BASE_CAPACITY
   * 4. if water there get its capacity
   * 5. if the water cube above level not reached (current cell's capacity - Exceed Modifier) flow there
   * 6. amount to flow is std::clamp(Current Water Level - Base Capacity, 0.0f, Current Water Level - Base Capacity)
   *
   * */
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* currWaterCube = GetWaterCubeIfVisible(i);
		if (currWaterCube == nullptr)
			continue;
		if (GetCurrentWaterLevel(i) + GetWaterSpilt(i) < BASE_CAPACITY + PRECISION_OFFSET)
			continue;

		int topIndex = Grid3d::GetTopNeighborIndex(i, xyNCells);
		if (!CheckIfCellWIthinBounds(topIndex) || CheckIfBlockCell(topIndex))
			continue;
		//check if cube is pressurized
		float upWaterLevel = GetCurrentWaterLevel(topIndex) + GetWaterSpilt(topIndex);
		float upShouldBeCapacity = std::clamp(currWaterCube->currentWaterCapacity - EXCEED_MODIFIER, BASE_CAPACITY, currWaterCube->currentWaterCapacity - EXCEED_MODIFIER);
		float upFreeAmount = std::clamp(upShouldBeCapacity - upWaterLevel, 0.0f, upShouldBeCapacity - upWaterLevel);
		float waterToFlowUp = std::clamp(GetCurrentWaterLevel(i) + GetWaterSpilt(i) - (float)BASE_CAPACITY, 0.0f, upFreeAmount);
		//UE_LOG(LogTemp, Warning, TEXT("WaterToflowup(%d): %f"), topIndex, waterToFlowUp);

		AddWaterSpilt(topIndex, waterToFlowUp);
		AddWaterSpilt(i, -waterToFlowUp);
	}

}


void AWorldController::FlowAccordingToPressure(const int& index) {

	int leftIndex = Grid3d::GetLeftNeighborIndex(index, yNCells);
	float leftOverload = GetWaterOverloadInCell(leftIndex);
	float leftWaterDiff = GetWaterAmountDiff(index, leftIndex);

	int rightIndex = Grid3d::GetRightNeighborIndex(index, yNCells);
	float rightOverload = GetWaterOverloadInCell(rightIndex);
	float rightWaterDiff = GetWaterAmountDiff(index, rightIndex);

	//double XOverload = rightOverload - leftOverload;

	int frontIndex = Grid3d::GetFrontNeighborIndex(index, xNCells, xyNCells);
	float frontOverload = GetWaterOverloadInCell(frontIndex);
	float frontWaterDiff = GetWaterAmountDiff(index, frontIndex);

	int behindIndex = Grid3d::GetBehindNeighborIndex(index, xNCells, xyNCells);
	float behindOverload = GetWaterOverloadInCell(behindIndex);
	float behindWaterDiff = GetWaterAmountDiff(index, behindIndex);

	std::vector<int> neighboursToEven;
	neighboursToEven.reserve(4);

	if (leftWaterDiff > PRECISION_OFFSET)
		neighboursToEven.emplace_back(leftIndex);
	if (rightWaterDiff > PRECISION_OFFSET)
		neighboursToEven.emplace_back(rightIndex);
	if (frontWaterDiff > PRECISION_OFFSET)
		neighboursToEven.emplace_back(frontIndex);
	if (behindWaterDiff > PRECISION_OFFSET)
		neighboursToEven.emplace_back(behindIndex);

	float summedWater = GetCurrentWaterLevel(index) + GetWaterSpilt(index);
	for(int& idx : neighboursToEven)
	{
		summedWater += GetCurrentWaterLevel(idx) + GetWaterSpilt(idx);
	}

	float waterLvlForEach = summedWater / ((float)(neighboursToEven.size() + 1));
	std::vector<int> amountForIdx;
	amountForIdx.reserve(neighboursToEven.size());
	for (int& idx : neighboursToEven)
	{
		float amountToSpill = waterLvlForEach - (GetCurrentWaterLevel(index) + GetWaterSpilt(index));
		amountForIdx.emplace_back(amountToSpill);
	}
	float amountForCurrIdx = waterLvlForEach - (GetCurrentWaterLevel(index) + GetWaterSpilt(index));

	for (int i = 0; i < amountForIdx.size(); i++) {
		AddWaterSpilt(neighboursToEven[i], amountForIdx[i]);
	}
	AddWaterSpilt(index, amountForCurrIdx);

	neighboursToEven.clear();
	amountForIdx.clear();
	//double YOverload = frontOverload - behindWaterDiff;

	int topIndex = Grid3d::GetTopNeighborIndex(index, xyNCells);
	float topOverload = GetWaterOverloadInCell(topIndex);
	int bottomIndex = Grid3d::GetBottomNeighborIndex(index, xyNCells);
	float bottomOverload = GetWaterOverloadInCell(bottomIndex);

	double ZOverload = topOverload - bottomOverload;
	//CategoryName
	UE_LOG(LogTemp, Warning, TEXT("RLOverload[%d]: %f, %f, FB: %f, %f TB: %f, %f"), index, rightOverload, leftOverload, frontOverload, behindOverload, topOverload, bottomOverload);
	UE_LOG(LogTemp, Warning, TEXT("GetCurrentWaterLevel(%d): %f, GetWaterSpilt() %f, GetWaterCapacity(): %f"), index, GetCurrentWaterLevel(index), GetWaterSpilt(index), GetWaterCapacity(index));

	double overloadedAmount = GetCurrentWaterLevel(index) + GetWaterSpilt(index) - GetWaterCapacity(index);
	if (overloadedAmount < PRECISION_OFFSET)
		return;

	UE_LOG(LogTemp, Warning, TEXT("overloadedAmount[%d]: %f"), index, overloadedAmount);
	double amountToSpread = std::clamp(overloadedAmount, 0.0, MAX_PRESSURED_AMOUNT_ALLOWED_TO_SPREAD);

	double sumOfOverloadedNeighbours = std::fabs(ZOverload);

	if (sumOfOverloadedNeighbours < PRECISION_OFFSET)
		return;

	double amountToSpreadToZ = (ZOverload / sumOfOverloadedNeighbours) * amountToSpread;

	if (ZOverload < 0.0f) {
		AddWaterSpilt(topIndex, amountToSpreadToZ);
	}
	else if (ZOverload > 0.0f) {
		AddWaterSpilt(bottomIndex, amountToSpreadToZ);
	}
	AddWaterSpilt(index, -amountToSpread);
}

void AWorldController::EvaluateFlowFromNeighbours(const int& index) {

	int leftIndex = Grid3d::GetLeftNeighborIndex(index, yNCells);
	float leftOverload = GetWaterOverloadInCell(leftIndex);
	float leftWaterDiff = GetWaterAmountDiff(index, leftIndex);
	int rightIndex = Grid3d::GetRightNeighborIndex(index, yNCells);
	float rightOverload = GetWaterOverloadInCell(rightIndex);
	float rightWaterDiff = GetWaterAmountDiff(index, rightIndex);

	double XOverload = rightOverload - leftOverload;

	int frontIndex = Grid3d::GetFrontNeighborIndex(index, xNCells, xyNCells);
	float frontOverload = GetWaterOverloadInCell(frontIndex);
	float frontWaterDiff = GetWaterAmountDiff(index, frontIndex);

	int behindIndex = Grid3d::GetBehindNeighborIndex(index, xNCells, xyNCells);
	float behindOverload = GetWaterOverloadInCell(behindIndex);
	float behindWaterDiff = GetWaterAmountDiff(index, behindIndex);

	double YOverload = frontOverload - behindWaterDiff;

	int topIndex = Grid3d::GetTopNeighborIndex(index, xyNCells);
	float topOverload = GetWaterOverloadInCell(topIndex);
	int bottomIndex = Grid3d::GetBottomNeighborIndex(index, xyNCells);
	float bottomOverload = GetWaterOverloadInCell(bottomIndex);

	double ZOverload = topOverload - bottomOverload;
	//CategoryName
	UE_LOG(LogTemp, Warning, TEXT("RLOverload[%d]: %f, %f, FB: %f, %f TB: %f, %f"), index, rightOverload, leftOverload, frontOverload, behindOverload, topOverload, bottomOverload);
	UE_LOG(LogTemp, Warning, TEXT("GetCurrentWaterLevel(%d): %f, GetWaterSpilt() %f, GetWaterCapacity(): %f"), index, GetCurrentWaterLevel(index), GetWaterSpilt(index), GetWaterCapacity(index));

	double overloadedAmount = GetCurrentWaterLevel(index) + GetWaterSpilt(index) - GetWaterCapacity(index);
	if (overloadedAmount < PRECISION_OFFSET)
		return;

	UE_LOG(LogTemp, Warning, TEXT("overloadedAmount[%d]: %f"), index, overloadedAmount);
	double amountToSpread = std::clamp(overloadedAmount, 0.0, MAX_PRESSURED_AMOUNT_ALLOWED_TO_SPREAD);

	double sumOfOverloadedNeighbours = std::fabs(XOverload) + std::fabs(YOverload);// + std::fabs(ZOverload);

	if (sumOfOverloadedNeighbours < PRECISION_OFFSET)
		return;

	double amountToSpreadToX = (XOverload / sumOfOverloadedNeighbours) * amountToSpread;
	double amountToSpreadToY = (YOverload / sumOfOverloadedNeighbours) * amountToSpread;
	double amountToSpreadToZ = (ZOverload / sumOfOverloadedNeighbours) * amountToSpread;
	//UE_LOG(LogTemp, Warning, TEXT("amountToSpreadToX: %f, amountToSpreadToY %f, amountToSpreadToZ %f"), amountToSpreadToX, amountToSpreadToY, amountToSpreadToZ);

	if (amountToSpreadToX < 0.0f) {
		AddWaterSpilt(rightIndex, amountToSpreadToX);
	}
	else if (amountToSpreadToX > 0.0f) {
		AddWaterSpilt(leftIndex, amountToSpreadToX);
	}

	if (amountToSpreadToY < 0.0f) {
		AddWaterSpilt(frontIndex, amountToSpreadToY);
	}
	else if (amountToSpreadToY > 0.0f) {
		AddWaterSpilt(behindIndex, amountToSpreadToY);
	}
	
	if (ZOverload < 0.0f) {
		AddWaterSpilt(topIndex, amountToSpreadToZ);
	}
	else if (ZOverload > 0.0f) {
		AddWaterSpilt(bottomIndex, amountToSpreadToZ);
	} 
	AddWaterSpilt(index, -amountToSpread);

}

float AWorldController::GetWaterAmountDiff(const int& index, const int& neighbourIndex) {
	/*
	 if the return value is positive then water should flow to the neighbour
	*/

	if (CheckIfCellWIthinBounds(neighbourIndex)) {
		AWaterCube* waterCube = GetWaterCubeIfVisible(neighbourIndex);
		if (waterCube == nullptr && !CheckIfBlockCell(neighbourIndex)) {
			return 0.0f;
		}
		if (waterCube != nullptr) {
			float neighWaterLevel = GetCurrentWaterLevel(neighbourIndex) + GetWaterSpilt(neighbourIndex);
			float currWaterLevel = GetCurrentWaterLevel(index) + GetWaterSpilt(index);
			return currWaterLevel - neighWaterLevel;
		}
	}
	return 0.0f;
}

float AWorldController::GetWaterOverloadInCell(const int& index) {
	if (CheckIfCellWIthinBounds(index)) {
		AWaterCube* waterCube = GetWaterCubeIfVisible(index);
		if (waterCube == nullptr && !CheckIfBlockCell(index)) {
			return 0.1f;
		}
		if (waterCube != nullptr) {
			float overload = CalculateWaterOverload(index);
			return overload;
		}
	}
	return 1.0f;
}
