// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldController.h"
#include "WaterCell.h"
#include "Cell.h"

#define BOUND 7
#define N_CELLS (BOUND + BOUND) * (BOUND + BOUND) * (BOUND + BOUND)
#define CELL_SIZE 100.0
#define SIMULATION_SPEED 30.0
#define GRAVITY_SPEED SIMULATION_SPEED

// Sets default values
AWorldController::AWorldController()
{
	PrimaryActorTick.bCanEverTick = true;
	XLeftBound = -BOUND;
	XRightBound = BOUND;
	YLeftBound = -BOUND;
	YRightBound = BOUND;
	ZLeftBound = -BOUND;
	ZRightBound = BOUND;
	grid3d = new Cell[N_CELLS];
	for (int i = 0; i < N_CELLS; i++)
		grid3d[i].CalculatePosition(i, CELL_SIZE, XLeftBound, XRightBound, YLeftBound, YRightBound, ZLeftBound, ZRightBound);

	//UE_LOG(LogTemp, Warning, TEXT("Grid3d %d"), grid3d[7999].waterCell );

}
AWorldController::~AWorldController()
{
	delete[] grid3d;
}

int AWorldController::GetCellIndexAtSnappedPosition(std::unique_ptr<FIntVector> cellPosition) {
	if (!CheckIfInBoundaries(cellPosition->X, cellPosition->Y, cellPosition->Z))
	{
		return -1;
	}
	const std::unique_ptr<FIntVector> localCoordinates = TranslateCellCoordinatesToLocal(std::move(cellPosition));
	return localCoordinates->X + (XRightBound - XLeftBound) * localCoordinates->Y + (XRightBound - XLeftBound) * (YRightBound - YLeftBound) * localCoordinates->Z;
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
	if (GetWaterCellIfPresent(cellIndex) != nullptr)
		return false;
	return true;
}

bool AWorldController::CheckIfCellWIthinBounds(const int& index) {
	return (index > -1 && index < N_CELLS);
}

bool AWorldController::CheckIfBlockCell(const int& index) {
	return grid3d[index].blockCell;
}

AWaterCell* AWorldController::GetWaterCellIfPresent(const int& index) {
	return grid3d[index].waterCell;
}

void AWorldController::SetWaterCubeInTheGrid(AWaterCell* newWaterCell, int cellIndex) {
	grid3d[cellIndex].waterCell = std::move(newWaterCell);
	newWaterCell->SetCurrentGridIndex(cellIndex);
}
void AWorldController::SetBlockCubeInTheGrid(int cellIndex) {
	grid3d[cellIndex].blockCell = true;
}
const UE::Math::TVector<double>* AWorldController::GetCellPosition(const int& index)
{
	return grid3d[index].GetPosition();
}

void AWorldController::RemoveWaterCellFromTheGrid(const int& index) {
	grid3d[index].waterCell = nullptr;
}

int AWorldController::GetTopNeighborIndex(const int& index) {
	int resultIndex = index + (XRightBound - XLeftBound) * (YRightBound - YLeftBound);
	return resultIndex;
}

int AWorldController::GetFrontNeighborIndex(const int& index) {
	int NZCells = (XRightBound - XLeftBound) * (YRightBound - YLeftBound);
	int resultIndex = index + (XRightBound - XLeftBound);
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex % NZCells > index % NZCells)
		return -1;
	return resultIndex;
}

int AWorldController::GetRightNeighborIndex(const int& index) {
	int NXCells = (YRightBound - YLeftBound);
	int resultIndex = index - 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex % NXCells < index % NXCells)
		return -1;
	return resultIndex;
}

int AWorldController::GetBehindNeighborIndex(const int& index) {
	int NZCells = (XRightBound - XLeftBound) * (YRightBound - YLeftBound);
	int resultIndex = index - (XRightBound - XLeftBound);
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex % NZCells < index % NZCells)
		return -1;
	return resultIndex;
}

int AWorldController::GetLeftNeighborIndex(const int& index) {
	int NXCells = (YRightBound - YLeftBound);
	int resultIndex = index + 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex % NXCells > index % NXCells)
		return -1;
	return resultIndex;
}

int AWorldController::GetBottomNeighborIndex(const int& index) {
	int resultIndex = index - (XRightBound - XLeftBound) * (YRightBound - YLeftBound);
	return resultIndex;
}

// Called when the game starts or when spawned
void AWorldController::BeginPlay()
{
	Super::BeginPlay();
	gameTimeElapsed = 0;
	//GetWorldTimerManager().SetTimer(MemberTimerHandle, this, Gravity, 1.0f, true, 2.0f);
}

// Called every frame
void AWorldController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	gameTimeElapsed += DeltaTime;
	if (((int) (gameTimeElapsed * SIMULATION_SPEED)) % 2 == 0)
		Gravity();
}

void AWorldController::Gravity() {
	for (int i = 0; i < N_CELLS; i++) {
		if (GetWaterCellIfPresent(i) != nullptr) {
			int bottomIndex = GetBottomNeighborIndex(i);
			if (CheckIfCellFree(bottomIndex)) {
				MoveTheWaterCell(i, bottomIndex);
			}
		}
	}
}

void AWorldController::MoveTheWaterCell(const int& fromIndex, const int& toIndex) {
	SetWaterCubeInTheGrid(grid3d[fromIndex].waterCell, toIndex);
	RemoveWaterCellFromTheGrid(fromIndex);
	UpdateWaterCellPosition(toIndex);
}

void AWorldController::UpdateWaterCellPosition(const int& index) {

	grid3d[index].waterCell->SetActorLocation(*grid3d[index].GetPosition());
}

