#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "TreeSeed.generated.h"

//Base class for any procedurally generated tree classes
UCLASS()
class MYPROJECT_API ATreeSeed : public AActor
{
	GENERATED_BODY()
	
public:

	//Constructor
	ATreeSeed();

	//Components
	UPROPERTY(Category = "SceneComponent", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SeedSceneComponent;

	//Mesh
	UPROPERTY(Category = "Mesh", VisibleAnywhere)
		UProceduralMeshComponent* MeshComponent;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		UMaterialInstance* Material;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		int NodeMeshesRenderedPerFrame;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		int GrowingNodeMeshesRenderedPerFrame;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		float MeshGrowthRate;

	UPROPERTY(Category = "Mesh", EditAnywhere)
		float levelOfDetail;

	//Environment
	UPROPERTY(Category = "Environment", EditAnywhere)
		bool applyEnvironment;

	UPROPERTY(Category = "Environment", EditAnywhere)
		bool useLight;
	
	UPROPERTY(Category = "Environment", EditAnywhere)
		float MaximumAngleOfLightRotation;

	//Attributes
	UPROPERTY(Category = "Attributes", EditAnywhere)
		float crownRadius;

	UPROPERTY(Category = "Attributes", EditAnywhere)
		float trunkHeight;

	UPROPERTY(Category = "Attributes", EditAnywhere)
		float TimeOfGrowth;

	UPROPERTY(Category = "Attributes", EditAnywhere)
		float RateOfGrowth;
protected:

	//Functions
	void ApplyEnvironment();
	void CalculateNodeMeshVerticesAndUV(float radius, FRotator rotation, FVector translation, TArray<FVector> &vertices, TArray<FVector2D> &uvs);

	//Environment variables
	FVector windOffset;
	FRotator lightRotation;

	//Mesh Variables
	int renderedNodeMeshes;
	int newBranchesGenerated;

};
