// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include "Kismet/GameplayStatics.h"
#include "ActorSpawner.h"
#include "WaterCube.h"
#include "BlockCube.h"
// Sets default values

ACameraPawn::ACameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create our components
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	StaticMeshComp = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));

	//Attach our components
	StaticMeshComp->SetupAttachment(RootComponent);
	SpringArmComp->SetupAttachment(StaticMeshComp);
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	//Assign SpringArm class variables.
	SpringArmComp->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-60.0f, 0.0f, 0.0f));
	SpringArmComp->TargetArmLength = 400.f;
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->CameraLagSpeed = 3.0f;
	boostSpeed = 1.0f;
	boostSpeedSpawner = 1.0f;
	changeCameraLock = false;
	spawnerAttached = true;
}

ACameraPawn::~ACameraPawn() {
}

void ACameraPawn::PostInitializeComponents() {
	Super::PostInitializeComponents(); 
}
// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCameraPawnTransform(DeltaTime);
	UpdateSpawnerTransform(DeltaTime);
}


void ACameraPawn::UpdateSpawnerTransform(const float& DeltaTime) {
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller->GetPawn() == Cameras[cameraID]) {
			{
				FRotator NewRotation = controller->GetPawn()->GetActorRotation();
				spawner->SetActorRotation(NewRotation);
			}
			{
				if (!MovementInputSpawner.IsZero())
				{
					//Scale our movement input axis values by 100 units per second
					MovementInputSpawner = MovementInputSpawner.GetSafeNormal() * 200.0f * boostSpeedSpawner;
					FVector NewLocation = spawner->GetActorLocation();
					NewLocation += spawner->GetActorForwardVector() * MovementInputSpawner.X * DeltaTime;
					NewLocation += spawner->GetActorRightVector() * MovementInputSpawner.Y * DeltaTime;
					NewLocation += spawner->GetActorUpVector() * MovementInputSpawner.Z * DeltaTime;
					spawner->SetActorLocation(NewLocation);
				}
			}
		}
}

void ACameraPawn::UpdateCameraPawnTransform(const float& DeltaTime) {
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller->GetPawn() == Cameras[cameraID]) {
		{
			if (bZoomingIn)
			{
				ZoomFactor += DeltaTime / 0.5f;         //Zoom in over half a second
			}
			else
			{
				ZoomFactor -= DeltaTime / 0.25f;        //Zoom out over a quarter of a second
			}
			ZoomFactor = FMath::Clamp<float>(ZoomFactor, 0.0f, 1.0f);

			//Blend our camera's FOV and our SpringArm's length based on ZoomFactor
			CameraComp->FieldOfView = FMath::Lerp<float>(90.0f, 60.0f, ZoomFactor);
			SpringArmComp->TargetArmLength = FMath::Lerp<float>(400.0f, 300.0f, ZoomFactor);
		}
		//Rotate our actor's yaw, which will turn our camera because we're attached to it
		{
			FRotator NewRotation = GetActorRotation();
			NewRotation.Yaw += CameraInput.X;
			SetActorRotation(NewRotation);
		}

		//Rotate our camera's pitch, but limit it so we're always looking downward
		{
			FRotator NewRotation = SpringArmComp->GetComponentRotation();
			NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + CameraInput.Y, -80.0f, -15.0f);
			SpringArmComp->SetWorldRotation(NewRotation);
		}
		{
			if (!MovementInput.IsZero())
			{
				//Scale our movement input axis values by 100 units per second
				MovementInput = MovementInput.GetSafeNormal() * 200.0f * boostSpeed;
				FVector NewLocation = GetActorLocation();
				NewLocation += GetActorForwardVector() * MovementInput.X * DeltaTime;
				NewLocation += GetActorRightVector() * MovementInput.Y * DeltaTime;
				NewLocation += GetActorUpVector() * MovementInput.Z * DeltaTime;
				SetActorLocation(NewLocation);
			}
		}
	}
}
// Called to bind functionality to input
void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//Hook up events for "ZoomIn"
	InputComponent->BindAction("ZoomIn", IE_Pressed, this, &ACameraPawn::ZoomIn);
	InputComponent->BindAction("ZoomIn", IE_Released, this, &ACameraPawn::ZoomOut);

	//Hook up every-frame handling for our four axes
	InputComponent->BindAxis("MoveForwardCamera", this, &ACameraPawn::MoveForwardCamera);
	InputComponent->BindAxis("MoveRightCamera", this, &ACameraPawn::MoveRightCamera);
	InputComponent->BindAxis("MoveUpCamera", this, &ACameraPawn::MoveUpCamera);

	InputComponent->BindAction("Camera1", IE_Pressed, this, &ACameraPawn::ChangeCameraTo1);
  	InputComponent->BindAction("Camera2", IE_Pressed, this, &ACameraPawn::ChangeCameraTo2);
  	InputComponent->BindAction("Camera3", IE_Pressed, this, &ACameraPawn::ChangeCameraTo3);
	InputComponent->BindAxis("BoostSpeed", this, &ACameraPawn::BoostSpeed);
	InputComponent->BindAxis("CameraPitch", this, &ACameraPawn::PitchCamera);
	InputComponent->BindAxis("CameraYaw", this, &ACameraPawn::YawCamera);

	InputComponent->BindAxis("MoveForwardSpawner", this, &ACameraPawn::MoveForwardSpawner);
	InputComponent->BindAxis("MoveRightSpawner", this, &ACameraPawn::MoveRightSpawner);
	InputComponent->BindAxis("MoveUpSpawner", this, &ACameraPawn::MoveUpSpawner);
	InputComponent->BindAction("ToggleSpawnerAttachment", IE_Pressed, this, &ACameraPawn::ToggleSpawnerAttachment);

	InputComponent->BindAxis("SpawnWaterCube", this, &ACameraPawn::SpawnWaterCube);
	InputComponent->BindAxis("SpawnBlockCube", this, &ACameraPawn::SpawnBlockCube);
	InputComponent->BindAction("DestroyActor", IE_Pressed, this, &ACameraPawn::DestroyActor);

}

//Input functions
void ACameraPawn::MoveForwardCamera(float AxisValue) {
	MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
	if (spawnerAttached) {
		MovementInputSpawner.X = MovementInput.X;
	}
}
void ACameraPawn::MoveRightCamera(float AxisValue) {
	MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
	if (spawnerAttached) {
		MovementInputSpawner.Y = MovementInput.Y;
	}
}
void ACameraPawn::MoveUpCamera(float AxisValue) {
	MovementInput.Z = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
	if (spawnerAttached) {
		MovementInputSpawner.Z = MovementInput.Z;
	}
}

void ACameraPawn::ChangeCamera(int camIndex) {

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller->GetPawn() != Cameras[camIndex]) {
		controller->UnPossess();
		controller->Possess(Cameras[camIndex]);
	}
}

void ACameraPawn::ChangeCameraTo1() {
	ChangeCamera(0);
}

void ACameraPawn::ChangeCameraTo2() {
	ChangeCamera(1);
}

void ACameraPawn::ChangeCameraTo3() {
	ChangeCamera(2);
}


void ACameraPawn::BoostSpeed(float AxisValue) {
	boostSpeed = FMath::Clamp<float>(AxisValue, 1.0f, 3.0f);
	boostSpeedSpawner = boostSpeed;
}

void ACameraPawn::PitchCamera(float AxisValue) {
	CameraInput.Y = AxisValue;
}

void ACameraPawn::YawCamera(float AxisValue) {
	CameraInput.X = AxisValue;
}
//Input functions
void ACameraPawn::ZoomIn() {
	bZoomingIn = true;
}

void ACameraPawn::ZoomOut() {
	bZoomingIn = false;
}

void ACameraPawn::MoveForwardSpawner(float AxisValue) {
	if (!spawnerAttached) {
		MovementInputSpawner.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
	}
}
void ACameraPawn::MoveRightSpawner(float AxisValue) {
	if (!spawnerAttached) {
		MovementInputSpawner.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
	}
}
void ACameraPawn::MoveUpSpawner(float AxisValue) {
	if (!spawnerAttached) {
		MovementInputSpawner.Z = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
	}
}
void ACameraPawn::ToggleSpawnerAttachment() {
	spawnerAttached = !spawnerAttached;
}

void ACameraPawn::SpawnWaterCube(float AxisValue) {
	if (AxisValue > 0.9f) {
		AActor* ActorSpawner = UGameplayStatics::GetActorOfClass(spawner, AActorSpawner::StaticClass());

		AActorSpawner* ActorSpawnerReference = Cast<AActorSpawner>(ActorSpawner);
		if (ActorSpawnerReference)
		{
			ActorSpawnerReference->SpawnWaterCube();
		}
	}
}
void ACameraPawn::SpawnBlockCube(float AxisValue) {
	if (AxisValue > 0.9f) {
		AActor* ActorSpawner = UGameplayStatics::GetActorOfClass(spawner, AActorSpawner::StaticClass());
		AActorSpawner* ActorSpawnerReference = Cast<AActorSpawner>(ActorSpawner);
		if (ActorSpawnerReference)
		{
			ActorSpawnerReference->SpawnBlockCube();
		}
	}
}

void ACameraPawn::DestroyActor() {
	//Get every Actor to Spawn in the world and invoke Destroy Actor
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaterCube::StaticClass(), FoundActors);
	for (AActor* ActorFound : FoundActors)
	{
		ActorFound->Destroy();
	}
}