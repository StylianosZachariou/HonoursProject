#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "AttractionNode.generated.h"

//Attraction Points used in the space colonization algorithm
UCLASS()
class MYPROJECT_API AAttractionNode : public AActor
{
	GENERATED_BODY()
	
public:	

	//Constructor
	AAttractionNode();

	//Component Declarations
	UPROPERTY(VisibleAnywhere)
		USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere)
		UCapsuleComponent* collider;

	///////////////// DEMONSTRATION PURPOSES /////////////////
/*	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* StaticMeshComponent;*/
	/////////////////////////////////////////////////////////
};
