// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldController.h"
#include "WaterCube.h"
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
	XLeftBound = -BOUND;
	XRightBound = BOUND;
	YLeftBound = -BOUND;
	YRightBound = BOUND;
	ZLeftBound = -BOUND;
	ZRightBound = BOUND;
	xNCells = XRightBound - XLeftBound;
	yNCells = YRightBound - YLeftBound;
	zNCells = ZRightBound - ZLeftBound;
	xyNCells = (XRightBound - XLeftBound) * (YRightBound - YLeftBound);

	grid3d = new Cell[N_CELLS];
	for (int i = 0; i < N_CELLS; i++) {
		grid3d[i].CalculatePosition(i, CELL_SIZE, XLeftBound, XRightBound, YLeftBound, YRightBound, ZLeftBound, ZRightBound);
		
	}
	//SpawnWorldBorders();
	//UE_LOG(LogTemp, Warning, TEXT("Grid3d %d"), grid3d[7999].WaterCube );

}
void AWorldController::BeginPlay()
{
	Super::BeginPlay();

	gameTimeElapsed = 0;
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
	SpawnWorldBorder(FVector(CELL_SIZE * (XLeftBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YRightBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)xNCells), FRotator3d(90.0f, 0.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)xNCells), FRotator3d(90.0f, 0.0f, 0.0f));


	// vertical
	SpawnWorldBorder(FVector(CELL_SIZE * (XLeftBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)zNCells), FRotator3d(0.0f, 90.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)zNCells), FRotator3d(0.0f, 90.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XLeftBound - 0.5f), CELL_SIZE * (YRightBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)zNCells), FRotator3d(0.0f, 90.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YRightBound - 0.5f), CELL_SIZE * ZLeftBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)zNCells), FRotator3d(0.0f, 90.0f, 0.0f));

	//up
	SpawnWorldBorder(FVector(CELL_SIZE * (XLeftBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZRightBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZRightBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)yNCells), FRotator3d(0.0f, 0.0f, 90.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YRightBound - 0.5f), CELL_SIZE * ZRightBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)xNCells), FRotator3d(90.0f, 0.0f, 0.0f));
	SpawnWorldBorder(FVector(CELL_SIZE * (XRightBound - 0.5f), CELL_SIZE * (YLeftBound - 0.5f), CELL_SIZE * ZRightBound), UE::Math::TVector<double>(0.1f, 0.1f, (float)xNCells), FRotator3d(90.0f, 0.0f, 0.0f));


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

	return std::make_unique<FIntVector>(cellPosition->X - XLeftBound, cellPosition->Y - YLeftBound, cellPosition->Z - ZLeftBound);
}

bool AWorldController::CheckIfInBoundaries(const int& x, const int& y, const int& z) {
	if (x < XLeftBound || x > XRightBound - 1)
		return false;
	else if (y < YLeftBound || y > YRightBound - 1)
		return false;
	else if (z < ZLeftBound || z > ZRightBound - 1)
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
int AWorldController::GetTopNeighborIndex(const int& index) {
	int resultIndex = index + xyNCells;
	return resultIndex;
}

int AWorldController::GetFrontNeighborIndex(const int& index) {
	int resultIndex = index + xNCells;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / xyNCells > index / xyNCells)
		return -1;
	return resultIndex;
}
int AWorldController::GetFrontRightNeighborIndex(const int& index) {
	int frontIndex = GetFrontNeighborIndex(index);
	int resultIndex = GetRightNeighborIndex(frontIndex);
	return resultIndex;
}
int AWorldController::GetRightNeighborIndex(const int& index) {
	int resultIndex = index - 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells < index / yNCells)
		return -1;
	return resultIndex;
}

int AWorldController::GetRightBehindNeighborIndex(const int& index) {
	int rightIndex = GetRightNeighborIndex(index);
	int resultIndex = GetBehindNeighborIndex(rightIndex);
	return resultIndex;
}


int AWorldController::GetBehindNeighborIndex(const int& index) {
	int resultIndex = index - xNCells;
	if (resultIndex / xyNCells < index / xyNCells)
		return -1;
	return resultIndex;
}
int AWorldController::GetBehindLeftNeighborIndex(const int& index) {
	int behindIndex = GetBehindNeighborIndex(index);
	int resultIndex = GetLeftNeighborIndex(behindIndex);
	return resultIndex;
}

int AWorldController::GetLeftNeighborIndex(const int& index) {
	int resultIndex = index + 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells > index / yNCells)
		return -1;
	return resultIndex;
}

int AWorldController::GetLeftFrontNeighborIndex(const int& index) {
	int leftIndex = GetLeftNeighborIndex(index);
	int resultIndex = GetFrontNeighborIndex(leftIndex);
	return resultIndex;
}

int AWorldController::GetBottomNeighborIndex(const int& index) {
	int resultIndex = index - xyNCells;
	return resultIndex;
}

// Called when the game starts or when spawned


void AWorldController::Tick(float DeltaTime) //delta time == around 0.02
{
	Super::Tick(DeltaTime);
	gameTimeElapsed += DeltaTime;
	if (gameTimeElapsed > SIMULATION_SPEED) {
		gameTimeElapsed -= SIMULATION_SPEED;
		ApplySimulationProccesses();
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
		int bottomIndex = GetBottomNeighborIndex(index);
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
	//for (int i = 0; i < N_CELLS; i++)
	//	if (GetWaterCubeIfVisible(i) != nullptr)
	//		UE_LOG(LogTemp, Warning, TEXT("GetCurrentWaterLevel(%d): %f, GetWaterSpilt() %f, GetWaterCapacity(): %f"), i, GetCurrentWaterLevel(i), GetWaterSpilt(i), GetWaterCapacity(i));

	DetermineWaterFlow();
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
		int rightIndex = GetRightNeighborIndex(index);

		if (IsNeighbourFreeToBeSpilledTo(index, rightIndex)) {
			sideNeighbours.emplace_back(rightIndex);
		}
		int leftIndex = GetLeftNeighborIndex(index);
		if (IsNeighbourFreeToBeSpilledTo(index, leftIndex)) {
			sideNeighbours.emplace_back(leftIndex);
		}
		int frontIndex = GetFrontNeighborIndex(index);
		if (IsNeighbourFreeToBeSpilledTo(index, frontIndex)) {
			sideNeighbours.emplace_back(frontIndex);
		}
		int behindIndex = GetBehindNeighborIndex(index);
		if (IsNeighbourFreeToBeSpilledTo(index, behindIndex)) {
			sideNeighbours.emplace_back(behindIndex);
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), frontIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
				int frontRightIndex = GetRightNeighborIndex(frontIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, frontRightIndex))
					diagonalNeighbours.emplace_back(frontRightIndex);
			}	
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), behindIndex) != sideNeighbours.end()) {
				int rightBehindIndex = GetBehindNeighborIndex(rightIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, rightBehindIndex))
					diagonalNeighbours.emplace_back(rightBehindIndex);
			}
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), behindIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), leftIndex) != sideNeighbours.end()) {
				int behindLeftIndex = GetLeftNeighborIndex(behindIndex);
				if (IsNeighbourFreeToBeSpilledTo(index, behindLeftIndex))
					diagonalNeighbours.emplace_back(behindLeftIndex);
			}
		}

		if (std::find(sideNeighbours.begin(), sideNeighbours.end(), rightIndex) != sideNeighbours.end()) {
			if (std::find(sideNeighbours.begin(), sideNeighbours.end(), frontIndex) != sideNeighbours.end()) {
				int frontRightIndex = GetFrontNeighborIndex(rightIndex);
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
	int bottomIndex = GetBottomNeighborIndex(currentIndex);
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
	int bottomIndex = GetBottomNeighborIndex(index);
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
		int topIndex = GetTopNeighborIndex(i);
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
			topIndex = GetTopNeighborIndex(topIndex);
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
	// bug
	UE_LOG(LogTemp, Warning, TEXT("indicesOfUnknownCapacity.size(): %d"), indicesOfUnknownCapacity.size());
	// bug
	while (!indicesOfUnknownCapacity.empty()) {
		for (int& idx : indicesOfUnknownCapacity) {

			if (SetByNeighbourWaterCapacityIfPresent(idx)) {
				toRemove.emplace_back(idx);
			}
			UE_LOG(LogTemp, Warning, TEXT("toRemove.size(): %d"), toRemove.size());
		}
		if (toRemove.empty())
			return;

		while (!toRemove.empty()) {
			int idx = toRemove.back();
			toRemove.pop_back();
			indicesOfUnknownCapacity.erase(std::remove(indicesOfUnknownCapacity.begin(), indicesOfUnknownCapacity.end(), idx), indicesOfUnknownCapacity.end());
		}
		UE_LOG(LogTemp, Warning, TEXT("indicesOfUnknownCapacity.size(): %d"), indicesOfUnknownCapacity.size());
	}


}

bool AWorldController::SetByNeighbourWaterCapacityIfPresent(const int& index) {
	float highestCapacity = 0.0f;
	bool isWaterAround = false;
	int leftIndex = GetLeftNeighborIndex(index);
	GetHigherCapacity(highestCapacity, leftIndex, isWaterAround);
	int frontIndex = GetFrontNeighborIndex(index);
	GetHigherCapacity(highestCapacity, frontIndex, isWaterAround);
	int rightIndex = GetRightNeighborIndex(index);
	GetHigherCapacity(highestCapacity, rightIndex, isWaterAround);
	int behindIndex = GetBehindNeighborIndex(index);
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

void AWorldController::DetermineWaterFlow() {
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* waterCube = GetWaterCubeIfVisible(i);
		if (waterCube != nullptr) {
			float currentLevel = GetCurrentWaterLevel(i) + GetWaterSpilt(i);
			if (CheckIfFullCapacityReached(i, currentLevel) && !CanWaterFallDown(i)) {
				EvaluateFlowFromNeighbours(i);
			}

		}
	}
}

void AWorldController::EvaluateFlowFromNeighbours(const int& index) {

	int leftIndex = GetLeftNeighborIndex(index);
	float leftOverload = GetWaterOverloadInCell(leftIndex);
	int rightIndex = GetRightNeighborIndex(index);
	float rightOverload = GetWaterOverloadInCell(rightIndex);

	double XOverload = rightOverload - leftOverload;

	int frontIndex = GetFrontNeighborIndex(index);
	float frontOverload = GetWaterOverloadInCell(frontIndex);
	int behindIndex = GetBehindNeighborIndex(index);
	float behindOverload = GetWaterOverloadInCell(behindIndex);

	double YOverload = frontOverload - behindOverload;

	int topIndex = GetTopNeighborIndex(index);
	float topOverload = GetWaterOverloadInCell(topIndex);
	int bottomIndex = GetBottomNeighborIndex(index);
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

	double sumOfOverloadedNeighbours = std::fabs(XOverload) + std::fabs(YOverload) + std::fabs(ZOverload);

	if (sumOfOverloadedNeighbours < PRECISION_OFFSET)
		return;

	double amountToSpreadToX = (XOverload / sumOfOverloadedNeighbours) * amountToSpread;
	double amountToSpreadToY = (YOverload / sumOfOverloadedNeighbours) * amountToSpread;
	double amountToSpreadToZ = (ZOverload / sumOfOverloadedNeighbours) * amountToSpread;
	UE_LOG(LogTemp, Warning, TEXT("amountToSpreadToX: %f, amountToSpreadToY %f, amountToSpreadToZ %f"), amountToSpreadToX, amountToSpreadToY, amountToSpreadToZ);

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
