// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterCube.generated.h"

UCLASS()
class UNREALCPPMANUAL_API AWaterCube : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* VisualMesh;
private:
	UPROPERTY(VisibleAnywhere)
		int currentCellIndex;
	UMaterialInstanceDynamic* material;
	FLinearColor* color;
	bool colorRaising;

	float lvlOfCapacity;
	void SetWaterColorByCapacity();
	void ChangeColorInTime(const float &delta);
public:	
	// Sets default values for this actor's properties
	AWaterCube();
	~AWaterCube();

	static float minGColor;
	static float maxGColor;
	static float GColorRange;
	static float minBColor;
	static float maxBColor;
	static float BColorRange;

	static int worldHeight;
	static float capacityRange;
	
	float currentWaterCapacity;
	float nextIterationWaterCapacity;
	bool isCapacityUndetermined;
	void SetCurrentGridIndex(const int& index);
	int GetCurrentGridIndex();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
