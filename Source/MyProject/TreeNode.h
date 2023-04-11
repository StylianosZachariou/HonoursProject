#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "TreeNode.generated.h"

//Forward Declarations
class USphereComponent;

//Tree node class used in Space Colonization algorithm
UCLASS()
class MYPROJECT_API ATreeNode : public AActor
{
	GENERATED_BODY()
	
public:	
	//Constructor
	ATreeNode();

	//Component Declaration
	UPROPERTY(VisibleAnywhere)
		USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere)
		USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere)
		USphereComponent* KillRange;

	UPROPERTY(VisibleAnywhere)
		UCapsuleComponent* collider;

	///////////////// DEMONSTRATION PURPOSES /////////////////
	//	UPROPERTY(EditAnywhere)
	//		UStaticMeshComponent* StaticMeshComponent;
	//////////////////////////////////////////////////////////

	//Attributes
	UPROPERTY(EditAnywhere)
		float TimeUntilDeath;

	UPROPERTY(EditAnywhere)
		float MaxDensity;

	//Overlap Functions
	UFUNCTION()
		void OnKillOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,	bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//Getters
	FVector* GetNextTreeNodePosition();
	FVector GetCurrentDirection();
	int GetMeshSectionIndex();
	bool GetIsActive();
	bool GetHasAttractionInfluences();
	int GetNumOfChildren();
	float GetGrowingTimer();
	ATreeNode* GetParent();

	//Setters
	void SetMeshSectionIndex(int index);
	void SetParent(ATreeNode* newParent);

	//Functions
	virtual void Tick(float DeltaTime) override;
	void ResetNextTreeNodePosition();
	void CalculateCurrentDirection(FVector parentNodeLocation);
	void CalculateNextTreeNodePosition(bool useDirection, float branchLength);
	void IncrementChildCount();
	void AddToGrowingTimer(float time);

protected:

	//Functions
	virtual void BeginPlay() override;

private:

	//General Variables
	FVector currentDirection;
	float nodebranchLength;

	//Influences
	TArray<AActor*> attractionInfluences;
	TArray<AActor*> detractionInfluences;

	//Mesh Variables
	int meshSectionIndex;
	int numOfChildren;
	float growingTimer;

	//Previous and Next Nodes
	ATreeNode* parent;
	FVector* nextTreeNodePosition;
};
