// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CameraPawn.generated.h"

UCLASS()
class UNREALCPPMANUAL_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACameraPawn();
	~ACameraPawn();
	virtual void PostInitializeComponents() override;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMeshComp;
protected:
	UPROPERTY(EditAnywhere)
		class USpringArmComponent* SpringArmComp;

	UPROPERTY(EditAnywhere)
		class UCameraComponent* CameraComp;

	FVector3d MovementInput;
	float boostSpeed;
	FVector3d MovementInputSpawner;
	float boostSpeedSpawner;
	FVector2D CameraInput;
	float ZoomFactor;
	bool bZoomingIn;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
		int cameraID;
	UPROPERTY(EditAnywhere)
		ACameraPawn* Cameras[3];
	UPROPERTY(EditAnywhere)
		AActor* spawner;

	bool changeCameraLock;
	bool spawnerAttached;

	void MoveForwardCamera(float AxisValue);
	void MoveRightCamera(float AxisValue);
	void MoveUpCamera(float AxisValue);
	void ChangeCamera(int camIndex);
	void ChangeCameraTo1();
	void ChangeCameraTo2();
	void ChangeCameraTo3();
	void BoostSpeed(float AxisValue);
	void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);
	void ZoomIn();
	void ZoomOut();

	void MoveForwardSpawner(float AxisValue);
	void MoveRightSpawner(float AxisValue);
	void MoveUpSpawner(float AxisValue);
	void ToggleSpawnerAttachment();
	void SpawnWaterCube();
	void SpawnBlockCube();
	void DestroyActor();

	void UpdateCameraPawnTransform(const float& DeltaTime);
	void UpdateSpawnerTransform(const float& DeltaTime);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
