// Fill out your copyright notice in the Description page of Project Settings.


#include "WaterCube.h"
#include <algorithm>
#include <string>
#include <WorldController.h>
// Sets default values
AWaterCube::AWaterCube(): currentWaterCapacity(BASE_CAPACITY), nextIterationWaterCapacity(BASE_CAPACITY), isCapacityUndetermined(false), flowStrength(0.0f), currDir(PressureDirection::None)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
		//UE_LOG(LogTemp, Warning, TEXT("The RGBs value is %f, %f, %f"), color->R, color->G, color->B);
		material->SetVectorParameterValue(FName(TEXT("WaterColor")), *color);
	}
}

// Called every frame
void AWaterCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//FVector NewLocation = GetActorLocation();
	//FRotator NewRotation = GetActorRotation();
	//float RunningTime = GetGameTimeSinceCreation();
	//float DeltaHeight = (FMath::Sin(RunningTime + DeltaTime) - FMath::Sin(RunningTime));
	//NewLocation.Z += DeltaHeight * 20.0f;       //Scale our height by a factor of 20
	//float DeltaRotation = DeltaTime * 20.0f;    //Rotate by 20 degrees per second
	//NewRotation.Yaw += DeltaRotation;
	//SetActorLocationAndRotation(NewLocation, NewRotation);
	ChangeColorInTime(DeltaTime);

}


