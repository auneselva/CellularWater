// Fill out your copyright notice in the Description page of Project Settings.


#include "Cell.h"
#include "WaterCube.h"
#include <algorithm>
/*
Cell::Cell()
{
	WaterCube = false;
	blockCell = false;
}
*/
Cell::Cell() : WaterCube(nullptr), blockCell(false), nextIterationWaterLevel(0.0f), currentWaterLevel(0.0f), waterSpilt(0.0f)
{
} 


Cell::~Cell()
{
}

void Cell::CalculatePosition(const int &index, const int &cell_size, const int& XLeftBound, const int& XRightBound, const int& YLeftBound, const int& YRightBound, const int& ZLeftBound, const int& ZRightBound) {
	int xSize = (XRightBound - XLeftBound);
	int ySize = (YRightBound - YLeftBound);

	int XY = index % (xSize * ySize);
	int xPart = XY % xSize;
	int yPart = XY / xSize;
	int zPart = (index / (xSize * ySize));

	// translating to the world
	int xPos = (xPart + XLeftBound) * cell_size;
	int yPos = (yPart + YLeftBound) * cell_size;
	int ZPos = (zPart + ZLeftBound) * cell_size;
	position = new UE::Math::TVector<double>(xPos, yPos, ZPos);
}

const UE::Math::TVector<double> * Cell::GetPosition() {
	return position;
}
void Cell::AdjustWaterCubesTransformIfPresent(const int& cell_size) {
	if (WaterCube == nullptr)
		return;
	if (WaterCube->IsHidden())
		return;

	float level = std::clamp(currentWaterLevel, 0.0f, 1.0f);
	//UE_LOG(LogTemp, Warning, TEXT("i: %d, cell_size: %f, Current Water level: %f, offset: %f"), WaterCube->GetCurrentGridIndex(), (double)cell_size, level, (double)cell_size / 2.0 * (level - 1.0));
	WaterCube->SetActorScale3D(FVector(cell_size / 100.0f, cell_size / 100.0f, level * cell_size / 100.0f));
	//UE::Math::TVector<double> offset = UE::Math::TVector<double>(0.0, 0.0, ((double)(cell_size)) / 5.0 *(level - 1.0));

	WaterCube->SetActorLocation(*position);
}