// Fill out your copyright notice in the Description page of Project Settings.

#include "Grid3d.h"

int Grid3d::GetTopNeighborIndex(const int& index, const int& xyNCells) {
	int resultIndex = index + xyNCells;
	return resultIndex;
}

int Grid3d::GetFrontNeighborIndex(const int& index, const int& xNCells, const int& xyNCells) {
	int resultIndex = index + xNCells;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / xyNCells > index / xyNCells)
		return -1;
	return resultIndex;
}
int Grid3d::GetRightNeighborIndex(const int& index, const int& yNCells) {
	int resultIndex = index - 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells < index / yNCells)
		return -1;
	return resultIndex;
}



int Grid3d::GetBehindNeighborIndex(const int& index, const int& xNCells, const int& xyNCells) {
	int resultIndex = index - xNCells;
	if (resultIndex / xyNCells < index / xyNCells)
		return -1;
	return resultIndex;
}

int Grid3d::GetLeftNeighborIndex(const int& index, const int& yNCells) {
	int resultIndex = index + 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells > index / yNCells)
		return -1;
	return resultIndex;
}

int Grid3d::GetBottomNeighborIndex(const int& index, const int& xyNCells) {
	int resultIndex = index - xyNCells;
	return resultIndex;
}
