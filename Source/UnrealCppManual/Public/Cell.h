// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaterCube.h"

class UNREALCPPMANUAL_API Cell
{
public:
	Cell();
	~Cell();
	//uniqueptr
	AWaterCube* WaterCube;
	bool blockCell;
	bool canWaterBeFlowDownFromHere;
	float nextIterationWaterLevel;
	float currentWaterLevel;
	float waterSpilt;
	int lockedFlowDownCounter;
	void CalculatePosition(const int& index, const int& cell_size, const int& XLeftBound, const int& XRightBound, const int& YLeftBound, const int& YRightBound, const int& ZLeftBound, const int& ZRightBound);
	void AdjustWaterCubesTransformIfPresent(const int& cell_size);
	const UE::Math::TVector<double>* GetPosition();
private:
	const UE::Math::TVector<double>* position;
};
