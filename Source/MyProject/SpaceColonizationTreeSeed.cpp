// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceColonizationTreeSeed.h"
#include "AttractionNode.h"
#include "EnvironmentSettings.h"
#include "KismetProceduralMeshLibrary.h"
#include "TreeNode.h"

ASpaceColonizationTreeSeed::ASpaceColonizationTreeSeed()
{
	SeedSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SeedSceneComponent);

	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	NumOfAttractionPoints = 2000;
}

// Called when the game starts or when spawned
void ASpaceColonizationTreeSeed::BeginPlay()
{
	Super::BeginPlay();

	CreateAttractionPoints();
	SpawnNewNode();

	//Moisture

	//Soil Acidity

	//Temperature
}

//ENVIRONMENT
void ASpaceColonizationTreeSeed::CreateAttractionPoints()
{
	int newAttractionPoints = 0;
	int radius = 500;

	AEnvironmentSettings* environment = Cast<AEnvironmentSettings>(GetWorldSettings());

	//Calculate Wind Offset

	FVector offset = FVector::Zero();


	//Calculate Light


	while (newAttractionPoints< NumOfAttractionPoints)
	{
		float x = FMath::FRandRange(-radius,radius);
		float y = FMath::FRandRange(-radius, radius);
		float z = FMath::FRandRange(-radius, radius);
		if (sqrt((x * x) + (y * y) + (z * z)) <= radius)
		{
			if (z >= 0)
			{
				FVector attractionPointPosition = FVector(x + GetActorLocation().X, y + GetActorLocation().Y, z + GetActorLocation().Z + 150);
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
			newTreeNode->parent = parentNode;
			nodes.Add(newTreeNode);
			parentNode->ResetNextTreeNodePosition();
			parentNode->IncrementChildCount();
		}
	}
}

void ASpaceColonizationTreeSeed::SpawnNewNode()
{
	ATreeNode* newTreeNode = GetWorld()->SpawnActor<ATreeNode>(TreeNodeToSpawn, GetActorLocation(), FRotator(0, 0, 0));
	newTreeNode->CalculateNextTreeNodePosition(GrowingWithDirection);
	nodes.Add(newTreeNode);
}

void ASpaceColonizationTreeSeed::CreateMesh()
{
	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector2D> uvs;
	int radialSubdivisions = 20;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, radialSubdivisions + 1, true, triangles);
	
	MeshComponent->ClearAllMeshSections();
	if (nodes.Num() >= 2)
	{
		for (int i = 0; i < nodes.Num(); i++)
		{
			if (nodes[i]->parent)
			{
				float radius = pow(nodes[i]->numOfChildren*MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

				//This Node
				for (int s = 0; s < radialSubdivisions + 1; s++)
				{
					FVector pos = FVector::Zero();

					FTransform rot = FTransform::Identity;
					rot.SetRotation(nodes[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());


					FTransform tf = FTransform::Identity;
					tf.AddToTranslation(nodes[i]->GetTransform().GetTranslation());

					// radial angle of the vertex
					float alpha = ((float)s / radialSubdivisions) * PI * 2.f;
					FTransform anotherOne = FTransform::Identity;
					anotherOne.AddToTranslation(FVector(FMath::Cos(alpha) * radius, FMath::Sin(alpha) * radius, 0));

					FTransform total = anotherOne * rot * tf;

					pos = total.TransformPosition(pos);
					pos -= GetActorLocation();

					vertices.Add(pos);

					FVector2D uv = FVector2D(s, 0);
					uvs.Add(uv);
				}

				radius =  pow(nodes[i]->parent->numOfChildren * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;
				//Parent
				for (int s = 0; s < radialSubdivisions + 1; s++)
				{
					FVector pos = FVector::Zero();

					FTransform rot = FTransform::Identity;
					rot.SetRotation(nodes[i]->parent->GetCurrentDirection().ToOrientationRotator().Add(90,0,0).Quaternion());

					FTransform tf = FTransform::Identity;
					tf.AddToTranslation(nodes[i]->parent->GetTransform().GetTranslation());

					// radial angle of the vertex
					float alpha = ((float)s / radialSubdivisions) * PI * 2.f;
					FTransform anotherOne = FTransform::Identity;
					anotherOne.AddToTranslation(FVector(FMath::Cos(alpha) * radius, FMath::Sin(alpha) * radius, 0));

					FTransform total = anotherOne * rot * tf;

					pos = total.TransformPosition(pos);
					pos -= GetActorLocation();

					vertices.Add(pos);
					FVector2D uv = FVector2D(s, 0);
					uvs.Add(uv);
				}


			}

			//Create Section
			MeshComponent->CreateMeshSection(MeshComponent->GetNumSections(), vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
			
			if(nodes[i]->numOfChildren==0)
			{
				FVector pos = FVector::Zero();
				FTransform tf = FTransform::Identity;
				tf.AddToTranslation(nodes[i]->GetTransform().GetTranslation());
				pos = tf.TransformPosition(pos);
				pos -= GetActorLocation();
				vertices.Add(pos);

				TArray<int32> circleIndices;
				for(int s = 0; s < radialSubdivisions + 1; s++)
				{
					int index1 = s;
					int index2 = s + 1;
					if(index1 >=radialSubdivisions)
					{
						index1 -= radialSubdivisions;
					}

					if (index2 >= radialSubdivisions)
					{
						index2 -= radialSubdivisions;
					}
					circleIndices.Add(index1);
					circleIndices.Add(index2);
					circleIndices.Add(vertices.Num()-1);
				}

				//Create Section
				MeshComponent->CreateMeshSection(MeshComponent->GetNumSections(), vertices, circleIndices, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
			}
			vertices.Empty();
			uvs.Empty();
		}

		//Material
		UMaterialInstanceDynamic* dynamicMaterial = UMaterialInstanceDynamic::Create(Material, MeshComponent);
		MeshComponent->SetMaterial(0, dynamicMaterial);
	}
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
		CreateMesh();
	}
//	else
//	{
		
	//	for(int i = 0 ; i < attractionPoints.Num(); i++)
	//	{
	//		if (ensure(attractionPoints[i]))
	//		{
	//			attractionPoints[i]->Destroy();
	//		}
	//	}
	//	attractionPoints.Empty();/
//	}
}

