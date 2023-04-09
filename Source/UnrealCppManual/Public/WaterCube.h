
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
	void ChangeColorInTime(const float &delta);
public:	
	AWaterCube();
	~AWaterCube();

	static float minGColor;
	static float maxGColor;
	static float GColorRange;
	static float minBColor;
	static float maxBColor;
	static float BColorRange;

	float capacityRange;
	
	float currentWaterCapacity;
	float nextIterationWaterCapacity;
	bool isCapacityUndetermined;
	int clusterNum;
	void SetCurrentGridIndex(const int& index);
	int GetCurrentGridIndex();
	void SetWaterColorByCapacity();

protected:
	virtual void BeginPlay() override;

};
