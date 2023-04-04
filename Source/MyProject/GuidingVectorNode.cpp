// Fill out your copyright notice in the Description page of Project Settings.


#include "GuidingVectorNode.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AGuidingVectorNode::AGuidingVectorNode()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	collider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
	collider->SetupAttachment(RootComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);

	auto Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	auto Material = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Game/StarterContent/Materials/M_Metal_Gold.M_Metal_Gold'"));
	endPointMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Game/StarterContent/Materials/M_Water_Lake.M_Water_Lake'")).Object;

//	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
//	StaticMeshComponent->SetStaticMesh(Mesh.Object);
//	StaticMeshComponent->SetMaterial(0, Material.Object);
//	StaticMeshComponent->SetRelativeScale3D(FVector(0.1, 0.1, 0.1));
//	StaticMeshComponent->SetupAttachment(RootComponent);
}

void AGuidingVectorNode::SetParent(FVector location)
{
	if (!hasParent)
	{
		//Rotate This Node
		//StaticMeshComponent->SetStaticMesh(coneMesh);
		directionVector = location- GetActorLocation();
		directionVector.Normalize();
		//StaticMeshComponent->SetRelativeRotationExact(directionVector.ToOrientationRotator().Add(270,0,0));
		hasParent = true;
	}
}

void AGuidingVectorNode::SetThisAsConnectionsParent()
{
	for(int i =0;i<connections.Num();i++)
	{
		connections[i]->SetParent(GetActorLocation());
		isParent = true;
	}
}

bool AGuidingVectorNode::GetIsEndpoint()
{
	return isEndpoint;
}

void AGuidingVectorNode::SetPrevious(AGuidingVectorNode* previous)
{
	previousNode = previous;
}

AGuidingVectorNode* AGuidingVectorNode::GetPrevious()
{
	return previousNode;
}

void AGuidingVectorNode::SetNext(AGuidingVectorNode* next)
{
	nextNode = next;
	if (next)
	{
		pathDirectionVector = GetActorLocation() - next->GetActorLocation();
		pathDirectionVector.Normalize();
	}
	else
	{
		pathDirectionVector = FVector::Zero();
	}
}

bool AGuidingVectorNode::GetIsParent()
{
	return isParent;
}

bool AGuidingVectorNode::GetHasParent()
{
	return hasParent;
}

void AGuidingVectorNode::SetEndpoint()
{
//	StaticMeshComponent->SetMaterial(0, endPointMaterial);
	isEndpoint = true;
}

void AGuidingVectorNode::CalculateFGScores(FVector endLocation)
{
	float gScoreUntilNow = 0;
	float thisGScore = 0;


	if (previousNode)
	{
		gScoreUntilNow = previousNode->GetGScore();

		thisGScore = FVector::Dist(previousNode->GetActorLocation(), GetActorLocation());

		if (FVector::DotProduct(directionVector, pathDirectionVector) > 0)
		{
			thisGScore /= 2;
		}
	}

	gscore = thisGScore + gScoreUntilNow;
	fscore = gscore + FVector::Dist(GetActorLocation(), endLocation);
}

void AGuidingVectorNode::detectConnections()
{
	TArray<AActor*> overlappingActors;
	collider->GetOverlappingActors(overlappingActors,AGuidingVectorNode::StaticClass());

	for(int i =0; i<overlappingActors.Num();i++)
	{
		if (overlappingActors[i] != this)
		{
			AGuidingVectorNode* detectedGuidingVector = Cast<AGuidingVectorNode>(overlappingActors[i]);

			if (!detectedGuidingVector->isEndpoint)
			{
				connections.Add(detectedGuidingVector);
			}
		}
	}
}

float AGuidingVectorNode::GetFScore()
{
	return fscore;
}

AGuidingVectorNode* AGuidingVectorNode::GetNext()
{
	return nextNode;
}

float AGuidingVectorNode::GetGScore()
{
	return gscore;
}

void AGuidingVectorNode::SetUsed(bool used)
{
	isUsed = used;
}

bool AGuidingVectorNode::GetUsed()
{
	return isUsed;
}

FVector AGuidingVectorNode::GetCurrentDirection()
{
	return pathDirectionVector;
}

float AGuidingVectorNode::GetGrowingTimer()
{
	return growingTimer;
}

void AGuidingVectorNode::AddToGrowingTimer(float time)
{
	growingTimer += time;
}

void AGuidingVectorNode::IncrementChildrenCount()
{
	numOfChildren++;
	if(nextNode)
	{
		nextNode->IncrementChildrenCount();
	}
}

int AGuidingVectorNode::GetNumOfChildren()
{
	return numOfChildren;
}

TArray<AGuidingVectorNode*> AGuidingVectorNode::GetConnections()
{
	return connections;
}

void AGuidingVectorNode::ResetInfo()
{
	gscore = -1;
	fscore = -1;
	previousNode = nullptr;
}

void AGuidingVectorNode::BeginPlay()
{
	Super::BeginPlay();

}

