// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "TreeNode.generated.h"

class USphereComponent;
UCLASS()
class MYPROJECT_API ATreeNode : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATreeNode();

	UPROPERTY(VisibleAnywhere)
		USceneComponent* SceneComponent;

//	UPROPERTY(EditAnywhere)
//		UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere)
		USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere)
		USphereComponent* KillRange;

	UPROPERTY(VisibleAnywhere)
		UCapsuleComponent* collider;

	UFUNCTION()
		void OnKillOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,	bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere)
		float TimeUntilDeath;

	UPROPERTY(EditAnywhere)
		float MaxDensity;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector* GetNextTreeNodePosition();
	void ResetNextTreeNodePosition();

	bool GetIsActive();

	void CalculateCurrentDirection(FVector parentNodeLocation);
	FVector GetCurrentDirection();

	void CalculateNextTreeNodePosition(bool useDirection, float branchLength);

	bool HasAttractionInfluences();

	int numOfChildren = 0;

	ATreeNode* parent=nullptr;

	void IncrementChildCount();

	float growingTimer = 0;

	void SetMeshSectionIndex(int index);
	int GetMeshSectionIndex();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TArray<AActor*> attractionInfluences;
	TArray<AActor*> detractionInfluences;
	FVector currentDirection;
	FVector* nextTreeNodePosition;
	float nodebranchLength;
	int meshSectionIndex=-1;
};
