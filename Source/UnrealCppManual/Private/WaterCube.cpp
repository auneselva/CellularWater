// Fill out your copyright notice in the Description page of Project Settings.


#include "WaterCube.h"
#include <algorithm>
#include <string>
#include <WorldController.h>
// Sets default values
AWaterCube::AWaterCube() : currentWaterCapacity(BASE_CAPACITY), nextIterationWaterCapacity(BASE_CAPACITY), isCapacityUndetermined(false)
{

	PrimaryActorTick.bCanEverTick = true;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	VisualMesh->SetupAttachment(RootComponent);

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
	Super::BeginPlay();
	colorRaising = true;
	color = new FLinearColor(0.0f, 0.2f, 1.0f);
	material = UMaterialInstanceDynamic::Create(VisualMesh->GetMaterial(0), NULL);
	VisualMesh->SetMaterial(0, material);
}

void AWaterCube::SetWaterColorByCapacity() {
	if (colorRaising)
		color->R = (0.5f * currentWaterCapacity);
	else
		color->R = (0.5f * currentWaterCapacity);
	color->G = 0.2f + (color->R / 1.0f) * 0.8f;

	UE_LOG(LogTemp, Warning, TEXT("The RGBAs value is %f, %f, %f, %f"), color->R, color->G, color->B, color->A);
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
	if (material->IsValidLowLevel()) {
		UE_LOG(LogTemp, Warning, TEXT("The RGBAs value is %f, %f, %f, %f"), color->R, color->G, color->B, color->A);
		material->SetVectorParameterValue(FName(TEXT("WaterColor")), *color);
	}
}

void AWaterCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetWaterColorByCapacity();
	//ChangeColorInTime(DeltaTime);

}


