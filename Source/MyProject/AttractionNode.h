// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "AttractionNode.generated.h"

UCLASS()
class MYPROJECT_API AAttractionNode : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAttractionNode();

	UPROPERTY(VisibleAnywhere)
		USceneComponent* SceneComponent;

//	UPROPERTY(VisibleAnywhere)
//		UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere)
		UCapsuleComponent* collider;
};
