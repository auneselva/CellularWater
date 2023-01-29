// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include "Kismet/GameplayStatics.h"
#include "ActorSpawner.h"
#include "WaterCell.h"

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
	changeCameraLock = false;
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
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller->GetPawn() == Cameras[cameraID]) {
		Super::Tick(DeltaTime);
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
	InputComponent->BindAxis("MoveForward", this, &ACameraPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACameraPawn::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &ACameraPawn::MoveUp);
	InputComponent->BindAction("Camera1", IE_Pressed, this, &ACameraPawn::ChangeCameraTo1);
  	InputComponent->BindAction("Camera2", IE_Pressed, this, &ACameraPawn::ChangeCameraTo2);
  	InputComponent->BindAction("Camera3", IE_Pressed, this, &ACameraPawn::ChangeCameraTo3);
	InputComponent->BindAxis("BoostSpeed", this, &ACameraPawn::BoostSpeed);
	InputComponent->BindAxis("CameraPitch", this, &ACameraPawn::PitchCamera);
	InputComponent->BindAxis("CameraYaw", this, &ACameraPawn::YawCamera);

	InputComponent->BindAction("SpawnActor", IE_Pressed, this, &ACameraPawn::SpawnActor);
	InputComponent->BindAction("DestroyActor", IE_Pressed, this, &ACameraPawn::DestroyActor);

}

//Input functions
void ACameraPawn::MoveForward(float AxisValue)
{
	MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void ACameraPawn::MoveRight(float AxisValue)
{
	MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}
void ACameraPawn::MoveUp(float AxisValue)
{
	MovementInput.Z = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void ACameraPawn::ChangeCamera(int camIndex) {

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller->GetPawn() != Cameras[camIndex]) {
		UE_LOG(LogTemp, Warning, TEXT("Camera should be changed now to %d"), cameraID)
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
}

void ACameraPawn::PitchCamera(float AxisValue)
{
	CameraInput.Y = AxisValue;
}

void ACameraPawn::YawCamera(float AxisValue)
{
	CameraInput.X = AxisValue;
}

void ACameraPawn::ZoomIn()
{
	bZoomingIn = true;
}

void ACameraPawn::ZoomOut()
{
	bZoomingIn = false;
}
void ACameraPawn::SpawnActor() {
	//Find the Actor Spawner in the world, and invoke it's Spawn Actor function
	AActor* ActorSpawnerTofind = UGameplayStatics::GetActorOfClass(GetWorld(), AActorSpawner::StaticClass());

	AActorSpawner* ActorSpawnerReference = Cast<AActorSpawner>(ActorSpawnerTofind);
	if (ActorSpawnerReference)
	{
		ActorSpawnerReference->SpawnActor();
	}
}
void ACameraPawn::DestroyActor() {
	//Get every Actor to Spawn in the world and invoke Destroy Actor
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaterCell::StaticClass(), FoundActors);
	for (AActor* ActorFound : FoundActors)
	{
		ActorFound->Destroy();
	}
}