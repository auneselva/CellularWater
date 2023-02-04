#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <UnrealCppManual/Public/CameraPawn.h>
#include "WorldController.h"
#include "ActorSpawner.generated.h"

UCLASS()
class UNREALCPPMANUAL_API AActorSpawner: public AActor
{
    GENERATED_BODY()
    
public:
    // Sets default values for this actor's properties
    AActorSpawner();
    UPROPERTY(EditAnywhere)
        AWorldController* worldController;
    UFUNCTION()
        void SpawnActor();
    FVector3d MovementInput;
    float boostSpeed;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        class UBoxComponent* SpawnVolume;
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
};