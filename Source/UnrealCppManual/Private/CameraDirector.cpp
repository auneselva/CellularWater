// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraDirector.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
ACameraDirector::ACameraDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACameraDirector::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACameraDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    const float TimeBetweenCameraChanges = 2.0f;
    const float SmoothBlendTime = 0.75f;
    TimeToNextCameraChange -= DeltaTime;
    if (TimeToNextCameraChange <= 0.0f)
    {
        TimeToNextCameraChange += TimeBetweenCameraChanges;

        // Find the actor that handles control for the local player.
        APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
        if (OurPlayerController)
        {
            AActor* viewTarget = OurPlayerController->GetViewTarget();
            if ((viewTarget != Cameras[0] && viewTarget != Cameras[1]) && (Cameras[0] != nullptr))
            {
                // Cut instantly to camera one.
                OurPlayerController->SetViewTargetWithBlend(Cameras[0]);
            }
            else if ((OurPlayerController->GetViewTarget() == Cameras[0]) && (Cameras[1] != nullptr))
            {
                // Blend smoothly to camera two.
                OurPlayerController->SetViewTargetWithBlend(Cameras[1], SmoothBlendTime);
            }
            else if ((OurPlayerController->GetViewTarget() == Cameras[1]) && (Cameras[2] != nullptr))
            {
                // Blend smoothly to camera two.
                OurPlayerController->SetViewTargetWithBlend(Cameras[2], SmoothBlendTime);
            }
        }
    }

}

