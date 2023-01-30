// Fill out your copyright notice in the Description page of Project Settings.


#include "Cell.h"
#include "WaterCell.h"
/*
Cell::Cell()
{
	waterCell = false;
	blockCell = false;
}
*/
Cell::Cell() : waterCell(nullptr), blockCell(false)
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

Cell::~Cell()
{
}
