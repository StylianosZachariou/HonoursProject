#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GuidingVectorNode.generated.h"

//Forward Declarations
class USphereComponent;
class UCapsuleComponent;

//Guiding Vectors used for the shortest path algorithm
UCLASS()
class MYPROJECT_API AGuidingVectorNode : public AActor
{
	GENERATED_BODY()

public:
	//Constructor
	AGuidingVectorNode();

	//Component Declarations
	UPROPERTY(VisibleAnywhere)
		USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere)
		USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere)
		UCapsuleComponent* collider;

	///////////////// DEMONSTRATION PURPOSES /////////////////
/*	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere)
		UStaticMesh* coneMesh;*/
	/////////////////////////////////////////////////////////

	//Getters
	bool GetHasParent();
	bool GetIsParent();
	bool GetIsEndpoint();
	AGuidingVectorNode* GetPrevious();
	AGuidingVectorNode* GetNext();
	float GetFScore();
	float GetGScore();
	FVector GetCurrentDirection();
	float GetGrowingTimer();
	TArray<AGuidingVectorNode*> GetConnections();
	int GetNumOfChildren();
	int GetMeshSectionIndex();

	//Setters
	void SetParent(FVector location);
	void SetThisAsConnectionsParent();
	void SetEndpoint();
	void SetPrevious(AGuidingVectorNode* previous);
	void SetNext(AGuidingVectorNode* next);
	void SetMeshSectionIndex(int index);

	//Functions
	void detectConnections();
	void CalculateFGScores(FVector endLocation);
	void AddToGrowingTimer(float time);
	void ResetInfo();
	void IncrementChildrenCount();

protected:

	//Functions
	virtual void BeginPlay() override;

private:

	//General Variables
	bool isParent;
	bool hasParent;
	bool isEndpoint;
	float fscore;
	float gscore;

	//Guiding Vector Direction
	FVector directionVector;

	//Front and Back Nodes
	AGuidingVectorNode* nextNode;
	AGuidingVectorNode* previousNode;

	//Connected Nodes
	TArray<AGuidingVectorNode*> connections;

	//Mesh Variables
	int meshSectionIndex;
	float growingTimer;
	FVector pathDirectionVector;
	int numOfChildren;

	///////////////// DEMONSTRATION PURPOSES /////////////////
	//UMaterial* endPointMaterial;
	/////////////////////////////////////////////////////////
};
