#include "ShortestPathTreeSeed.h"
#include "KismetProceduralMeshLibrary.h"

//Constructor
AShortestPathTreeSeed::AShortestPathTreeSeed():
trunkBuild(false),
trunkNodesGenerated(0),
NumberOfEndpoints(70),
growingBranchesGenerated(0)
{
	//Component Initializations
	SeedSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SeedSceneComponent);

	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
}


///////////Functions///////////

//Reset the A Star algorithm
void AShortestPathTreeSeed::ResetAStar()
{
	//Reset Info of all visited nodes
	for(auto node : visitedVectorNodes)
	{
		node->ResetInfo();
	}
	//Empty visited node array
	visitedVectorNodes.Empty();

	//Reset info of all unvisited nodes
	for (auto node : unvisitedVectorNodes)
	{
		node->ResetInfo();
	}
	//Empty unvisited node array
	unvisitedVectorNodes.Empty();

	//Remove the completed endpoint from the array
	endPointIndexes.RemoveAtSwap(0);

	//If there are more endpoints
	if(!endPointIndexes.IsEmpty())
	{
		//Kickstart next cycle
		guidingVectorNodes[endPointIndexes[0]]->CalculateFGScores(guidingVectorNodes[0]->GetActorLocation());
		unvisitedVectorNodes.Add(guidingVectorNodes[endPointIndexes[0]]);
	}
}

//One step of A Star algorithm
bool AShortestPathTreeSeed::StepAStarAlgorithm()
{
	//If there are unvisited nodes
	if (!unvisitedVectorNodes.IsEmpty())
	{
		//The current node becomes the first unvisited node
		AGuidingVectorNode* currentNode = unvisitedVectorNodes[0];

		//For all unvisited nodes
		for (int i = 0; i < unvisitedVectorNodes.Num(); i++)
		{
			if (currentNode)
			{
				//Find the unvisited node with the list F score
				if (currentNode->GetFScore() >= unvisitedVectorNodes[i]->GetFScore())
				{
					//Choose that one as the current node
					currentNode = unvisitedVectorNodes[i];
				}
			}
		}

		if (currentNode)
		{
			//Add the current node to the visited array
			visitedVectorNodes.Add(currentNode);

			//Remove it from the unvisited array
			unvisitedVectorNodes.RemoveSwap(currentNode);

			//If the current node is the goal node
			if (currentNode == guidingVectorNodes[0])
			{
				//The shortest path is found
				//Create a new branch
				FBranch* newBranch = new FBranch;

				//While the current node is valid
				while (currentNode)
				{
					//Add current node to the branch
					newBranch->nodes.Add(currentNode);

					//If there is a previous node
					if (currentNode->GetPrevious())
					{
						//If the previous node doesn't have a next node
						if (!currentNode->GetPrevious()->GetNext())
						{
							//Set this node as the previous node's next node
							currentNode->GetPrevious()->SetNext(currentNode);
						}
					}

					//The current node becomes the previous node
					currentNode = currentNode->GetPrevious();
				}				

				//If there is no trunk yet
				if (!trunk)
				{
					//This branch is the trunk
					for(int i =0; i<newBranch->nodes.Num();i++)
					{
						//Reverse previous and next nodes so that it grows from the ground up
						if (newBranch->nodes[i]->GetPrevious())
						{
							newBranch->nodes[i]->SetNext(newBranch->nodes[i]->GetPrevious());
						}
						else
						{
							newBranch->nodes[i]->SetNext(nullptr);
						}
					}
					
					trunk = newBranch;
				}
				else
				{
					//Add this branch to growing branches
					growingTreeNodes.Add(newBranch);
				}

				//The algorithm us complete
				return true;
			}

			//Path not found yet
			TArray<AGuidingVectorNode*> currentNodeConnections = currentNode->GetConnections();

			//For all connected nodes of the current node
			for (int j = 0; j < currentNodeConnections.Num(); j++)
			{
				//If node is not visited already
				if (visitedVectorNodes.Find(currentNodeConnections[j]) == INDEX_NONE)
				{
					//Set current node as previous
					currentNodeConnections[j]->SetPrevious(currentNode);

					//Calculate F and G scores
					currentNodeConnections[j]->CalculateFGScores(guidingVectorNodes[0]->GetActorLocation());

					//If the node is not already in the unvisited array
					if (unvisitedVectorNodes.Find(currentNodeConnections[j]) == INDEX_NONE)
					{
						//Add it
						unvisitedVectorNodes.Add(currentNodeConnections[j]);
					}
				}

			}

		}
	}

	//Path not found yet
	return false;
}

//Tick Function, called every frame
void AShortestPathTreeSeed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If there are endpoints
	if (!endPointIndexes.IsEmpty())
	{
		//Step in A Star
		if (StepAStarAlgorithm())
		{
			//If a star is completed, reset
			ResetAStar();
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("Done"));
	}
		//If the trunk is built
		if (trunkBuild)
		{
			//If new branches are complete
			if (newBranchesGenerated<=0)
			{
				GrowBranches(DeltaTime);
			}
		}
		else
		{
			//Growing trunk
			GrowTrunk(DeltaTime);
		}

		if (newBranchesGenerated>0)
		{
			//Update Mesh
			CreateMesh();
		}
	
}

//When the application begins
void AShortestPathTreeSeed::BeginPlay()
{
	Super::BeginPlay();

	//Apply environment
	ApplyEnvironment();
	
	//Calculate First Guiding Node's Position
	FVector centreGuidingVectorPosition = FVector::Zero();
	
	FTransform translations = FTransform::Identity;
	translations.AddToTranslation(GetActorLocation());
	translations.AddToTranslation(FVector(0, 0, trunkHeight));
	centreGuidingVectorPosition = translations.TransformPosition(centreGuidingVectorPosition);

	//Spawn Guiding Vector
	SpawnGuidingVector(centreGuidingVectorPosition);
	guidingVectorNodes[0]->SetParent(GetActorLocation());

	//Spawn Trunk nodes
	CreateTrunk();

	//Spawn guiding vector nodes
	SpawnAllGuidingVectors();

	//For all guiding vector nodes
	for (int j = 0; j < guidingVectorNodes.Num(); j++)
	{
		//Detect connections
		guidingVectorNodes[j]->DetectConnections();
	}

	//Set parents to guiding vectors
	SetParentsGuidingVectors();

	//Choose random endpoints
	ChooseEndpoints();

	//Kick start algorithm 
	guidingVectorNodes[endPointIndexes[0]]->CalculateFGScores(guidingVectorNodes[0]->GetActorLocation());
	unvisitedVectorNodes.Add(guidingVectorNodes[endPointIndexes[0]]);
}


///////////Nodes///////////

//Spawn one guiding vector
void AShortestPathTreeSeed::SpawnGuidingVector(FVector location)
{
	//Spawn guiding vector at location
	AGuidingVectorNode* newGuidingVectorNode = GetWorld()->SpawnActor<AGuidingVectorNode>(GuidingVectorNodeToSpawn, location, FRotator(0, 0, 0));
	guidingVectorNodes.Add(newGuidingVectorNode);
}

//Choose random endpoints
void AShortestPathTreeSeed::ChooseEndpoints()
{
	//For the required number of endpoints
	for (int i = 0; i < NumberOfEndpoints; i++)
	{
		//Choose a random guiding vector and turn it to an endpoint
		int randIndex = FMath::RandRange(1, guidingVectorNodes.Num() - 1);
		guidingVectorNodes[randIndex]->SetEndpoint();
		endPointIndexes.Add(randIndex);
	}
}

//Create trunk nodes
void AShortestPathTreeSeed::CreateTrunk()
{
	//Spawn first trunk node and make it an endpoint
	SpawnGuidingVector(GetActorLocation());
	guidingVectorNodes[1]->SetEndpoint();
	endPointIndexes.Add(1);

	//Spaw trunk nodes with rnadom offsets
	for(int i =0; i<trunkHeight;i+=trunkHeight/20)
	{
		FVector randomOffset = FMath::VRand()*10;
		FVector spawnLocation = GetActorLocation() + randomOffset + FVector(0, 0, i);
		SpawnGuidingVector(spawnLocation);
	}
}

//Spawn all guiding vectors
void AShortestPathTreeSeed::SpawnAllGuidingVectors()
{
	//Until the required amount of guiding vectors are spawned
	while(guidingVectorNodes.Num()<NumberOfGuidingVectors)
	{
		//Get a random position within a radius
		FVector pos;
		pos.X = FMath::FRandRange(-crownRadius, crownRadius);
		pos.Y = FMath::FRandRange(-crownRadius, crownRadius);
		pos.Z = FMath::FRandRange(-crownRadius, crownRadius);

		//If the position is in the sphere
		if (sqrt((pos.X * pos.X) + (pos.Y * pos.Y) + (pos.Z * pos.Z)) <= crownRadius)
		{
			//If the position is in the top semi-sphere
			if (pos.Z >= 0)
			{
				//Calculate guiding vector position
				FVector guidingVectorPosition = FVector::Zero();

				//Random position translation
				FTransform randomPosTF = FTransform::Identity;
				randomPosTF.AddToTranslation(FVector(pos));
				guidingVectorPosition = randomPosTF.TransformPosition(guidingVectorPosition);

				//Light rotation
				FTransform lightRotationTF = FTransform::Identity;
				lightRotationTF.SetRotation(lightRotation.Quaternion());
				guidingVectorPosition = lightRotationTF.TransformPosition(guidingVectorPosition);

				//Wind offset and trunk height translation
				FTransform windAndTrunkTF = FTransform::Identity;
				windAndTrunkTF.AddToTranslation(GetActorLocation() + windOffset);
				windAndTrunkTF.AddToTranslation(FVector(0, 0, trunkHeight));
				guidingVectorPosition = windAndTrunkTF.TransformPosition(guidingVectorPosition);

				//Spawn Guiding Vector
				SpawnGuidingVector(guidingVectorPosition);
			}
		}
	}
}

//Set Parents to all guiding vectors
void AShortestPathTreeSeed::SetParentsGuidingVectors()
{
	//For all guiding Vectors
	for (int i = 0; i < guidingVectorNodes.Num(); i++)
	{
		//If this guiding vector is not a parent
		if (!guidingVectorNodes[i]->GetIsParent())
		{
			//If this vector has a parent
			if (guidingVectorNodes[i]->GetHasParent())
			{
				//Set this as connections parent
				guidingVectorNodes[i]->SetThisAsConnectionsParent();
			}
		}
	}

	for (int i = 0; i < guidingVectorNodes.Num(); i++)
	{
		if (guidingVectorNodes[i]->GetHasParent())
		{
			guidingVectorNodes[i]->SetParent(guidingVectorNodes[0]->GetActorLocation());
		}
	}
}


///////////Mesh///////////

//Create Procedural Mesh
void AShortestPathTreeSeed::CreateMesh()
{
	//Vertex and UV arrays
	TArray<FVector> vertices;
	TArray<FVector2D> uvs;

	//Triangle indices
	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, false, triangles);

	//For the required nodes to render
	for (int i = finalTreeNodeActors.Num() - 1 - renderedNodeMeshes; i >= finalTreeNodeActors.Num() - 1 - NodeMeshesRenderedPerFrame - renderedNodeMeshes && i >= 0; i--)
	{
		//If there is a next node in path
		if (finalTreeNodeActors[i]->GetNext())
		{
			//Delete growing mesh section
			if (i > finalTreeNodeActors.Num() - 1 - newBranchesGenerated)
			{
				int currentMeshSection = finalTreeNodeActors[i]->GetMeshSectionIndex();
				//If this node was rendered before
				if (currentMeshSection >= 0)
				{
					//Clear previous mesh section
					MeshComponent->ClearMeshSection(currentMeshSection);
					finalTreeNodeActors[i]->SetMeshSectionIndex(-1);
				}
			}


			int currentMeshSection = finalTreeNodeActors[i]->GetMeshSectionIndex();
			//If this node was rendered before
			if (currentMeshSection >= 0)
			{
				//Clear previous mesh section
				MeshComponent->ClearMeshSection(currentMeshSection);
			}
			else
			{
				//Assing a new mesh section to node
				currentMeshSection = MeshComponent->GetNumSections();
				finalTreeNodeActors[i]->SetMeshSectionIndex(currentMeshSection);
			}

			//Calculate radius for next node's mesh based on children
			float radius = pow(finalTreeNodeActors[i]->GetNext()->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

			//Generate next node's mesh vertices
			CalculateNodeMeshVerticesAndUV(radius,
				finalTreeNodeActors[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
				finalTreeNodeActors[i]->GetNext()->GetTransform().GetTranslation(),
				vertices,
				uvs);

			//Calculate radius for node's mesh based on children
			radius = pow(finalTreeNodeActors[i]->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

			//Generate node's mesh vertices
			CalculateNodeMeshVerticesAndUV(radius,
				finalTreeNodeActors[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
				finalTreeNodeActors[i]->GetTransform().GetTranslation(),
				vertices,
				uvs);

			//Set Material to section
			MeshComponent->SetMaterial(currentMeshSection, Material);

			//Create Node's Mesh Section
			MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

			vertices.Empty();
			uvs.Empty();
		}

		//Create a sphere mesh
		CreateSphereMesh(finalTreeNodeActors[i]);
	}

	renderedNodeMeshes += NodeMeshesRenderedPerFrame;

	//If all node's mesh sections were rendered
	if (renderedNodeMeshes >= finalTreeNodeActors.Num())
	{
		//Start from the beginning
		renderedNodeMeshes = 0;
		newBranchesGenerated = 0;
	}

}

//Slowly grow trunk mesh
void AShortestPathTreeSeed::GrowTrunk(float DeltaTime)
{
	//If the trunk nodes are generated
	if (trunk)
	{
		//Vertex and UV arrays for procedural mesh
		TArray<FVector> vertices;
		TArray<FVector2D> uvs;

		//Triangle indices
		TArray<int32> triangles;
		UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, false, triangles);

		//For all trunk nodes
		for (int i = trunk->nodes.Num() - 1; i >= 0; i--)
		{
			//If the node is not already generated
			if (finalTreeNodeActors.Find(trunk->nodes[i]) == INDEX_NONE)
			{
				//If there is a next node
				if (trunk->nodes[i]->GetNext())
				{
					//Calculate the branch's length
					float BranchLength = FVector::Dist(trunk->nodes[i]->GetActorLocation(), trunk->nodes[i]->GetNext()->GetActorLocation());

					//Add to timer
					trunk->nodes[i]->AddToGrowingTimer(DeltaTime);

					//Calculate progress based on time
					float currentProgress = BranchLength * trunk->nodes[i]->GetGrowingTimer() / (1 / RateOfGrowth);

					//If the branch is not fully developed yet
					if (currentProgress < BranchLength)
					{
						int currentMeshSection = trunk->nodes[i]->GetMeshSectionIndex();

						//If this node's mesh was previously rendered
						if (currentMeshSection >= 0)
						{
							//Clear the mesh section
							MeshComponent->ClearMeshSection(currentMeshSection);
						}
						else
						{
							//Assign a new mesh section to the node
							currentMeshSection = MeshComponent->GetNumSections();
							trunk->nodes[i]->SetMeshSectionIndex(currentMeshSection);
						}

						//Calculate the next node's mesh radius based on children
						float radius = pow(trunk->nodes[i]->GetNext()->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

						//Generate next node's mesh vertices
						CalculateNodeMeshVerticesAndUV(radius,
							trunk->nodes[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
							trunk->nodes[i]->GetNext()->GetTransform().GetTranslation(),
							vertices,
							uvs);

						//Calculate this node;s mesh radius based on children
						radius = pow(trunk->nodes[i]->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

						//Generate this node's mesh vertices
						FVector translation = trunk->nodes[i]->GetTransform().GetTranslation()-(trunk->nodes[i]->GetTransform().GetTranslation() - trunk->nodes[i]->GetNext()->GetTransform().GetTranslation()) * (BranchLength - currentProgress) / BranchLength;
						CalculateNodeMeshVerticesAndUV(radius,
							trunk->nodes[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
							translation,
							vertices,
							uvs);

						//Set Material to Section
						MeshComponent->SetMaterial(currentMeshSection, Material);

						//Create the Mesh Section
						MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

						//Clear arrays
						vertices.Empty();
						uvs.Empty();

						break;
					}

					//Reset node's mesh section
					//trunk->nodes[i]->SetMeshSectionIndex(-1);

					//Increment Children count
					trunk->nodes[i]->IncrementChildrenCount();

					//Add to the final mesh
					finalTreeNodeActors.Add(trunk->nodes[i]);
					trunkNodesGenerated++;
					newBranchesGenerated++;
				}
				else
				{
					//Add to the final mesh
					finalTreeNodeActors.Add(trunk->nodes[i]);
					trunkNodesGenerated++;
					newBranchesGenerated++;
				}
			}
		}

		//If all trunk meshes are generated
		if (trunkNodesGenerated >= trunk->nodes.Num())
		{
			trunkBuild = true;
		}
	}
}

//Slowly grow branches mesh
void AShortestPathTreeSeed::GrowBranches(float DeltaTime)
{
	//Vertex and UV arrays for procedural mesh
	TArray<FVector> vertices;
	TArray<FVector2D> uvs;

	//Triangle indices
	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, false, triangles);

	int currentGrownBranches = growingBranchesGenerated;

	//For the required amount of grown branches growing branches
	for (int i = currentGrownBranches; i < growingTreeNodes.Num() && i <= GrowingNodeMeshesRenderedPerFrame + currentGrownBranches; i++)
	{
		//For all nodes in this branch
		for (int j = 0; j < growingTreeNodes[i]->nodes.Num(); j++)
		{
			//If the node is not rendered already
			if (finalTreeNodeActors.Find(growingTreeNodes[i]->nodes[j]) == INDEX_NONE )
			{
				//If the node has a next node
				if (growingTreeNodes[i]->nodes[j]->GetNext())
				{
					//If the next node is not rendered yet
					if (finalTreeNodeActors.Find(growingTreeNodes[i]->nodes[j]->GetNext()) != INDEX_NONE)
					{
						if (growingTreeNodes[i]->nodes[j]->GetNext()->GetMeshSectionIndex() >= 0)
						{
							//Calculate branch length
							float BranchLength = FVector::Dist(growingTreeNodes[i]->nodes[j]->GetActorLocation(), growingTreeNodes[i]->nodes[j]->GetNext()->GetActorLocation());

							//Increment growing timer
							growingTreeNodes[i]->nodes[j]->AddToGrowingTimer(DeltaTime);

							//Calculate progress using rate of growth and time
							float currentProgress = BranchLength * growingTreeNodes[i]->nodes[j]->GetGrowingTimer() / (1 / RateOfGrowth);

							//If the branch is not fully grown
							if (currentProgress < BranchLength)
							{
								int currentMeshSection = growingTreeNodes[i]->nodes[j]->GetMeshSectionIndex();
								//If this node's mesh wa rendered before
								if (currentMeshSection >= 0)
								{
									//Clear mesh section
									MeshComponent->ClearMeshSection(currentMeshSection);
								}
								else
								{
									//Assign new mesh section index
									currentMeshSection = MeshComponent->GetNumSections();
									growingTreeNodes[i]->nodes[j]->SetMeshSectionIndex(currentMeshSection);
								}
								//Calculate the next node's mesh radius based on children
								float radius = pow(growingTreeNodes[i]->nodes[j]->GetNext()->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

								//Generate next node's mesh vertices
								CalculateNodeMeshVerticesAndUV(radius,
									growingTreeNodes[i]->nodes[j]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
									growingTreeNodes[i]->nodes[j]->GetNext()->GetTransform().GetTranslation(),
									vertices,
									uvs);

								//Calculate this node;s mesh radius based on children
								radius = pow(growingTreeNodes[i]->nodes[j]->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

								//Generate this node's mesh vertices
								FVector translation = (growingTreeNodes[i]->nodes[j]->GetTransform().GetTranslation() - growingTreeNodes[i]->nodes[j]->GetNext()->GetTransform().GetTranslation()) * (BranchLength - currentProgress) / BranchLength;
								CalculateNodeMeshVerticesAndUV(radius,
									growingTreeNodes[i]->nodes[j]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
									growingTreeNodes[i]->nodes[j]->GetTransform().GetTranslation() - translation,
									vertices,
									uvs);

								//Set Material to Section
								MeshComponent->SetMaterial(currentMeshSection, Material);

								//Create the Mesh Section
								MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

								//Clear arrays
								vertices.Empty();
								uvs.Empty();

								break;
							}

							//Increment node's Children count
							growingTreeNodes[i]->nodes[j]->IncrementChildrenCount();

							//Add this node to the final mesh
							finalTreeNodeActors.Add(growingTreeNodes[i]->nodes[j]);

							//Remove from growing nodes
							growingTreeNodes[i]->nodes.RemoveAtSwap(j);

							newBranchesGenerated++;
							growingBranchesGenerated++;
						}
					}
				}
				else
				{
					//Add this node to the final mesh
					finalTreeNodeActors.Add(growingTreeNodes[i]->nodes[j]);

					//Remove from growing node
					growingTreeNodes[i]->nodes.RemoveAtSwap(j);
					newBranchesGenerated++;
					growingBranchesGenerated++;
				}
			}
			else
			{
				//Remove from growing node
				growingTreeNodes[i]->nodes.RemoveAtSwap(j);
			}
		}
	}
}

//Create a Spherical mesh section
void AShortestPathTreeSeed::CreateSphereMesh(AGuidingVectorNode* node)
{
	//Vertex and UV arrays
	TArray<FVector> vertices;
	TArray<FVector2D> uvs;

	//Triangle indices
	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(levelOfDetail + 1, levelOfDetail + 1, true, triangles);

	//Calculate sphere radius based on node's children count
	float radius = pow(node->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

	int currentMeshSection = node->GetMeshSectionIndex();
	//If this node's mesh was rendered before
	if (currentMeshSection >= 0)
	{
		//Clear the sphere mesh
		currentMeshSection++;
		MeshComponent->ClearMeshSection(currentMeshSection);
	}
	else
	{
		//Assign a new mesh section to the node
		currentMeshSection = MeshComponent->GetNumSections();
		node->SetMeshSectionIndex(currentMeshSection);
	}

	// Create Sphere
	for (int m = levelOfDetail + 1; m > 0; m--)
	{
		for (int n = 0; n < levelOfDetail + 1; n++)
		{
			//Calculate sphere vertex positions
			FVector pos = node->GetActorLocation();
			pos.X = FMath::Sin(PI * m / levelOfDetail) * FMath::Cos(2 * PI * n / levelOfDetail) * (radius);
			pos.Y = FMath::Sin(PI * m / levelOfDetail) * FMath::Sin(2 * PI * n / levelOfDetail) * (radius);
			pos.Z = FMath::Cos(PI * m / levelOfDetail) * (radius);
			pos = node->GetActorTransform().TransformPosition(pos);
			pos -= GetActorLocation();

			vertices.Add(pos);

			//Calculate UV positions
			uvs.Add(FVector2D(m / levelOfDetail, n / levelOfDetail));
		}
	}

	//Set Material to Sphere
	MeshComponent->SetMaterial(currentMeshSection, Material);

	//Create Section for Sphere
	MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
}






