// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TreeSeed.generated.h"

UCLASS()
class MYPROJECT_API ATreeSeed : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATreeSeed();

	UPROPERTY(EditAnywhere)
		float TimeOfGrowth;

	UPROPERTY(EditAnywhere)
		float RateOfGrowth;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	TArray<AActor*> branches;
};
