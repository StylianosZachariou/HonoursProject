// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TreeSeed.h"
#include "ProceduralMeshComponent.h"
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

	UPROPERTY(Category = "Mesh", VisibleAnywhere)
		UProceduralMeshComponent* MeshComponent;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		UMaterialInstance* Material;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		TSubclassOf<ATreeNode> TreeNodeToSpawn;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		TSubclassOf<AAttractionNode> AttractionNodeToSpawn;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		int NumOfAttractionPoints;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		float MeshGrowthRate=3.52;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TArray<AAttractionNode*> attractionPoints;
	TArray<ATreeNode*> nodes;

	void ApplyEnvironment();

	//Create Attraction Points
	void CreateAttractionPoints();
	void SpawnNewAttractionNode(FVector location);

	//Tree Nodes
	void QueueNewTreeNodes();
	void CreateNewNodes();
	void SpawnNewNode(ATreeNode* parentNode);
	void SpawnNewNode();

	//Mesh Stuff
	void CreateMesh();

	TArray<ATreeNode*> newNodeQueue;

	bool GrowingWithDirection = true;

	FVector windOffset = FVector::Zero();
	FRotator lightRotation = FRotator::ZeroRotator;
	float crownRadius;
	float branchSize;
	float trunkheight;

};
