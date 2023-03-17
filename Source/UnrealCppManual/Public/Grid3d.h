// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UNREALCPPMANUAL_API Grid3d
{
public:

	static int GetTopNeighborIndex(const int& index, const int& xyNCells);
	static int GetFrontNeighborIndex(const int& index, const int& xNCells, const int& xyNCells);
	static int GetRightNeighborIndex(const int& index, const int& yNCells);
	static int GetBehindNeighborIndex(const int& index, const int& xNCells, const int& xyNCells);
	static int GetLeftNeighborIndex(const int& index, const int& yNCells);
	static int GetBottomNeighborIndex(const int& index, const int& xyNCells);

};

