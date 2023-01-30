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
	int GetCellIndexAtFloatPosition(const FVector& position);
	bool CheckIfCellFree(const int& cellIndex);
	void SetCellInTheGrid(AWaterCell* newWaterCell, int cellIndex);
	const UE::Math::TVector<double>* GetCellPosition(const int& index);
	void RemoveWaterCellFromTheGrid(const int& index);
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

	
	bool CheckIfInBoundaries(const int& x, const int& y, const int& z);
	bool CheckIfCellWIthinBounds(const int& index);
	bool CheckIfBlockCell(const int& index);
	AWaterCell* GetWaterCellIfPresent(const int& index);
	int GetCellIndexAtSnappedPosition(const FIntVector& cellPosition);
	const FIntVector* TranslateCellCoordinatesToLocal(const FIntVector& cellPosition);
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
	void Gravity();
	void UpdateWaterCellPosition(const int& index);


};
