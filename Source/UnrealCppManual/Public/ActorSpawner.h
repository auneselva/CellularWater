#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <UnrealCppManual/Public/CameraPawn.h>
#include "WorldController.h"
#include "WaterSimGameInstance.h"
#include "ActorSpawner.generated.h"

UCLASS()
class UNREALCPPMANUAL_API AActorSpawner: public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AActorSpawner();
    ~AActorSpawner();
    UPROPERTY(EditAnywhere)
        AWorldController* worldController;
    UFUNCTION()
        void SpawnWaterCube();
    UFUNCTION()
        void SpawnBlockCube();
    void SpawnWorldBorder(FVector spawn, UE::Math::TVector<double> scale, FRotator3d rotator);
    void CreateWorldBorders();
    UPROPERTY(EditAnywhere)
        AActor* spawner;
    double* defaultScale;
    FRotator3d* defaultRotation;

protected:

    UWaterSimGameInstance* wsgi;

    virtual void BeginPlay() override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        class UBoxComponent* SpawnVolume;

};