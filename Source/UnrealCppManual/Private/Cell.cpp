// Fill out your copyright notice in the Description page of Project Settings.


#include "Cell.h"
#include "WaterCube.h"
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
	if (WaterCube != nullptr) {
		float level = currentWaterLevel;
		FVector currentScale = WaterCube->GetActorScale();
		WaterCube->SetActorScale3D(FVector(currentScale.X, currentScale.Y, level * 1.0f));
		UE::Math::TVector<double> offset = UE::Math::TVector<double>(0.0f, 0.0f, - ((double) (cell_size)) * (double)level);
		WaterCube->SetActorLocation(*position - offset);
	}
}