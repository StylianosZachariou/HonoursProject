// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TreeSeed.h"
#include "ProceduralMeshComponent.h"
#include "GuidingVectorNode.h"
#include "ShortestPathTreeSeed.generated.h"

USTRUCT()
struct FBranch
{
	GENERATED_BODY()

	TArray<AGuidingVectorNode*> nodes;
};

UCLASS()
class MYPROJECT_API AShortestPathTreeSeed : public ATreeSeed
{
	GENERATED_BODY()

	AShortestPathTreeSeed();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(Category = "Mesh", VisibleAnywhere)
		UProceduralMeshComponent* MeshComponent;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		UMaterialInstance* Material;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SeedSceneComponent;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		TSubclassOf<AGuidingVectorNode> GuidingVectorNodeToSpawn;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		int NumberOfGuidingVectors;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		int NumberOfEndpoints;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		float MeshGrowthRate = 3.52;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		float levelOfDetail = 15;

	UPROPERTY(Category = "Environment", EditAnywhere)
		bool useLight = false;

	UPROPERTY(Category = "Environment", EditAnywhere)
		float MaximumAngleOfLightRotation = 15;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	void ApplyEnvironment();
	void GrowBranches(float DeltaTime);
	void ResetAStar();
	bool StepAStarAlgorithm();
	void SpawnAllGuidingVectors();
	void SetParentsGuidingVectors();
	void SpawnGuidingVector(FVector location);
	void ChooseEndpoints();
	void CreateTrunk();
	void CreateMesh();
	void GrowTrunk(float DeltaTime);
	void CreateSphereMesh(AGuidingVectorNode* node, float radius);

	TArray<AGuidingVectorNode*> guidingVectorNodes;
	TArray<AGuidingVectorNode*> visitedVectorNodes;
	TArray<AGuidingVectorNode*> unvisitedVectorNodes;
	TArray<int> endPointIndexes;

	TArray<AGuidingVectorNode*> finalTreeNodeActors;
	TArray<FBranch*> growingTreeNodes;
	FBranch* trunk;
	int trunkNodesGenerated = 0;

	float crownRadius=0;
	float trunkHeight=0;
	FVector windOffset=FVector::Zero();
	FRotator lightRotation = FRotator::ZeroRotator;
	bool trunkBuild = false;
};
