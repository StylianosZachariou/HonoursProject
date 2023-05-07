#pragma once

#include "CoreMinimal.h"
#include "TreeSeed.h"
#include "GuidingVectorNode.h"
#include "ShortestPathTreeSeed.generated.h"

//Branch structure
USTRUCT()
struct FBranch
{
	GENERATED_BODY()

	//All nodes in this branch
	TArray<AGuidingVectorNode*> nodes;
};

//The Shortest Path algorithm tree
UCLASS()
class MYPROJECT_API AShortestPathTreeSeed : public ATreeSeed
{
	GENERATED_BODY()

	//Constructor
	AShortestPathTreeSeed();

public:
	//Attributes
	UPROPERTY(Category = "Nodes", EditAnywhere)
		TSubclassOf<AGuidingVectorNode> GuidingVectorNodeToSpawn;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		int NumberOfGuidingVectors;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		int NumberOfEndpoints;

protected:

	//Functions
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	//Functions
	void ResetAStar();
	bool StepAStarAlgorithm();

	//Nodes
	void SpawnAllGuidingVectors();
	void SpawnGuidingVector(FVector location);
	void SetParentsGuidingVectors();
	void ChooseEndpoints();
	void CreateTrunk();

	//Mesh
	void GrowBranches(float DeltaTime);
	void GrowTrunk(float DeltaTime);
	void CreateMesh();
	void CreateSphereMesh(AGuidingVectorNode* node);

	//General Variables
	bool trunkGenerated;
	bool trunkBuild;

	//Mesh
	TArray<AGuidingVectorNode*> finalTreeNodeActors;
	TArray<FBranch*> growingTreeNodes;
	int growingBranchesGenerated;

	//A Star Arrays
	TArray<AGuidingVectorNode*> guidingVectorNodes;
	TArray<AGuidingVectorNode*> visitedVectorNodes;
	TArray<AGuidingVectorNode*> unvisitedVectorNodes;
	TArray<int> endPointIndexes;

	///////////////////////////DELETE THIS AFETER
	float currentTime;
	TArray<FString> times;
};
