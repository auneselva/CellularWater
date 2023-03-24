// Fill out your copyright notice in the Description page of Project Settings.


#include "WaterSimGameInstance.h"


UWaterSimGameInstance::UWaterSimGameInstance() {
	SimulationParameters.Init(0, 3);
	XCells = 10;
	YCells = 10;
	ZCells = 10;
}
UWaterSimGameInstance::~UWaterSimGameInstance() {
}