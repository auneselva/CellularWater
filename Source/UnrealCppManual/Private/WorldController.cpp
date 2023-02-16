// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldController.h"
#include "WaterCube.h"
#include "Cell.h"
#include <vector>
#include <algorithm>
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

AWorldController::AWorldController()
{
	PrimaryActorTick.bCanEverTick = true;
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
	for (int i = 0; i < N_CELLS; i++)
		grid3d[i].CalculatePosition(i, CELL_SIZE, XLeftBound, XRightBound, YLeftBound, YRightBound, ZLeftBound, ZRightBound);
	defaultRotation = new FRotator3d();
	//UE_LOG(LogTemp, Warning, TEXT("Grid3d %d"), grid3d[7999].WaterCube );

}
AWorldController::~AWorldController()
{
	delete[] grid3d;
	delete defaultRotation;
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
	if (GetWaterCubeIfPresent(cellIndex) != nullptr)
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
	return grid3d[index].WaterCube;
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
	return GetCurrentWaterLevel(index) / GetWaterCapacity(index);
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

void AWorldController::DestroyWaterCubeActor(const int& index) {
	if (grid3d[index].WaterCube != nullptr)
		GetWorld()->DestroyActor(grid3d[index].WaterCube);
	DetachWaterCubeFromTheCell(index);
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

int AWorldController::GetRightNeighborIndex(const int& index) {
	int resultIndex = index - 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells < index / yNCells)
		return -1;
	return resultIndex;
}

int AWorldController::GetBehindNeighborIndex(const int& index) {
	int resultIndex = index - xNCells;
	if (resultIndex / xyNCells < index / xyNCells)
		return -1;
	return resultIndex;
}

int AWorldController::GetLeftNeighborIndex(const int& index) {
	int resultIndex = index + 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells > index / yNCells)
		return -1;
	return resultIndex;
}

int AWorldController::GetBottomNeighborIndex(const int& index) {
	int resultIndex = index - xyNCells;
	return resultIndex;
}

// Called when the game starts or when spawned
void AWorldController::BeginPlay()
{
	Super::BeginPlay();

	gameTimeElapsed = 0;
	//GetWorldTimerManager().SetTimer(MemberTimerHandle, this, Gravity, 1.0f, true, 2.0f);
}

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
		if (GetWaterCubeIfPresent(bottomIndex) != nullptr) {
			float waterAmountToBeFlown = GetWaterCapacity(bottomIndex) - GetCurrentWaterLevel(bottomIndex);
			
			AddWaterSpilt(bottomIndex, waterAmountToBeFlown);
			AddWaterSpilt(index, -waterAmountToBeFlown);
		}
		else {
			AddWaterSpilt(bottomIndex, GetCurrentWaterLevel(index));
			AddWaterSpilt(index, -GetCurrentWaterLevel(index));
		}
	}
}

void AWorldController::ApplySimulationProccesses() {
	for (int i = 0; i < N_CELLS; i++) {
		if (GetWaterCubeIfPresent(i) != nullptr) {
			Gravity(i);
			SpillAround(i);
		}
		if (i % xyNCells == xyNCells - 1)
			HandleSpiltWater(i - xyNCells + 1, i + 1);
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
	//CalculateWaterCubeCapacity();
	//ApplyCalculatedCapacities();
	//DetermineWaterFlow();
	HandleSpiltWater(0, N_CELLS);
	for (int i = 0; i < N_CELLS; i++)
		grid3d[i].AdjustWaterCubesTransformIfPresent(CELL_SIZE);
}

void AWorldController::SpillAround(const int& index) {
	//only spill water around if the cell below is occupied

	if (!CanWaterFallDown(index)) {
		//UE_LOG(LogTemp, Warning, TEXT("Cannot fall down"));
		std::vector<int> sideNeighbours;
		sideNeighbours.reserve(4);
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
		/*
		UE_LOG(LogTemp, Warning, TEXT("frontIndex: %d"), frontIndex);
		UE_LOG(LogTemp, Warning, TEXT("behindIndex: %d"), behindIndex);
		UE_LOG(LogTemp, Warning, TEXT("leftIndex: %d"), leftIndex);
		UE_LOG(LogTemp, Warning, TEXT("rightIndex: %d"), rightIndex);
		*/
		float waterAmountForEachNeighbour = GetCurrentWaterLevel(index) / ((float)(sideNeighbours.size()) + 1.0f);
		//UE_LOG(LogTemp, Warning, TEXT("waterAmountForEachNeighbour: %f"), waterAmountForEachNeighbour);
		for (int& i : sideNeighbours)
			AddWaterSpilt(i, waterAmountForEachNeighbour);
		AddWaterSpilt(index, -waterAmountForEachNeighbour * (float)(sideNeighbours.size()));
	}
}
bool AWorldController::IsNeighbourFreeToBeSpilledTo(const int& currentIndex, const int& neighbourIndex) {
	if (CheckIfCellWIthinBounds(neighbourIndex)) {
		if (CheckIfBlockCell(neighbourIndex))
			return false;
		if (GetWaterCubeIfPresent(neighbourIndex) == nullptr)
			return true;
		if (GetCurrentWaterLevel(neighbourIndex) < GetCurrentWaterLevel(currentIndex))
			return true;
	}
	return false;
}
bool AWorldController::CanWaterFallDown(const int& currentIndex) {
	int bottomIndex = GetBottomNeighborIndex(currentIndex);
	if (CheckIfCellWIthinBounds(bottomIndex)) {
		if (CheckIfBlockCell(bottomIndex))
			return false;
		if (GetWaterCubeIfPresent(bottomIndex) == nullptr)
			return true;
		if (!CheckIfFullCapacityReached(bottomIndex, GetCurrentWaterLevel(bottomIndex)))
			return true;
	}
	return false;
}
void AWorldController::HandleSpiltWater(int start, int end) { //or handle pressure?
	for (int i = 0; i < N_CELLS; i++) {
		if (!CheckIfBlockCell(i)) {
			float summedWaterInCell = GetWaterSpilt(i) + GetCurrentWaterLevel(i);
			if (summedWaterInCell < PRECISION_OFFSET) {
				DestroyWaterCubeActor(i);
				SetNextIterationWaterLevel(i, 0.0f);
			}
			else if (summedWaterInCell >= PRECISION_OFFSET) {
				if (GetWaterCubeIfPresent(i) == nullptr) {
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
		AWaterCube* waterCube = grid3d[i].WaterCube;
		if (waterCube == nullptr)
			continue;
		waterCube->nextIterationWaterCapacity = BASE_CAPACITY;

		indicesInZStack.emplace_back(i);

		bool checkingUpwards = true;
		bool blockAbove = false;
		int topIndex;

		while (true) {
			topIndex = GetTopNeighborIndex(i);
			if (!CheckIfCellWIthinBounds(topIndex)) {
				break;
			}

			if (CheckIfBlockCell(topIndex)) {
				blockAbove = true;
				break;
			}

			if (GetWaterCubeIfPresent(topIndex) == nullptr) {
				break;
			}

			indicesInZStack.emplace_back(topIndex);
			waterCube->nextIterationWaterCapacity += EXCEED_MODIFIER;
			float currentLevel = GetCurrentWaterLevel(topIndex);
			if (!CheckIfFullCapacityReached(topIndex, currentLevel)) {
				break;
			}

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

			if (SetNeighbourWaterCapacityIfPresent(idx))
				toRemove.emplace_back(idx);
		}
		if (toRemove.empty())
			return;

		while (!toRemove.empty()) {
			int idx = toRemove.back();
			toRemove.pop_back();
#pragma warning(suppress : 4834)
			std::remove(indicesOfUnknownCapacity.begin(), indicesOfUnknownCapacity.end(), idx);
		}
	}


}

bool AWorldController::SetNeighbourWaterCapacityIfPresent(const int& index) {
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

	AWaterCube* waterCube = GetWaterCubeIfPresent(index);
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
		AWaterCube* waterCube = grid3d[i].WaterCube;
		if (waterCube != nullptr) {
			waterCube->currentWaterCapacity = waterCube->nextIterationWaterCapacity;
			waterCube->nextIterationWaterCapacity = BASE_CAPACITY;
		}
	}
}

void AWorldController::DetermineWaterFlow() {
	for (int i = 0; i < N_CELLS; i++) {
		AWaterCube* waterCube = GetWaterCubeIfPresent(i);
		if (waterCube != nullptr) {
			float currentLevel = GetCurrentWaterLevel(i);
			int bottomIndex = GetBottomNeighborIndex(i);
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

	float XOverload = rightOverload - leftOverload;

	int frontIndex = GetFrontNeighborIndex(index);
	float frontOverload = GetWaterOverloadInCell(frontIndex);
	int behindIndex = GetBehindNeighborIndex(index);
	float behindOverload = GetWaterOverloadInCell(behindIndex);

	float YOverload = frontOverload - behindOverload;

	int topIndex = GetTopNeighborIndex(index);
	float topOverload = GetWaterOverloadInCell(topIndex);
	int bottomIndex = GetBottomNeighborIndex(index);
	float bottomOverload = GetWaterOverloadInCell(bottomIndex);

	float ZOverload = topOverload - bottomOverload;

	double overloadedAmount = GetCurrentWaterLevel(index) - BASE_CAPACITY;
	float amountToSpread = std::clamp(overloadedAmount, 0.0, MAX_PRESSURED_AMOUNT_ALLOWED_TO_SPREAD);

	float sumOfOverloadedNeighbours = std::fabs(XOverload) + std::fabs(YOverload) + std::fabs(ZOverload);

	float amountToSpreadToX = XOverload / sumOfOverloadedNeighbours;
	float amountToSpreadToY = YOverload / sumOfOverloadedNeighbours;
	float amountToSpreadToZ = ZOverload / sumOfOverloadedNeighbours;

	if (amountToSpreadToX < 0.0f - PRECISION_OFFSET) {
		AddWaterSpilt(rightIndex, amountToSpreadToX);
	}
	else if (amountToSpreadToX > 0.0f + PRECISION_OFFSET) {
		AddWaterSpilt(leftIndex, amountToSpreadToX);
	}

	if (amountToSpreadToY < 0.0f - PRECISION_OFFSET) {
		AddWaterSpilt(frontIndex, amountToSpreadToY);
	}
	else if (amountToSpreadToY > 0.0f + PRECISION_OFFSET) {
		AddWaterSpilt(behindIndex, amountToSpreadToY);
	}

	if (ZOverload < 0.0f - PRECISION_OFFSET) {
		AddWaterSpilt(topIndex, amountToSpreadToZ);
	}
	else if (ZOverload > 0.0f + PRECISION_OFFSET) {
		AddWaterSpilt(bottomIndex, amountToSpreadToZ);
	}
	AddWaterSpilt(index, -amountToSpread);

}

float AWorldController::GetWaterOverloadInCell(const int& index) {
	if (CheckIfCellWIthinBounds(index)) {
		AWaterCube* waterCube = grid3d[index].WaterCube;
		if (waterCube == nullptr && !CheckIfBlockCell(index)) {
			return 10.0f;
		}
		if (waterCube != nullptr) {
			float overload = CalculateWaterOverload(index);
			return overload;
		}
	}
	return 1.0f;
}
