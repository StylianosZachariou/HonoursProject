// Fill out your copyright notice in the Description page of Project Settings.


#include "TreeNode.h"

#include "AttractionNode.h"
#include "Components/SphereComponent.h"

// Sets default values
ATreeNode::ATreeNode()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	auto Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));

	collider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
	collider->SetupAttachment(RootComponent);
//	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
//	StaticMeshComponent->SetStaticMesh(Mesh.Object);
//	StaticMeshComponent->SetRelativeScale3D(FVector(0.2, 0.2, 0.2));
//	StaticMeshComponent->SetupAttachment(RootComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);

	KillRange = CreateDefaultSubobject<USphereComponent>(TEXT("KillRange"));
	KillRange->SetupAttachment(RootComponent);

	currentDirection = FVector::UpVector;
	nextTreeNodePosition = nullptr;
	MaxDensity = 25;
}

void ATreeNode::OnKillOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAttractionNode* detectedAttractionNode = Cast<AAttractionNode>(OtherActor);

	if (detectedAttractionNode)
	{
		detectedAttractionNode->Destroy();
	}
	
}

void ATreeNode::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<UCapsuleComponent>(OtherComp))
	{
		if (AAttractionNode* detectedAttractionNode = Cast<AAttractionNode>(OtherActor))
		{
			attractionInfluences.Add(OtherActor);
			if (nodebranchLength > 0)
			{
				CalculateNextTreeNodePosition(false, nodebranchLength);
			}


		}
		else if (ATreeNode* detectedTreeNode = Cast<ATreeNode>(OtherActor))
		{
			detractionInfluences.Add(OtherActor);
			if (nodebranchLength > 0)
			{
				CalculateNextTreeNodePosition(false, nodebranchLength);
			}
		}
	}
}

void ATreeNode::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<UCapsuleComponent>(OtherComp))
	{
		if (AAttractionNode* detectedAttractionNode = Cast<AAttractionNode>(OtherActor))
		{
			attractionInfluences.RemoveSwap(OtherActor);
			if (nodebranchLength > 0)
			{
				CalculateNextTreeNodePosition(false, nodebranchLength);
			}
		}
	}
}

void ATreeNode::IncrementChildCount()
{
	numOfChildren++;
	if(parent)
	{
		parent->IncrementChildCount();
	}
}

// Called when the game starts or when spawned
void ATreeNode::BeginPlay()
{
	Super::BeginPlay();

	KillRange->OnComponentBeginOverlap.AddDynamic(this, &ATreeNode::OnKillOverlapBegin);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ATreeNode::OnOverlapBegin);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &ATreeNode::OnOverlapEnd);

}

void ATreeNode::CalculateNextTreeNodePosition(bool useDirection, float branchLength)
{
	if (numOfChildren <= 2)
	{
		if (!useDirection)
		{
			if (detractionInfluences.Num() < MaxDensity)
			{

				if (attractionInfluences.Num() > 0)
				{
					/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					FVector averageVector;
					for (auto vector : attractionInfluences)
					{
						FVector directionVector = vector->GetActorLocation() - GetActorLocation();
						directionVector.Normalize();
						averageVector += directionVector;
					}

					averageVector += FMath::VRand();
					averageVector += currentDirection/5;
					averageVector.Normalize();

					FVector badaverageVector;
					for (auto dvector : detractionInfluences)
					{
						FVector directionVector = GetActorLocation() - dvector->GetActorLocation();
						directionVector.Normalize();
						badaverageVector += directionVector;
					}
					badaverageVector.Normalize();

					FVector totalVector;
					totalVector += averageVector + badaverageVector * detractionInfluences.Num();
					totalVector.Normalize();

					FVector newSpawnLocation = GetActorLocation() + totalVector * branchLength;
					
					nextTreeNodePosition = new FVector(newSpawnLocation);
					//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				}
			}
		}
		else
		{
			FVector newSpawnLocation = GetActorLocation() + currentDirection * branchLength;
			nextTreeNodePosition = new FVector(newSpawnLocation);
		}

	}
	nodebranchLength = branchLength;
}

bool ATreeNode::HasAttractionInfluences()
{
	if(attractionInfluences.Num()>0)
	{
		return true;
	}
	return false;
}

// Called every frame
void ATreeNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TimeUntilDeath>=0)
	{
		TimeUntilDeath -= DeltaTime;
	}
}

void ATreeNode::ResetNextTreeNodePosition()
{
	delete nextTreeNodePosition;
	nextTreeNodePosition = nullptr;
}

bool ATreeNode::GetIsActive()
{
	return TimeUntilDeath>0;
}

void ATreeNode::CalculateCurrentDirection(FVector parentNodeLocation)
{
	currentDirection = GetActorLocation() - parentNodeLocation;
	FVector randomVector = FMath::VRand();
	randomVector.Z = FMath::Abs(randomVector.Z);
	currentDirection += randomVector / 2;
	currentDirection.Normalize();

}

FVector ATreeNode::GetCurrentDirection()
{
	return currentDirection;
}

FVector* ATreeNode::GetNextTreeNodePosition()
{
	return nextTreeNodePosition;
}

