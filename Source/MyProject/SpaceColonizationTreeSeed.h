// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TreeSeed.h"

#include "SpaceColonizationTreeSeed.generated.h"

class AAttractionNode;
class ATreeNode;

UCLASS()
class MYPROJECT_API ASpaceColonizationTreeSeed : public ATreeSeed
{
	GENERATED_BODY()

	ASpaceColonizationTreeSeed();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SeedSceneComponent;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		TSubclassOf<ATreeNode> TreeNodeToSpawn;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		TSubclassOf<AAttractionNode> AttractionNodeToSpawn;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		int NumOfAttractionPoints;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TArray<AAttractionNode*> attractionPoints;
	TArray<ATreeNode*> nodes;

	//Create Attraction Points
	void CreateAttractionPoints();
	void SpawnNewAttractionNode(FVector location);

	//Tree Nodes
	void QueueNewTreeNodes();
	void CreateNewNodes();
	void SpawnNewNode(ATreeNode* parentNode);
	void SpawnNewNode();

	TArray<ATreeNode*> newNodeQueue;

	bool GrowingWithDirection = true;
};
