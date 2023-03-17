// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

static class UNREALCPPMANUAL_API Grid3d
{
public:

	static int GetTopNeighborIndex(const int& index, const int& xyNCells);
	static int GetFrontNeighborIndex(const int& index, const int& xNCells, const int& xyNCells);
	static int GetFrontRightNeighborIndex(const int& index, const int& xNCells, const int& yNCells, const int& xyNCells); //by diagonal
	static int GetRightNeighborIndex(const int& index, const int& yNCells);
	static int GetRightBehindNeighborIndex(const int& index, const int& xNCells, const int& yNCells, const int& xyNCells); //by diagonal
	static int GetBehindNeighborIndex(const int& index, const int& xNCells, const int& xyNCells);
	static int GetBehindLeftNeighborIndex(const int& index, const int& xNCells, const int& yNCells, const int& xyNCells); //by diagonal
	static int GetLeftNeighborIndex(const int& index, const int& yNCells);
	static int GetLeftFrontNeighborIndex(const int& index, const int& xNCells, const int& yNCells, const int& xyNCells); //by diagonal
	static int GetBottomNeighborIndex(const int& index, const int& xyNCells);

};

