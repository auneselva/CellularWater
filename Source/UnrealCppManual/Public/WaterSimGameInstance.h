// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaterSimGameInstance.generated.h"
/**
 * 
 */
UCLASS(config = Game, transient, BlueprintType)
class UNREALCPPMANUAL_API UWaterSimGameInstance: public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int XCells;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int YCells;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int ZCells;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int> SimulationParameters;
	UWaterSimGameInstance();
	~UWaterSimGameInstance();

};
