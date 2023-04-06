// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GuidingVectorNode.generated.h"

class USphereComponent;
class UCapsuleComponent;

UCLASS()
class MYPROJECT_API AGuidingVectorNode : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGuidingVectorNode();

	UPROPERTY(VisibleAnywhere)
		USceneComponent* SceneComponent;

//	UPROPERTY(VisibleAnywhere)
//		UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere)
		USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere)
		UCapsuleComponent* collider;

	UPROPERTY(EditAnywhere)
		UStaticMesh* coneMesh;
	
	void SetParent(FVector location);
	void SetThisAsConnectionsParent();
	bool GetIsParent();
	bool GetHasParent();
	void SetEndpoint();
	void detectConnections();
	bool GetIsEndpoint();

	void SetPrevious(AGuidingVectorNode* previous);
	AGuidingVectorNode* GetPrevious();
	void SetNext(AGuidingVectorNode* next);
	AGuidingVectorNode* GetNext();

	void CalculateFGScores(FVector endLocation);
	float GetFScore();
	float GetGScore();
	void SetUsed(bool used);
	bool GetUsed();
	FVector GetCurrentDirection();

	float GetGrowingTimer();
	void AddToGrowingTimer(float time);

	TArray<AGuidingVectorNode*> GetConnections();

	void ResetInfo();

	void IncrementChildrenCount();
	int GetNumOfChildren();

	void SetMeshSectionIndex(int index);
	int GetMeshSectionIndex();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FVector directionVector;
	FVector pathDirectionVector;
	AGuidingVectorNode* nextNode;
	AGuidingVectorNode* previousNode;
	UMaterial* endPointMaterial;
	TArray<AGuidingVectorNode*> connections;
	bool isParent = false;
	bool hasParent = false;
	bool isEndpoint = false;
	float fscore = -1;
	float gscore = -1;
	bool isUsed = false;
	float growingTimer = 0;
	int numOfChildren=0;
	int meshSectionIndex = -1;
};
