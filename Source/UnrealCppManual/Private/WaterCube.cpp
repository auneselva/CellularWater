// Fill out your copyright notice in the Description page of Project Settings.


#include "WaterCube.h"
#include <algorithm>
#include <string>
#include <WorldController.h>
#include "Grid3d.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
float AWaterCube::minGColor = 0.05f;
float AWaterCube::maxGColor = 0.2f;
float AWaterCube::GColorRange = AWaterCube::maxGColor - AWaterCube::minGColor;
float AWaterCube::minBColor = 0.1f;
float AWaterCube::maxBColor = 0.85f;
float AWaterCube::BColorRange = AWaterCube::maxBColor - AWaterCube::minBColor;

AWaterCube::AWaterCube() : currentWaterCapacity(BASE_CAPACITY), nextIterationWaterCapacity(BASE_CAPACITY), isCapacityUndetermined(false), clusterNum(0)
{

	PrimaryActorTick.bCanEverTick = false;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(VisualMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_WaterCube.Shape_WaterCube"));

	if (CubeVisualAsset.Succeeded())
	{
		VisualMesh->SetStaticMesh(CubeVisualAsset.Object);
		VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	}

}

AWaterCube::~AWaterCube() {
	delete color;
}

void AWaterCube::SetCurrentGridIndex(const int& index) {
	currentCellIndex = index;
}
int AWaterCube::GetCurrentGridIndex() {
	return currentCellIndex;
}
// Called when the game starts or when spawned
void AWaterCube::BeginPlay()
{
	int worldHeight = Cast<UWaterSimGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->ZCells;
	capacityRange = std::max(0.0f, (float)((worldHeight - 1) * EXCEED_MODIFIER));
	Super::BeginPlay();
	colorRaising = true;

	color = new FLinearColor(0.0f, minGColor, minBColor, 0.0f);
	material = UMaterialInstanceDynamic::Create(VisualMesh->GetMaterial(0), NULL);
	VisualMesh->SetMaterial(0, material);
}

void AWaterCube::SetWaterColorByCapacity() {
	color->R = 0.0f;
	lvlOfCapacity = (currentWaterCapacity - (float)BASE_CAPACITY) / capacityRange;
	color->G = maxGColor - GColorRange * lvlOfCapacity;
	color->B = maxBColor - BColorRange * lvlOfCapacity;

	if (material->IsValidLowLevel()) {
		//UE_LOG(LogTemp, Warning, TEXT("%d The RGBAs value is %f, %f, %f, %f"), GetCurrentGridIndex(),color->R, color->G, color->B, color->A);
		material->SetVectorParameterValue(FName(TEXT("WaterColor")), *color);
	}
}

void AWaterCube::ChangeColorInTime(const float &delta) {
	if (color->R < 0.01f && !colorRaising)
		colorRaising = true;
	else if (color->R > 0.99f && colorRaising)
		colorRaising = false;
	//std::clamp(color->R, 0.0f, 1.0f);
	if (colorRaising)
		color->R += (0.05f * delta);
	else
		color->R -= (0.05f * delta);
	color->G = 0.2f + (color->R / 1.0f) * 0.8f;
	//std::clamp(color.B, 0.0f, 1.0f);
}


