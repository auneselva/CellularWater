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
int Grid3d::GetFrontRightNeighborIndex(const int& index, const int& xNCells, const int& yNCells, const int& xyNCells) {
	int frontIndex = GetFrontNeighborIndex(index, xNCells, xyNCells);
	int resultIndex = GetRightNeighborIndex(frontIndex, yNCells);
	return resultIndex;
}
int Grid3d::GetRightNeighborIndex(const int& index, const int& yNCells) {
	int resultIndex = index - 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells < index / yNCells)
		return -1;
	return resultIndex;
}

int Grid3d::GetRightBehindNeighborIndex(const int& index, const int& xNCells, const int& yNCells, const int& xyNCells) {
	int rightIndex = GetRightNeighborIndex(index, yNCells);
	int resultIndex = GetBehindNeighborIndex(rightIndex, xNCells, xyNCells);
	return resultIndex;
}


int Grid3d::GetBehindNeighborIndex(const int& index, const int& xNCells, const int& xyNCells) {
	int resultIndex = index - xNCells;
	if (resultIndex / xyNCells < index / xyNCells)
		return -1;
	return resultIndex;
}
int Grid3d::GetBehindLeftNeighborIndex(const int& index, const int& xNCells, const int& yNCells, const int& xyNCells) {
	int behindIndex = GetBehindNeighborIndex(index, xNCells, xyNCells);
	int resultIndex = GetLeftNeighborIndex(behindIndex, yNCells);
	return resultIndex;
}

int Grid3d::GetLeftNeighborIndex(const int& index, const int& yNCells) {
	int resultIndex = index + 1;
	// if outside bounds return -1 to let us know it is a blocking cell
	if (resultIndex / yNCells > index / yNCells)
		return -1;
	return resultIndex;
}

int Grid3d::GetLeftFrontNeighborIndex(const int& index, const int& xNCells, const int& yNCells, const int& xyNCells) {
	int leftIndex = GetLeftNeighborIndex(index, yNCells);
	int resultIndex = GetFrontNeighborIndex(leftIndex, xNCells, xyNCells);
	return resultIndex;
}

int Grid3d::GetBottomNeighborIndex(const int& index, const int& xyNCells) {
	int resultIndex = index - xyNCells;
	return resultIndex;
}
