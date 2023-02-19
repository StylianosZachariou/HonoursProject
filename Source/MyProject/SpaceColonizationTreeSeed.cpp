// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceColonizationTreeSeed.h"
#include "AttractionNode.h"
#include "TreeNode.h"

ASpaceColonizationTreeSeed::ASpaceColonizationTreeSeed()
{
	SeedSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SeedSceneComponent);
	NumOfAttractionPoints = 2000;
}

// Called when the game starts or when spawned
void ASpaceColonizationTreeSeed::BeginPlay()
{
	Super::BeginPlay();

	CreateAttractionPoints();
	SpawnNewNode();
}

void ASpaceColonizationTreeSeed::CreateAttractionPoints()
{
	int newAttractionPoints = 0;
	int radius = 1000;
	while (newAttractionPoints< NumOfAttractionPoints)
	{
		float x = FMath::FRandRange(-radius,radius);
		float y = FMath::FRandRange(-radius, radius);
		float z = FMath::FRandRange(-radius, radius);
		if (sqrt((x * x) + (y * y) + (z * z)) <= radius)
		{
			if (z >= 0)
			{
				FVector attractionPointPosition = FVector(x + GetActorLocation().X, y + GetActorLocation().Y, z + GetActorLocation().Z + 250);
				SpawnNewAttractionNode(attractionPointPosition);
				newAttractionPoints++;
			}
		}
	}
}

void ASpaceColonizationTreeSeed::SpawnNewAttractionNode(FVector location)
{
	AAttractionNode* newAttractionNode = GetWorld()->SpawnActor<AAttractionNode>(AttractionNodeToSpawn, location, FRotator(0, 0, 0));
	attractionPoints.Add(newAttractionNode);
}

void ASpaceColonizationTreeSeed::QueueNewTreeNodes()
{
	for (int i =0;i<nodes.Num();i++)
	{
		if (nodes[i]->GetIsActive() && nodes[i]->GetNextTreeNodePosition())
		{
			newNodeQueue.Add(nodes[i]);
		}
	}
}

void ASpaceColonizationTreeSeed::CreateNewNodes()
{
	int spawnCounter = 0;
	while (newNodeQueue.Num() > 0 && spawnCounter <= RateOfGrowth)
	{
		SpawnNewNode(newNodeQueue.Pop());
		spawnCounter++;
	}
}

void ASpaceColonizationTreeSeed::SpawnNewNode(ATreeNode* parentNode)
{
	if (parentNode->GetNextTreeNodePosition())
	{
		ATreeNode* newTreeNode = GetWorld()->SpawnActor<ATreeNode>(TreeNodeToSpawn, *parentNode->GetNextTreeNodePosition(), FRotator(0, 0, 0));

		if (newTreeNode)
		{

			if (GrowingWithDirection)
			{
				newTreeNode->CalculateCurrentDirection(*parentNode->GetNextTreeNodePosition()+FVector::DownVector);
				if (newTreeNode->HasAttractionInfluences())
				{
					GrowingWithDirection = false;
				}
			}
			else
			{
				newTreeNode->CalculateCurrentDirection(parentNode->GetActorLocation());
			}

			newTreeNode->CalculateNextTreeNodePosition(GrowingWithDirection);
			nodes.Add(newTreeNode);
			parentNode->ResetNextTreeNodePosition();
			parentNode->numOfChildren++;
		}
	}
}

void ASpaceColonizationTreeSeed::SpawnNewNode()
{
	ATreeNode* newTreeNode = GetWorld()->SpawnActor<ATreeNode>(TreeNodeToSpawn, GetActorLocation(), FRotator(0, 0, 0));
	newTreeNode->CalculateNextTreeNodePosition(GrowingWithDirection);
	nodes.Add(newTreeNode);
}

// Called every frame
void ASpaceColonizationTreeSeed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeOfGrowth -= DeltaTime;
	if (TimeOfGrowth>=0)
	{
		if (newNodeQueue.Num() > 0)
		{
			CreateNewNodes();
		}
		else
		{
			QueueNewTreeNodes();
		}
	}
	else
	{
		
		for(int i = 0 ; i < attractionPoints.Num(); i++)
		{
			if (ensure(attractionPoints[i]))
			{
				attractionPoints[i]->Destroy();
			}
		}
		attractionPoints.Empty();
	}
}

