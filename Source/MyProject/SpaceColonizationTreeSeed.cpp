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

	ApplyEnvironment();
	CreateAttractionPoints();
	SpawnNewNode();
}

void ASpaceColonizationTreeSeed::ApplyEnvironment()
{
	//Just for testing
	crownRadius = 500;
	trunkheight = 150;

	AEnvironmentSettings* environment = Cast<AEnvironmentSettings>(GetWorldSettings());

	float maxWindPower = 75;
	float windPower = FMath::Min(environment->GetWindPower(), maxWindPower);
	//Wind
	//If the wind is strong
	if(windPower >25)
	{
		FVector direction = environment->GetWindDirection();
		direction.Normalize();
		windOffset = direction * ((windPower * crownRadius) / maxWindPower);
	}

	FVector crownStartPosition = GetActorLocation();
	crownStartPosition.Z += trunkheight;

	//Light
	if (useLight)
	{
		FVector lightGrowthDirection = environment->GetLightPosition() - crownStartPosition;
		lightGrowthDirection.Normalize();

		lightRotation = lightGrowthDirection.ToOrientationRotator().Add(-90, 0, 0);
		lightRotation.Pitch = FMath::Min(lightRotation.Pitch, MaximumAngleOfLightRotation);
		lightRotation.Pitch = FMath::Max(lightRotation.Pitch, -MaximumAngleOfLightRotation);
	}

	
	//Moisture
	RateOfGrowth = 1;
	float moisturePercentage = environment->GetMoisture();
	if(moisturePercentage>45)
	{
		RateOfGrowth -= FMath::Abs(45 - moisturePercentage) / 25;
	}
	else if(moisturePercentage < 20)
	{
		RateOfGrowth -= FMath::Abs(20 - moisturePercentage) / 10;
	}

	//Soil Acidity
	float soilPH = environment->GetSoilAcidity();
	if(soilPH>8)
	{
		RateOfGrowth -= FMath::Abs(8 - soilPH) / 3;
	}
	else if (soilPH < 4.5)
	{
		RateOfGrowth -= FMath::Abs(4.5 - soilPH) / 2;
	}

	//Temperature
	float temperatureCelcius = environment->GetTemperature();
	if(temperatureCelcius>30)
	{
		RateOfGrowth -= FMath::Abs(30 - temperatureCelcius) / 11;
	}
	else if (temperatureCelcius<25)
	{
		RateOfGrowth -= FMath::Abs(25 - temperatureCelcius) / 21;
	}

	RateOfGrowth = FMath::Max(0, RateOfGrowth);
}

void ASpaceColonizationTreeSeed::CreateAttractionPoints()
{
	int newAttractionPoints = 0;

	while (attractionPoints.Num() < NumOfAttractionPoints)
	{
		FVector pos;
		pos.X = FMath::FRandRange(-crownRadius, crownRadius);
		pos.Y = FMath::FRandRange(-crownRadius, crownRadius);
		pos.Z = FMath::FRandRange(-crownRadius, crownRadius);

		if (sqrt((pos.X * pos.X) + (pos.Y * pos.Y) + (pos.Z * pos.Z)) <= crownRadius)
		{
			if (pos.Z >= 0)
			{
				FVector attractionPointPosition = FVector::Zero();

				FTransform translation2 = FTransform::Identity;
				translation2.AddToTranslation(FVector(pos));
				attractionPointPosition = translation2.TransformPosition(attractionPointPosition);

				FTransform rot = FTransform::Identity;
				rot.SetRotation(lightRotation.Quaternion());
				
				attractionPointPosition = rot.TransformPosition(attractionPointPosition);

				FTransform translation = FTransform::Identity;
				translation.AddToTranslation(GetActorLocation() + windOffset);
				translation.AddToTranslation(FVector(0, 0, trunkheight));
				attractionPointPosition = translation.TransformPosition(attractionPointPosition);
				
				SpawnNewAttractionNode(attractionPointPosition);
				
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
	while (newNodeQueue.Num() > 0 && spawnCounter <= 2)
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

			newTreeNode->CalculateNextTreeNodePosition(GrowingWithDirection, BranchLength);
			newTreeNode->parent = parentNode;
			growingNodeQueue.Add(newTreeNode);
			parentNode->ResetNextTreeNodePosition();
			parentNode->IncrementChildCount();
		}
	}
}

void ASpaceColonizationTreeSeed::SpawnNewNode()
{
	ATreeNode* newTreeNode = GetWorld()->SpawnActor<ATreeNode>(TreeNodeToSpawn, GetActorLocation(), FRotator(0, 0, 0));

	if (newTreeNode)
	{

		if (GrowingWithDirection)
		{
			newTreeNode->CalculateCurrentDirection(GetActorLocation() + FVector::DownVector);
			if (newTreeNode->HasAttractionInfluences())
			{
				GrowingWithDirection = false;
			}
		}
		else
		{
			newTreeNode->CalculateCurrentDirection(GetActorLocation());
		}

		newTreeNode->CalculateNextTreeNodePosition(GrowingWithDirection, BranchLength);
		newTreeNode->parent = nullptr;
		growingNodeQueue.Add(newTreeNode);
	}
}

void ASpaceColonizationTreeSeed::CreateMesh()
{
	for (int i = 0; i < nodes.Num(); i++)
	{
		TArray<FVector> vertices;
		TArray<FVector2D> uvs;
		TArray<int32> triangles;
		UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, false, triangles);

		if (nodes[i]->parent)
		{
			float radius = pow(nodes[i]->parent->numOfChildren * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

			//Parent
			for (int s = 0; s < levelOfDetail + 1; s++)
			{
				FVector pos = FVector::Zero();

				FTransform rot = FTransform::Identity;

				rot.SetRotation(nodes[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());


				FTransform tf = FTransform::Identity;
				tf.AddToTranslation(nodes[i]->parent->GetTransform().GetTranslation());

				// radial angle of the vertex
				float alpha = ((float)s / levelOfDetail) * PI * 2.f;
				FTransform anotherOne = FTransform::Identity;
				anotherOne.AddToTranslation(FVector(FMath::Cos(alpha) * radius, FMath::Sin(alpha) * radius, 0));

				FTransform total = anotherOne * rot * tf;

				pos = total.TransformPosition(pos);
				pos -= GetActorLocation();

				vertices.Add(pos);
				FVector2D uv = FVector2D(s / levelOfDetail, 0);
				uvs.Add(uv);
			}

			radius = pow(nodes[i]->numOfChildren * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

			//This Node
			for (int s = 0; s < levelOfDetail + 1; s++)
			{
				FVector pos = FVector::Zero();

				FTransform rot = FTransform::Identity;
				rot.SetRotation(nodes[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());


				FTransform tf = FTransform::Identity;
				tf.AddToTranslation(nodes[i]->GetTransform().GetTranslation());

				// radial angle of the vertex
				float alpha = ((float)s / levelOfDetail) * PI * 2.f;
				FTransform anotherOne = FTransform::Identity;
				anotherOne.AddToTranslation(FVector(FMath::Cos(alpha) * radius, FMath::Sin(alpha) * radius, 0));

				FTransform total = anotherOne * rot * tf;

				pos = total.TransformPosition(pos);
				pos -= GetActorLocation();

				vertices.Add(pos);

				FVector2D uv = FVector2D(s / levelOfDetail, 1);
				uvs.Add(uv);
			}

			//Material
			MeshComponent->SetMaterial(MeshComponent->GetNumSections(), Material);

			//Create Section
			MeshComponent->CreateMeshSection(MeshComponent->GetNumSections(), vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

			vertices.Empty();
			uvs.Empty();
			triangles.Empty();

			int sphereDetail = levelOfDetail;
			UKismetProceduralMeshLibrary::CreateGridMeshTriangles(sphereDetail + 1, sphereDetail + 1, true, triangles);

			// Create Circle
			for (int m = sphereDetail + 1; m > 0; m--)
			{
				for (int n = 0; n < sphereDetail + 1; n++)
				{
					FVector pos = nodes[i]->GetActorLocation();
					pos.X = FMath::Sin(PI * m / sphereDetail) * FMath::Cos(2 * PI * n / sphereDetail) * (radius);
					pos.Y = FMath::Sin(PI * m / sphereDetail) * FMath::Sin(2 * PI * n / sphereDetail) * (radius);
					pos.Z = FMath::Cos(PI * m / sphereDetail) * (radius);
					pos = nodes[i]->GetActorTransform().TransformPosition(pos);
					pos -= GetActorLocation();

					vertices.Add(pos);
					uvs.Add(FVector2D(m / sphereDetail, n / sphereDetail));
				}
			}

			//Material
			MeshComponent->SetMaterial(MeshComponent->GetNumSections(), Material);

			//Create Section
			MeshComponent->CreateMeshSection(MeshComponent->GetNumSections(), vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

			vertices.Empty();
			uvs.Empty();
			triangles.Empty();
		}
	}
}

void ASpaceColonizationTreeSeed::GrowBranches(float DeltaTime)
{
	int maxGrowingCount = 2;
	bool allGrown = true;

	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, true, triangles);
	for (int i = 0; i<growingNodeQueue.Num() && i <maxGrowingCount; i++)
	{
		allGrown = false;
		if (ensure(growingNodeQueue[i]))
		{
			if (growingNodeQueue[i]->parent)
			{
				TArray<FVector> vertices;
				TArray<FVector2D> uvs;

				growingNodeQueue[i]->growingTimer += DeltaTime;
				float currentProgress = BranchLength * growingNodeQueue[i]->growingTimer / (1 / RateOfGrowth);

				if (currentProgress < BranchLength)
				{
					float radius = pow(growingNodeQueue[i]->numOfChildren * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;					
					//This Node
					for (int s = 0; s < levelOfDetail + 1; s++)
					{
						FVector pos = FVector::Zero();

						FTransform rot = FTransform::Identity;
						rot.SetRotation(growingNodeQueue[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());


						FTransform tf = FTransform::Identity;
						FVector translation = (growingNodeQueue[i]->GetTransform().GetTranslation() - growingNodeQueue[i]->parent->GetTransform().GetTranslation()) * (BranchLength - currentProgress) / BranchLength;
						tf.AddToTranslation(growingNodeQueue[i]->GetTransform().GetTranslation() - translation);

						// radial angle of the vertex
						float alpha = ((float)s / levelOfDetail) * PI * 2.f;
						FTransform anotherOne = FTransform::Identity;
						anotherOne.AddToTranslation(FVector(FMath::Cos(alpha) * radius, FMath::Sin(alpha) * radius, 0));

						FTransform total = anotherOne * rot * tf;

						pos = total.TransformPosition(pos);
						pos -= GetActorLocation();

						vertices.Add(pos);

						FVector2D uv = FVector2D(s / levelOfDetail, 1);
						uvs.Add(uv);
					}

					radius = pow(growingNodeQueue[i]->parent->numOfChildren * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

					//Parent
					for (int s = 0; s < levelOfDetail + 1; s++)
					{
						FVector pos = FVector::Zero();

						FTransform rot = FTransform::Identity;
						rot.SetRotation(growingNodeQueue[i]->GetCurrentDirection().ToOrientationRotator().Add(90,0,growingNodeQueue[i]->parent->GetCurrentDirection().ToOrientationRotator().Roll).Quaternion());


						FTransform tf = FTransform::Identity;
						tf.AddToTranslation(growingNodeQueue[i]->parent->GetTransform().GetTranslation());

						// radial angle of the vertex
						float alpha = ((float)s / levelOfDetail) * PI * 2.f;
						FTransform anotherOne = FTransform::Identity;
						anotherOne.AddToTranslation(FVector(FMath::Cos(alpha) * radius, FMath::Sin(alpha) * radius, 0));

						FTransform total = anotherOne * rot * tf;

						pos = total.TransformPosition(pos);
						pos -= GetActorLocation();

						vertices.Add(pos);
						FVector2D uv = FVector2D(s / levelOfDetail, 0);
						uvs.Add(uv);
					}

					//Create Section
					//Material
					MeshComponent->SetMaterial(MeshComponent->GetNumSections(), Material);
					MeshComponent->CreateMeshSection(MeshComponent->GetNumSections(), vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
					vertices.Empty();
					uvs.Empty();
				}
				else
				{
					nodes.Add(growingNodeQueue[i]);
					growingNodeQueue.RemoveAt(i);
				}
			}
			else
			{
				nodes.Add(growingNodeQueue[i]);
				growingNodeQueue.RemoveAt(i);
			}
		}
	}

	if(allGrown)
	{
		growingNodeQueue.Empty();
	}

}

// Called every frame
void ASpaceColonizationTreeSeed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TimeOfGrowth -= DeltaTime;
	if (TimeOfGrowth >= 0)
	{
		MeshComponent->ClearAllMeshSections();
		if (newNodeQueue.Num() > 0)
		{
			CreateNewNodes();
		}
		else
		{
			if (growingNodeQueue.Num() == 0)
			{
				QueueNewTreeNodes();
			}
			else
			{
				GrowBranches(DeltaTime);
			}
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

