#include "TreeNode.h"
#include "AttractionNode.h"
#include "Components/SphereComponent.h"

//Constructor
ATreeNode::ATreeNode():
currentDirection(FVector::UpVector),
nextTreeNodePosition(nullptr),
MaxDensity(25),
meshSectionIndex(-1),
growingTimer(0),
numOfChildren(0),
parent(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	//Component Initializations
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	collider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
	collider->SetupAttachment(RootComponent);

	//Detection Radiuses
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);

	KillRange = CreateDefaultSubobject<USphereComponent>(TEXT("KillRange"));
	KillRange->SetupAttachment(RootComponent);

	///////////////// DEMONSTRATION PURPOSES /////////////////
/*	auto Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetStaticMesh(Mesh.Object);
	StaticMeshComponent->SetRelativeScale3D(FVector(0.2, 0.2, 0.2));
	StaticMeshComponent->SetupAttachment(RootComponent);*/
	//////////////////////////////////////////////////////////
}


///////////Overlap Event Functions///////////

//On overlap with the kill radius
void ATreeNode::OnKillOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAttractionNode* detectedAttractionNode = Cast<AAttractionNode>(OtherActor);

	//If an attraction node is detected
	if (detectedAttractionNode)
	{
		//Destroy it
		detectedAttractionNode->Destroy();
	}
	
}

//On overlap with the detection radius
void ATreeNode::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//If the component detected is a capsule
	if (Cast<UCapsuleComponent>(OtherComp))
	{
		//If its an attraction node
		if (AAttractionNode* detectedAttractionNode = Cast<AAttractionNode>(OtherActor))
		{
			//Add to attraction influeces
			attractionInfluences.Add(OtherActor);
			if (nodebranchLength > 0)
			{
				//Calculate next position
				CalculateNextTreeNodePosition(false, nodebranchLength);
			}
		}
		else if (ATreeNode* detectedTreeNode = Cast<ATreeNode>(OtherActor)) //If its a tree node
		{
			//Add to detraction influences
			detractionInfluences.Add(OtherActor);
			if (nodebranchLength > 0)
			{
				//Calclate next position
				CalculateNextTreeNodePosition(false, nodebranchLength);
			}
		}
	}
}

//On the end of overlap with the detection radius
void ATreeNode::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//If component is capsule component
	if (Cast<UCapsuleComponent>(OtherComp))
	{
		//If detected node is attraction node
		if (AAttractionNode* detectedAttractionNode = Cast<AAttractionNode>(OtherActor))
		{
			//Remove it from attraction influences
			attractionInfluences.RemoveSwap(OtherActor);
			if (nodebranchLength > 0)
			{
				//Calculate next position
				CalculateNextTreeNodePosition(false, nodebranchLength);
			}
		}
	}
}


///////////Getters///////////

//Mesh Section Index
int ATreeNode::GetMeshSectionIndex()
{
	return meshSectionIndex;
}

//Has Attraction Influences
bool ATreeNode::GetHasAttractionInfluences()
{
	if(attractionInfluences.Num()>0)
	{
		return true;
	}
	return false;
}

//Number of Children
int ATreeNode::GetNumOfChildren()
{
	return numOfChildren;
}

//Growing Timer
float ATreeNode::GetGrowingTimer()
{
	return growingTimer;
}

//Parent node
ATreeNode* ATreeNode::GetParent()
{
	return parent;
}

//Is Active
bool ATreeNode::GetIsActive()
{
	return TimeUntilDeath>0;
}

//Current Direction
FVector ATreeNode::GetCurrentDirection()
{
	return currentDirection;
}

//Next Node Position
FVector* ATreeNode::GetNextTreeNodePosition()
{
	return nextTreeNodePosition;
}


///////////Setters///////////

//Mesh Section Index
void ATreeNode::SetMeshSectionIndex(int index)
{
	meshSectionIndex = index;
}

//Parent Node
void ATreeNode::SetParent(ATreeNode* newParent)
{
	parent = newParent;
}


///////////Functions///////////

//Increment Children Count
void ATreeNode::IncrementChildCount()
{
	//Increment this nodes children
	numOfChildren++;
	if(parent)
	{
		//Increment Parents children
		parent->IncrementChildCount();
	}
}

//Increment growing timer
void ATreeNode::AddToGrowingTimer(float time)
{
	growingTimer += time;
}

//Calculate next tree node's position
void ATreeNode::CalculateNextTreeNodePosition(bool useDirection, float branchLength)
{
	//If this node has less than 3 children 
	if (numOfChildren <= 2)
	{
		//If it has influences
		if (!useDirection)
		{
			//If there are not too many nodes around this node
			if (detractionInfluences.Num() < MaxDensity)
			{
				//If there are attraction influences
				if (attractionInfluences.Num() > 0)
				{
					//Calculating attraction vector
					FVector attractionVector;

					//For all attraction influences
					for (auto vector : attractionInfluences)
					{
						//Find direction vector between this node and influence
						FVector directionVector = vector->GetActorLocation() - GetActorLocation();
						directionVector.Normalize();

						//Add to average vector
						attractionVector += directionVector;
					}

					//Add random vector
					attractionVector += FMath::VRand();

					//Add current direction
					attractionVector += currentDirection/5;

					attractionVector.Normalize();

					//Calculating the detraction vector
					FVector detractionVector;

					//For all detraction influences
					for (auto dvector : detractionInfluences)
					{
						//Calculate direction vector between this node and detraction node
						FVector directionVector = GetActorLocation() - dvector->GetActorLocation();
						directionVector.Normalize();

						//Add it to detraction vector
						detractionVector += directionVector;
					}
					detractionVector.Normalize();

					//Calculating the total vector
					FVector totalVector;

					//Adding attraction vector and detraction vector. (The more detraction influences the more influence detraction will have)
					totalVector += attractionVector + detractionVector * detractionInfluences.Num();
					totalVector.Normalize();

					//Calculating the new spawn location
					FVector newSpawnLocation = GetActorLocation() + totalVector * branchLength;

					nextTreeNodePosition = new FVector(newSpawnLocation);
					
				}
			}
		}
		else //Only use direction for calculating next node's location
		{
			FVector newSpawnLocation = GetActorLocation() + currentDirection * branchLength;
			nextTreeNodePosition = new FVector(newSpawnLocation);
		}

	}
	nodebranchLength = branchLength;
}

//Reset next tree node position
void ATreeNode::ResetNextTreeNodePosition()
{
	delete nextTreeNodePosition;
	nextTreeNodePosition = nullptr;
}

//Calculate current direction
void ATreeNode::CalculateCurrentDirection(FVector parentNodeLocation)
{
	//Calculate current direction with the addition of a random factor
	currentDirection = GetActorLocation() - parentNodeLocation;
	FVector randomVector = FMath::VRand();
	randomVector.Z = FMath::Abs(randomVector.Z);
	currentDirection += randomVector / 2;
	currentDirection.Normalize();
}

//At the beginning of the application
void ATreeNode::BeginPlay()
{
	Super::BeginPlay();

	//Add Overlap events to respective components
	KillRange->OnComponentBeginOverlap.AddDynamic(this, &ATreeNode::OnKillOverlapBegin);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ATreeNode::OnOverlapBegin);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &ATreeNode::OnOverlapEnd);

}

//Called every frame
void ATreeNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If node is alive
	if (TimeUntilDeath>=0)
	{
		//Bring closer to death
		TimeUntilDeath -= DeltaTime;
	}
}
