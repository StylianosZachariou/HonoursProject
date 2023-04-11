#pragma once

#include "CoreMinimal.h"
#include "TreeSeed.h"
#include "SpaceColonizationTreeSeed.generated.h"

//Forward Declaration
class AAttractionNode;
class ATreeNode;

//The Space Colonization Tree
UCLASS()
class MYPROJECT_API ASpaceColonizationTreeSeed : public ATreeSeed
{
	GENERATED_BODY()

	//Constructor
	ASpaceColonizationTreeSeed();

public:

	//Nodes To Spawn
	UPROPERTY(Category = "Nodes", EditAnywhere)
		TSubclassOf<ATreeNode> TreeNodeToSpawn;

	UPROPERTY(Category = "Nodes", EditAnywhere)
		TSubclassOf<AAttractionNode> AttractionNodeToSpawn;

	//Attributes
	UPROPERTY(Category = "Nodes", EditAnywhere)
		int NumOfAttractionPoints;

	UPROPERTY(Category = "Branch", EditAnywhere)
		float BranchLength;

protected:

	//Functions
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:

	//Functions

	//Create Attraction Points
	void CreateAttractionPoints();
	void SpawnNewAttractionNode(FVector location);

	//Tree Nodes
	void QueueNewTreeNodes();
	void CreateNewNodes();
	void SpawnNewNode(ATreeNode* parentNode);

	//Mesh
	void CreateMesh();
	void GrowBranches(float DeltaTime);
	void CreateSphereMesh(ATreeNode* node);

	//General Variables
	bool GrowingWithDirection;

	//Nodes
	TArray<AAttractionNode*> attractionPoints;
	TArray<ATreeNode*> nodes;
	TArray<ATreeNode*> newNodeQueue;
	TArray<ATreeNode*> growingNodeQueue;
};
