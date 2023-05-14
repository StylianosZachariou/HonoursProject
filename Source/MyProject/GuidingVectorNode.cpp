#include "GuidingVectorNode.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

//Constructor
AGuidingVectorNode::AGuidingVectorNode() :
isParent(false),
hasParent(false),
isEndpoint(false),
fscore(-1),
gscore(-1),
meshSectionIndex(-1),
growingTimer(0),
numOfChildren(0)
{
	PrimaryActorTick.bCanEverTick = true;

	//Component Initialization
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	Collider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
	Collider->SetupAttachment(RootComponent);

	//detection radius
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);

	///////////////// DEMONSTRATION PURPOSES /////////////////
/*	auto Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	auto Material = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Game/StarterContent/Materials/M_Metal_Gold.M_Metal_Gold'"));
	endPointMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Game/StarterContent/Materials/M_Water_Lake.M_Water_Lake'")).Object;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetStaticMesh(Mesh.Object);
	StaticMeshComponent->SetMaterial(0, Material.Object);
	StaticMeshComponent->SetRelativeScale3D(FVector(0.1, 0.1, 0.1));
	StaticMeshComponent->SetupAttachment(RootComponent);*/
	/////////////////////////////////////////////////////////
}


///////////Getters///////////

//Is Endpoint
bool AGuidingVectorNode::GetIsEndpoint()
{
	return isEndpoint;
}

//Is Parent
bool AGuidingVectorNode::GetIsParent()
{
	return isParent;
}

//Has Parent
bool AGuidingVectorNode::GetHasParent()
{
	return hasParent;
}

//F Score
float AGuidingVectorNode::GetFScore()
{
	return fscore;
}

//Next node
AGuidingVectorNode* AGuidingVectorNode::GetNext()
{
	return nextNode;
}

//G Score
float AGuidingVectorNode::GetGScore()
{
	return gscore;
}

//Direction
FVector AGuidingVectorNode::GetCurrentDirection()
{
	return pathDirectionVector;
}

//Growing TImer
float AGuidingVectorNode::GetGrowingTimer()
{
	return growingTimer;
}

//Number of Children
int AGuidingVectorNode::GetNumOfChildren()
{
	return numOfChildren;
}

//Mesh Section Index
int AGuidingVectorNode::GetMeshSectionIndex()
{
	return meshSectionIndex;
}

//Connected Nodes
TArray<AGuidingVectorNode*> AGuidingVectorNode::GetConnections()
{
	return connections;
}

//Previous Node
AGuidingVectorNode* AGuidingVectorNode::GetPrevious()
{
	return previousNode;
}



///////////Setters///////////

//Is Parent
void AGuidingVectorNode::SetParent(FVector location)
{
	//If no parent
	if (!hasParent)
	{
		//Change direction of guiding vector
		directionVector = location- GetActorLocation();
		directionVector.Normalize();
		hasParent = true;

		///////////////// DEMONSTRATION PURPOSES /////////////////
//		StaticMeshComponent->SetStaticMesh(coneMesh);
//		StaticMeshComponent->SetRelativeRotationExact(directionVector.ToOrientationRotator().Add(270,0,0));
		//////////////////////////////////////////////////////////
	}
}

//Previous node
void AGuidingVectorNode::SetPrevious(AGuidingVectorNode* previous)
{
	previousNode = previous;
}

//Next node
void AGuidingVectorNode::SetNext(AGuidingVectorNode* next)
{
	//Save next node
	nextNode = next;

	if (next)
	{
		//Calculate path direction
		pathDirectionVector = GetActorLocation() - next->GetActorLocation();
		pathDirectionVector.Normalize();

	}
	else
	{
		//Clear Direction
		pathDirectionVector = FVector::Zero();
	}
}

//Endpoint
void AGuidingVectorNode::SetEndpoint()
{
	///////////////// DEMONSTRATION PURPOSES /////////////////
//	StaticMeshComponent->SetMaterial(0, endPointMaterial);
	/////////////////////////////////////////////////////////
	isEndpoint = true;
}

//Mesh Section Index
void AGuidingVectorNode::SetMeshSectionIndex(int index)
{
	meshSectionIndex = index;
}


//////////Functions//////////

//Calculate F and G score of this node, using goal location
void AGuidingVectorNode::CalculateFGScores(FVector endLocation)
{
	float gScoreUntilNow = 0;
	float thisGScore = 0;

	if (previousNode)
	{
		//Get the previous node's G score
		gScoreUntilNow = previousNode->GetGScore();

		//Calculate this node's G score
		thisGScore = FVector::Dist(previousNode->GetActorLocation(), GetActorLocation());

		//G score is less if guiding vector is pointing at the right direction
		if (FVector::DotProduct(directionVector, pathDirectionVector) > 0)
		{
			thisGScore /= 2;
		}
	}

	//Total G Score
	gscore = thisGScore + gScoreUntilNow;

	//F score is gscore added to the distance to the end location
	fscore = gscore + FVector::Dist(GetActorLocation(), endLocation);
}

//Detect All connections
void AGuidingVectorNode::DetectConnections()
{
	TArray<AActor*> overlappingActors;
	SphereComponent->GetOverlappingActors(overlappingActors, AGuidingVectorNode::StaticClass());;

	while(overlappingActors.Num()<=0)
	{
		SphereComponent->SetSphereRadius(SphereComponent->GetScaledSphereRadius() + 1);
		SphereComponent->GetOverlappingActors(overlappingActors, AGuidingVectorNode::StaticClass());
	}

	//For all overlapping guiding vectors
	for(int i =0; i<overlappingActors.Num();i++)
	{
		//If its not this
		if (overlappingActors[i] != this)
		{
			AGuidingVectorNode* detectedGuidingVector = Cast<AGuidingVectorNode>(overlappingActors[i]);

			//If its not an  end point
			if (!detectedGuidingVector->isEndpoint)
			{
				//Add to connections
				connections.Add(detectedGuidingVector);
			}
		}
	}
}

//Increment the growing timer
void AGuidingVectorNode::AddToGrowingTimer(float time)
{
	growingTimer += time;
}

//Increment children count
void AGuidingVectorNode::IncrementChildrenCount()
{
	//Increment Children Count
	numOfChildren++;
	if(nextNode)
	{
		//Increment Next Node's Children Count
		nextNode->IncrementChildrenCount();
	}
}

//Reset node's info
void AGuidingVectorNode::ResetInfo()
{
	//Reset Variables
	gscore = -1;
	fscore = -1;
	previousNode = nullptr;
}

//Set this node as the parent of all its connections
void AGuidingVectorNode::SetThisAsConnectionsParent()
{
	//For all connections
	for(int i =0;i<connections.Num();i++)
	{
		//Set this as the parent
		connections[i]->SetParent(GetActorLocation());
		isParent = true;
	}
}

