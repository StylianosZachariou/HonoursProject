// Fill out your copyright notice in the Description page of Project Settings.


#include "ShortestPathTreeSeed.h"

#include "EnvironmentSettings.h"
#include "KismetProceduralMeshLibrary.h"


AShortestPathTreeSeed::AShortestPathTreeSeed()
{
	SeedSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SeedSceneComponent);

	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
}

void AShortestPathTreeSeed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!endPointIndexes.IsEmpty())
	{
		if (!endPointIndexes.IsEmpty())
		{
			if (StepAStarAlgorithm())
			{
				ResetAStar();
			}
		}

		if (trunkBuild)
		{
			GrowBranches(DeltaTime);
		}
		else
		{
			GrowTrunk(DeltaTime);
		}
		CreateMesh();
	}
}

void AShortestPathTreeSeed::BeginPlay()
{
	Super::BeginPlay();

	ApplyEnvironment();
	//First Guiding Node
	FVector centrePosition = FVector(0, 0, 0);

	FVector centreGuidingVectorPosition = FVector::Zero();

	FTransform translations = FTransform::Identity;
	translations.AddToTranslation(FVector(centrePosition));
	centreGuidingVectorPosition = translations.TransformPosition(centreGuidingVectorPosition);

	FTransform translations1 = FTransform::Identity;
	translations1.AddToTranslation(GetActorLocation());
	translations1.AddToTranslation(FVector(0, 0, trunkHeight));
	centreGuidingVectorPosition = translations1.TransformPosition(centreGuidingVectorPosition);
	SpawnGuidingVector(centreGuidingVectorPosition);
	guidingVectorNodes[0]->SetParent(GetActorLocation());

	CreateTrunk();
	SpawnAllGuidingVectors();

	for (int j = 0; j < guidingVectorNodes.Num(); j++)
	{
		guidingVectorNodes[j]->detectConnections();
	}

	SetParentsGuidingVectors();
	ChooseEndpoints();

	guidingVectorNodes[endPointIndexes[0]]->CalculateFGScores(guidingVectorNodes[0]->GetActorLocation());
	unvisitedVectorNodes.Add(guidingVectorNodes[endPointIndexes[0]]);
}

void AShortestPathTreeSeed::SpawnGuidingVector(FVector location)
{
	AGuidingVectorNode* newGuidingVectorNode = GetWorld()->SpawnActor<AGuidingVectorNode>(GuidingVectorNodeToSpawn, location, FRotator(0, 0, 0));
	guidingVectorNodes.Add(newGuidingVectorNode);
}

void AShortestPathTreeSeed::ChooseEndpoints()
{
	for (int i = 0; i < NumberOfEndpoints; i++)
	{
		int randIndex = FMath::RandRange(1, guidingVectorNodes.Num() - 1);
		guidingVectorNodes[randIndex]->SetEndpoint();
		endPointIndexes.Add(randIndex);
	}
}

void AShortestPathTreeSeed::CreateTrunk()
{
	//First Node
	SpawnGuidingVector(GetActorLocation());
	guidingVectorNodes[1]->SetEndpoint();
	endPointIndexes.Add(1);

	for(int i =0; i<trunkHeight;i+=trunkHeight/5)
	{
		FVector randomOffset = FMath::VRand()*10;
		FVector spawnLocation = GetActorLocation() + randomOffset + FVector(0, 0, i);
		SpawnGuidingVector(spawnLocation);
	}
}

void AShortestPathTreeSeed::CreateMesh()
{
	if (renderedNodeMeshes >= finalTreeNodeActors.Num())
	{
		renderedNodeMeshes = 0;
	}

	TArray<FVector> vertices;
	TArray<FVector2D> uvs;
	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, false, triangles);

	for (int i = renderedNodeMeshes; i < NodeMeshesRenderedPerFrame + renderedNodeMeshes && i < finalTreeNodeActors.Num(); i++)
	{
		float radius;

		if (finalTreeNodeActors[i]->GetNext())
		{
			int currentMeshSection = finalTreeNodeActors[i]->GetMeshSectionIndex();
			if (currentMeshSection >= 0)
			{
				MeshComponent->ClearMeshSection(currentMeshSection);
			}
			else
			{
				currentMeshSection = MeshComponent->GetNumSections();
				finalTreeNodeActors[i]->SetMeshSectionIndex(currentMeshSection);
			}

			radius = pow(finalTreeNodeActors[i]->GetNext()->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

			//Parent
			for (int s = 0; s < levelOfDetail + 1; s++)
			{
				FVector pos = FVector::Zero();

				FTransform rot = FTransform::Identity;

				rot.SetRotation(finalTreeNodeActors[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());


				FTransform tf = FTransform::Identity;
				tf.AddToTranslation(finalTreeNodeActors[i]->GetNext()->GetTransform().GetTranslation());

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

			radius = pow(finalTreeNodeActors[i]->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;
			//This Node
			for (int s = 0; s < levelOfDetail + 1; s++)
			{
				FVector pos = FVector::Zero();

				FTransform rot = FTransform::Identity;
				rot.SetRotation(finalTreeNodeActors[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());


				FTransform tf = FTransform::Identity;
				tf.AddToTranslation(finalTreeNodeActors[i]->GetTransform().GetTranslation());

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
			MeshComponent->SetMaterial(currentMeshSection, Material);

			//Create Section
			MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

			vertices.Empty();
			uvs.Empty();
		}

		radius = pow(finalTreeNodeActors[i]->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;
		CreateSphereMesh(finalTreeNodeActors[i], radius);
		
		renderedNodeMeshes++;
	}
}

void AShortestPathTreeSeed::GrowTrunk(float DeltaTime)
{
	if (trunk)
	{
		TArray<int32> triangles;
		UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, false, triangles);
		for (int i = trunk->nodes.Num() - 1; i >= 0; i--)
		{
			TArray<FVector> vertices;
			TArray<FVector2D> uvs;
			if (finalTreeNodeActors.Find(trunk->nodes[i]) == INDEX_NONE)
			{

				if (trunk->nodes[i]->GetNext())
				{
					int currentMeshSection = trunk->nodes[i]->GetMeshSectionIndex();
					if (currentMeshSection >= 0)
					{
						MeshComponent->ClearMeshSection(currentMeshSection);
					}
					else
					{
						currentMeshSection = MeshComponent->GetNumSections();
						trunk->nodes[i]->SetMeshSectionIndex(currentMeshSection);
					}
					float BranchLength = FVector::Dist(trunk->nodes[i]->GetActorLocation(), trunk->nodes[i]->GetNext()->GetActorLocation());
					trunk->nodes[i]->AddToGrowingTimer(DeltaTime);
					float currentProgress = BranchLength * trunk->nodes[i]->GetGrowingTimer() / (1 / RateOfGrowth);

					float radius;
					if (currentProgress < BranchLength)
					{

						radius = pow(trunk->nodes[i]->GetNext()->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

						//Parent
						for (int s = 0; s < levelOfDetail + 1; s++)
						{
							FVector pos = FVector::Zero();

							FTransform rot = FTransform::Identity;

							rot.SetRotation(trunk->nodes[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());


							FTransform tf = FTransform::Identity;
							tf.AddToTranslation(trunk->nodes[i]->GetNext()->GetTransform().GetTranslation());

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

						radius = pow(trunk->nodes[i]->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

						//This Node
						for (int s = 0; s < levelOfDetail + 1; s++)
						{
							FVector pos = FVector::Zero();

							FTransform rot = FTransform::Identity;
							rot.SetRotation(trunk->nodes[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());

							FTransform tf = FTransform::Identity;
							FVector translation = (trunk->nodes[i]->GetTransform().GetTranslation() - trunk->nodes[i]->GetNext()->GetTransform().GetTranslation()) * (BranchLength - currentProgress) / BranchLength;
							tf.AddToTranslation(trunk->nodes[i]->GetTransform().GetTranslation() - translation);

							// radial angle of the vertex
							float alpha = (float)s / levelOfDetail * PI * 2.f;
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
						MeshComponent->SetMaterial(currentMeshSection, Material);

						//Create Section
						MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

						vertices.Empty();
						uvs.Empty();
						break;
					}

					trunk->nodes[i]->SetMeshSectionIndex(-1);
					trunk->nodes[i]->IncrementChildrenCount();
					finalTreeNodeActors.Add(trunk->nodes[i]);
					trunkNodesGenerated++;

				}
				else
				{
					finalTreeNodeActors.Add(trunk->nodes[i]);
					trunkNodesGenerated++;

				}
			}
		}

		if (trunkNodesGenerated >= trunk->nodes.Num())
		{
			trunkBuild = true;
		}
	}
}

void AShortestPathTreeSeed::CreateSphereMesh(AGuidingVectorNode* node, float radius)
{

	TArray<FVector> vertices;
	TArray<FVector2D> uvs;
	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(levelOfDetail + 1, levelOfDetail + 1, true, triangles);

	int currentMeshSection = node->GetMeshSectionIndex();
	if (currentMeshSection >= 0)
	{
		currentMeshSection++;
		MeshComponent->ClearMeshSection(currentMeshSection);
	}
	else
	{
		currentMeshSection = MeshComponent->GetNumSections();
		node->SetMeshSectionIndex(currentMeshSection);
	}

	// Create Circle
	for (int m = levelOfDetail + 1; m > 0; m--)
	{
		for (int n = 0; n < levelOfDetail + 1; n++)
		{
			FVector pos = node->GetActorLocation();
			pos.X = FMath::Sin(PI * m / levelOfDetail) * FMath::Cos(2 * PI * n / levelOfDetail) * (radius);
			pos.Y = FMath::Sin(PI * m / levelOfDetail) * FMath::Sin(2 * PI * n / levelOfDetail) * (radius);
			pos.Z = FMath::Cos(PI * m / levelOfDetail) * (radius);
			pos = node->GetActorTransform().TransformPosition(pos);
			pos -= GetActorLocation();

			vertices.Add(pos);
			uvs.Add(FVector2D(m / levelOfDetail, n / levelOfDetail));
		}
	}


	//Material
	MeshComponent->SetMaterial(currentMeshSection, Material);

	//Create Section
	MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

	vertices.Empty();
	uvs.Empty();
	triangles.Empty();
}

void AShortestPathTreeSeed::ApplyEnvironment()
{
	crownRadius = 350;
	trunkHeight = 150;

	AEnvironmentSettings* environment = Cast<AEnvironmentSettings>(GetWorldSettings());

	float maxWindPower = 75;
	float windPower = FMath::Min(environment->GetWindPower(), maxWindPower);
	//Wind
	//If the wind is strong
	if (windPower > 25)
	{
		FVector direction = environment->GetWindDirection();
		direction.Normalize();
		windOffset = direction * ((windPower * crownRadius) / maxWindPower);
	}

	FVector crownStartPosition = GetActorLocation();
	crownStartPosition.Z += trunkHeight;

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
	if (moisturePercentage > 45)
	{
		RateOfGrowth -= FMath::Abs(45 - moisturePercentage) / 25;
	}
	else if (moisturePercentage < 20)
	{
		RateOfGrowth -= FMath::Abs(20 - moisturePercentage) / 10;
	}

	//Soil Acidity
	float soilPH = environment->GetSoilAcidity();
	if (soilPH > 8)
	{
		RateOfGrowth -= FMath::Abs(8 - soilPH) / 3;
	}
	else if (soilPH < 4.5)
	{
		RateOfGrowth -= FMath::Abs(4.5 - soilPH) / 2;
	}

	//Temperature
	float temperatureCelcius = environment->GetTemperature();
	if (temperatureCelcius > 30)
	{
		RateOfGrowth -= FMath::Abs(30 - temperatureCelcius) / 11;
	}
	else if (temperatureCelcius < 25)
	{
		RateOfGrowth -= FMath::Abs(25 - temperatureCelcius) / 21;
	}

	RateOfGrowth = FMath::Max(0, RateOfGrowth);
}

void AShortestPathTreeSeed::GrowBranches(float DeltaTime)
{
	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, false, triangles);

	for (int i = 0; i < growingTreeNodes.Num(); i++)
	{
		for (int j = 0; j < growingTreeNodes[i]->nodes.Num(); j++)
		{
			TArray<FVector> vertices;
			TArray<FVector2D> uvs;
			if (finalTreeNodeActors.Find(growingTreeNodes[i]->nodes[j]) == INDEX_NONE )
			{
				if (growingTreeNodes[i]->nodes[j]->GetNext())
				{
					if (finalTreeNodeActors.Find(growingTreeNodes[i]->nodes[j]->GetNext()) != INDEX_NONE)
					{
						int currentMeshSection = growingTreeNodes[i]->nodes[j]->GetMeshSectionIndex();
						if (currentMeshSection >= 0)
						{
							MeshComponent->ClearMeshSection(currentMeshSection);
						}
						else
						{
							currentMeshSection = MeshComponent->GetNumSections();
							growingTreeNodes[i]->nodes[j]->SetMeshSectionIndex(currentMeshSection);
						}

						float BranchLength = FVector::Dist(growingTreeNodes[i]->nodes[j]->GetActorLocation(), growingTreeNodes[i]->nodes[j]->GetNext()->GetActorLocation());
						growingTreeNodes[i]->nodes[j]->AddToGrowingTimer(DeltaTime);
						float currentProgress = BranchLength * growingTreeNodes[i]->nodes[j]->GetGrowingTimer() / (1 / RateOfGrowth);

						float radius;
						if (currentProgress < BranchLength)
						{
							radius = pow(growingTreeNodes[i]->nodes[j]->GetNext()->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

							//Parent
							for (int s = 0; s < levelOfDetail + 1; s++)
							{
								FVector pos = FVector::Zero();

								FTransform rot = FTransform::Identity;

								rot.SetRotation(growingTreeNodes[i]->nodes[j]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());


								FTransform tf = FTransform::Identity;
								tf.AddToTranslation(growingTreeNodes[i]->nodes[j]->GetNext()->GetTransform().GetTranslation());

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

							radius = pow(growingTreeNodes[i]->nodes[j]->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

							//This Node
							for (int s = 0; s < levelOfDetail + 1; s++)
							{
								FVector pos = FVector::Zero();

								FTransform rot = FTransform::Identity;
								rot.SetRotation(growingTreeNodes[i]->nodes[j]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0).Quaternion());


								FTransform tf = FTransform::Identity;
								FVector translation = (growingTreeNodes[i]->nodes[j]->GetTransform().GetTranslation() - growingTreeNodes[i]->nodes[j]->GetNext()->GetTransform().GetTranslation()) * (BranchLength - currentProgress) / BranchLength;
								tf.AddToTranslation(growingTreeNodes[i]->nodes[j]->GetTransform().GetTranslation() - translation);

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
							MeshComponent->SetMaterial(currentMeshSection, Material);

							//Create Section
							MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

							break;
						}

						growingTreeNodes[i]->nodes[j]->SetMeshSectionIndex(-1);

						growingTreeNodes[i]->nodes[j]->IncrementChildrenCount();
						finalTreeNodeActors.Add(growingTreeNodes[i]->nodes[j]);

						growingTreeNodes[i]->nodes.RemoveAt(j);

						vertices.Empty();
						uvs.Empty();
					}
				}
				else
				{
					finalTreeNodeActors.Add(growingTreeNodes[i]->nodes[j]);
					growingTreeNodes[i]->nodes.RemoveAt(j);
				}
			}
			else
			{
				growingTreeNodes[i]->nodes.RemoveAt(j);
			}
		}
	}

	if (!growingTreeNodes.IsEmpty())
	{
		if (growingTreeNodes.Last()->nodes.IsEmpty())
		{
			growingTreeNodes.Pop();
		}
	}
}

void AShortestPathTreeSeed::ResetAStar()
{
	for(auto node : visitedVectorNodes)
	{
		node->ResetInfo();
	}
	visitedVectorNodes.Empty();

	for (auto node : unvisitedVectorNodes)
	{
		node->ResetInfo();
	}
	unvisitedVectorNodes.Empty();

	endPointIndexes.RemoveAtSwap(0);

	if(!endPointIndexes.IsEmpty())
	{
		guidingVectorNodes[endPointIndexes[0]]->CalculateFGScores(guidingVectorNodes[0]->GetActorLocation());
		unvisitedVectorNodes.Add(guidingVectorNodes[endPointIndexes[0]]);
	}
}

bool AShortestPathTreeSeed::StepAStarAlgorithm()
{
	if (!unvisitedVectorNodes.IsEmpty())
	{
		AGuidingVectorNode* currentNode = unvisitedVectorNodes[0];

		for (int i = 0; i < unvisitedVectorNodes.Num(); i++)
		{
			if (currentNode)
			{
				if (currentNode->GetFScore() >= unvisitedVectorNodes[i]->GetFScore())
				{
					currentNode = unvisitedVectorNodes[i];
				}
			}
		}

		if (currentNode)
		{
			visitedVectorNodes.Add(currentNode);
			unvisitedVectorNodes.RemoveSwap(currentNode);

			if (currentNode == guidingVectorNodes[0])
			{
				//Done
				FBranch* newBranch = new FBranch;
				while (currentNode)
				{
					newBranch->nodes.Add(currentNode);
					if (currentNode->GetPrevious())
					{
						if (!currentNode->GetPrevious()->GetNext())
						{
							currentNode->GetPrevious()->SetNext(currentNode);
						}
					}
					currentNode = currentNode->GetPrevious();
				}				

				if (!trunk)
				{
					for(int i =0; i<newBranch->nodes.Num();i++)
					{
						if (newBranch->nodes[i]->GetPrevious())
						{
							newBranch->nodes[i]->SetNext(newBranch->nodes[i]->GetPrevious());
						}
						else
						{
							newBranch->nodes[i]->SetNext(nullptr);
						}
					}
				}

				if(trunk)
				{
					growingTreeNodes.Add(newBranch);
				}
				else
				{
 					trunk = newBranch;
				}

				return true;
			}

			TArray<AGuidingVectorNode*> currentNodeConnections = currentNode->GetConnections();

			for (int j = 0; j < currentNodeConnections.Num(); j++)
			{
				if (visitedVectorNodes.Find(currentNodeConnections[j]) == INDEX_NONE)
				{
					currentNodeConnections[j]->SetPrevious(currentNode);
					currentNodeConnections[j]->CalculateFGScores(guidingVectorNodes[0]->GetActorLocation());

					if (unvisitedVectorNodes.Find(currentNodeConnections[j]) == INDEX_NONE)
					{
						unvisitedVectorNodes.Add(currentNodeConnections[j]);
					}
				}

			}

		}
	}
	return false;
}

void AShortestPathTreeSeed::SpawnAllGuidingVectors()
{
	while(guidingVectorNodes.Num()<NumberOfGuidingVectors)
	{
		FVector pos;
		pos.X = FMath::FRandRange(-crownRadius, crownRadius);
		pos.Y = FMath::FRandRange(-crownRadius, crownRadius);
		pos.Z = FMath::FRandRange(-crownRadius, crownRadius);

		if (sqrt((pos.X * pos.X) + (pos.Y * pos.Y) + (pos.Z * pos.Z)) <= crownRadius)
		{
			if (pos.Z >= 0)
			{
				FVector guidingVectorPosition = FVector::Zero();

				FTransform translation2 = FTransform::Identity;
				translation2.AddToTranslation(FVector(pos));
				guidingVectorPosition = translation2.TransformPosition(guidingVectorPosition);


				FTransform rot = FTransform::Identity;
				rot.SetRotation(lightRotation.Quaternion());

				guidingVectorPosition = rot.TransformPosition(guidingVectorPosition);

				FTransform translation = FTransform::Identity;
				translation.AddToTranslation(GetActorLocation() + windOffset);
				translation.AddToTranslation(FVector(0, 0, trunkHeight));
				guidingVectorPosition = translation.TransformPosition(guidingVectorPosition);

				SpawnGuidingVector(guidingVectorPosition);
			}
		}
	}
}

void AShortestPathTreeSeed::SetParentsGuidingVectors()
{
	for (int i = 0; i < guidingVectorNodes.Num(); i++)
	{
		if (!guidingVectorNodes[i]->GetIsParent())
		{
			if (guidingVectorNodes[i]->GetHasParent())
			{
				guidingVectorNodes[i]->SetThisAsConnectionsParent();
			}
		}
	}
}
