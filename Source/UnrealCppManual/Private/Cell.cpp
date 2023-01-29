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

void Cell::CalculatePosition(const int &index, const int &cell_size, const int& XLeftBound, const int& XRightBound, const int& YLeftBound, const int& YRightBound) {
	int XY = index % ((XRightBound - XLeftBound) * (YRightBound - YLeftBound));
	int xIndex = XY % (XRightBound - XLeftBound);
	int xCoord = xIndex * cell_size;
	int yPart = XY / (XRightBound - XLeftBound);
	int yCoord = yPart * cell_size;
	int ZCoord = (index / ((XRightBound - XLeftBound) * (YRightBound - YLeftBound))) * cell_size;
	position = new UE::Math::TVector<double> (xCoord, yCoord, ZCoord);
}

const UE::Math::TVector<double> * Cell::GetPosition() {
	return position;
}

Cell::~Cell()
{
}
