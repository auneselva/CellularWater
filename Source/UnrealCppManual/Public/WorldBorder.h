// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldBorder.generated.h"

UCLASS()
class UNREALCPPMANUAL_API AWorldBorder : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* VisualMesh;
public:	
	// Sets default values for this actor's properties
	AWorldBorder();

};
