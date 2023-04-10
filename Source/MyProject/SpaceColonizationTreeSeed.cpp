#include "SpaceColonizationTreeSeed.h"
#include "AttractionNode.h"
#include "KismetProceduralMeshLibrary.h"
#include "TreeNode.h"

//Constructor
ASpaceColonizationTreeSeed::ASpaceColonizationTreeSeed():
NumOfAttractionPoints(2500),
BranchLength(20),
GrowingWithDirection(true)
{
	//Component Initializations
	SeedSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SeedSceneComponent);

	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
}


///////////Functions///////////

//When the application starts
void ASpaceColonizationTreeSeed::BeginPlay()
{
	Super::BeginPlay();

	//Apply environment to tree
	ApplyEnvironment();

	//Create all attraction points
	CreateAttractionPoints();

	//Spawn first node
	SpawnNewNode(nullptr);
}

//Called every frame
void ASpaceColonizationTreeSeed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If the tree is still growing
	TimeOfGrowth -= DeltaTime;
	if (TimeOfGrowth >= 0)
	{
		//If there are no queued nodes
		if (newNodeQueue.Num() > 0)
		{
			//Create new nodes
			CreateNewNodes();
		}
		else
		{
			//If there are no growing nodes
			if (growingNodeQueue.Num() == 0)
			{
				//Queue growing nodes
				QueueNewTreeNodes();
			}
			else
			{
				//Continue branch growth
				GrowBranches(DeltaTime);
			}
		}
		//Update Mesh
		CreateMesh();
	}
}


///////////Nodes///////////

// Create all attraction nodes
void ASpaceColonizationTreeSeed::CreateAttractionPoints()
{
	//Until all attraction points are spawned
	while (attractionPoints.Num() < NumOfAttractionPoints)
	{
		//Get a random position in radius
		FVector pos;
		pos.X = FMath::FRandRange(-crownRadius, crownRadius);
		pos.Y = FMath::FRandRange(-crownRadius, crownRadius);
		pos.Z = FMath::FRandRange(-crownRadius, crownRadius);

		//If position is in circle
		if (sqrt((pos.X * pos.X) + (pos.Y * pos.Y) + (pos.Z * pos.Z)) <= crownRadius)
		{
			//If position is in top semi-sphere
			if (pos.Z >= 0)
			{
				//Calculate attraction point position
				FVector attractionPointPosition = FVector::Zero();

				//Random position translation
				FTransform randomPosTF = FTransform::Identity;
				randomPosTF.AddToTranslation(FVector(pos));
				attractionPointPosition = randomPosTF.TransformPosition(attractionPointPosition);

				//Light rotation
				FTransform lightRotationTF = FTransform::Identity;
				lightRotationTF.SetRotation(lightRotation.Quaternion());
				attractionPointPosition = lightRotationTF.TransformPosition(attractionPointPosition);

				//Wind offset and trunk height translation
				FTransform windAndTrunkTF = FTransform::Identity;
				windAndTrunkTF.AddToTranslation(GetActorLocation() + windOffset);
				windAndTrunkTF.AddToTranslation(FVector(0, 0, trunkHeight));
				attractionPointPosition = windAndTrunkTF.TransformPosition(attractionPointPosition);

				//Spawn new attraction point
				SpawnNewAttractionNode(attractionPointPosition);
			}
		}
	}
}

//Spawn one attraction node
void ASpaceColonizationTreeSeed::SpawnNewAttractionNode(FVector location)
{
	//Spawn a new attraction node
	AAttractionNode* newAttractionNode = GetWorld()->SpawnActor<AAttractionNode>(AttractionNodeToSpawn, location, FRotator(0, 0, 0));
	attractionPoints.Add(newAttractionNode);
}

//Queue new tree nodes
void ASpaceColonizationTreeSeed::QueueNewTreeNodes()
{
	//For all nodes
	for (int i =0;i<nodes.Num();i++)
	{
		//If the node is active and has a next node position
		if (nodes[i]->GetIsActive() && nodes[i]->GetNextTreeNodePosition())
		{
			//Queue this node
			newNodeQueue.Add(nodes[i]);
		}
	}
}

//Create all new nodes
void ASpaceColonizationTreeSeed::CreateNewNodes()
{
	//Spawn three nodes
	for (int i = 0; i <= 2 && newNodeQueue.Num() > 0;i++)
	{
		SpawnNewNode(newNodeQueue.Pop());
	}
}

//Spawn one new node
void ASpaceColonizationTreeSeed::SpawnNewNode(ATreeNode* parentNode)
{
	FVector position;
	//If parent has a next node
	if (parentNode)
	{
		position = *parentNode->GetNextTreeNodePosition();
	}
	else
	{
		position = GetActorLocation();
	}

	//Spawn new tree node 
	ATreeNode* newTreeNode = GetWorld()->SpawnActor<ATreeNode>(TreeNodeToSpawn, position, FRotator(0, 0, 0));

	if (newTreeNode)
	{
		//If its growing with only direction
		if (GrowingWithDirection)
		{
			//Calculate direction to go up (for trunk)
			newTreeNode->CalculateCurrentDirection(position + FVector::DownVector);

			//If node has attraction influences 
			if (newTreeNode->GetHasAttractionInfluences())
			{
				//Stop growing with direction
				GrowingWithDirection = false;
			}
		}
		else
		{
			//Calculate direction using parent node's location
			newTreeNode->CalculateCurrentDirection(position);
		}

		//Calculate next node position
		newTreeNode->CalculateNextTreeNodePosition(GrowingWithDirection, BranchLength);

		//Add to the queue
		growingNodeQueue.Add(newTreeNode);

		if (parentNode)
		{
			//Set parent
			newTreeNode->SetParent(parentNode);

			//Parent node reset
			parentNode->ResetNextTreeNodePosition();
			parentNode->IncrementChildCount();
		}
	}
}


///////////Mesh///////////

//Create Mesh for all grown branches
void ASpaceColonizationTreeSeed::CreateMesh()
{
	//If all nodes are rendered
	if (renderedNodeMeshes >= nodes.Num())
	{
		//Start from the beginning
		renderedNodeMeshes = 0;
	}

	//Vertex and UV arrays
	TArray<FVector> vertices;
	TArray<FVector2D> uvs;

	//Mesh Triangle Indices
	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, false, triangles);

	//For the required nodes to render
	for (int i = renderedNodeMeshes; i <= NodeMeshesRenderedPerFrame + renderedNodeMeshes && i < nodes.Num(); i++)
	{
		//If this node has a parent
		if (nodes[i]->GetParent())
		{
			int currentMeshSection = nodes[i]->GetMeshSectionIndex();
			//If this nodes has been rendered before
			if (currentMeshSection >= 0)
			{
				//Clear that section
				MeshComponent->ClearMeshSection(currentMeshSection);
			}
			else
			{
				//Assign a new section number
				currentMeshSection = MeshComponent->GetNumSections();
				nodes[i]->SetMeshSectionIndex(currentMeshSection);
			}

			//Calculate radius of the parent node's mesh based on children
			float radius = pow(nodes[i]->GetParent()->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

			CalculateNodeMeshVerticesAndUV(radius,
				nodes[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
				nodes[i]->GetParent()->GetTransform().GetTranslation(),
				vertices,
				uvs);

			//Calculate radius for this node's mesh based on children
			radius = pow(nodes[i]->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

			CalculateNodeMeshVerticesAndUV(radius,
				nodes[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
				nodes[i]->GetTransform().GetTranslation(),
				vertices,
				uvs);

			//Set Material to mesh section
			MeshComponent->SetMaterial(currentMeshSection, Material);

			//Create Mesh Section
			MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

			//Create Sphere At Position
			CreateSphereMesh(nodes[i]);

			vertices.Empty();
			uvs.Empty();
		}
		renderedNodeMeshes++;
	}
//	renderedNodeMeshes += NodeMeshesRenderedPerFrame;
}

//Slowly grow branches' mesh
void ASpaceColonizationTreeSeed::GrowBranches(float DeltaTime)
{
	int maxGrowingCount = 2;
	bool allGrown = true;

	//Mesh Triangle Indices
	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(2, levelOfDetail + 1, true, triangles);

	//Vertex and UV arrays
	TArray<FVector> vertices;
	TArray<FVector2D> uvs;

	//For some of the growing nodes (depending on max count)
	for (int i = 0; i<growingNodeQueue.Num() && i <maxGrowingCount; i++)
	{
		//Its not all grown
		allGrown = false;
		if (ensure(growingNodeQueue[i]))
		{
			//If this node has a parent
			if (growingNodeQueue[i]->GetParent())
			{

				int currentMeshSection = growingNodeQueue[i]->GetMeshSectionIndex();

				//If this node was rendered before
				if (currentMeshSection >= 0)
				{
					//Clear node's mesh section
					MeshComponent->ClearMeshSection(currentMeshSection);
				}
				else
				{
					//Assign new section index to node
					currentMeshSection = MeshComponent->GetNumSections();
					growingNodeQueue[i]->SetMeshSectionIndex(currentMeshSection);
				}

				//Increase Timer
				growingNodeQueue[i]->AddToGrowingTimer(DeltaTime);

				//Calculate progress of branch using timer
				float currentProgress = BranchLength * growingNodeQueue[i]->GetGrowingTimer() / (1 / RateOfGrowth);

				//If the progress is less than the branch length
				if (currentProgress > BranchLength)
				{
					//Calculate radius of this node based on children
					float radius = pow(growingNodeQueue[i]->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

					//Generating this node's vertices and UVs
					FVector translationVector = (growingNodeQueue[i]->GetTransform().GetTranslation() - growingNodeQueue[i]->GetParent()->GetTransform().GetTranslation()) * (BranchLength - currentProgress) / BranchLength;
					CalculateNodeMeshVerticesAndUV(radius,
						growingNodeQueue[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
						growingNodeQueue[i]->GetTransform().GetTranslation() - translationVector,
						vertices,
						uvs);

					//Calculate radius of parent node based on children
					radius = pow(growingNodeQueue[i]->GetParent()->GetNumOfChildren() * MeshGrowthRate, 1 / MeshGrowthRate) + 0.5;

					//Generating parent node's vertices and UVs
					CalculateNodeMeshVerticesAndUV(radius,
						growingNodeQueue[i]->GetCurrentDirection().ToOrientationRotator().Add(90, 0, 0),
						growingNodeQueue[i]->GetParent()->GetTransform().GetTranslation(),
						vertices,
						uvs);
					
					//Set Material to section
					MeshComponent->SetMaterial(currentMeshSection, Material);

					//Create new mesh section
					MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

					//Empty Arrays
					vertices.Empty();
					uvs.Empty();
				}
				else
				{
					//Reset Mesh Section
					growingNodeQueue[i]->SetMeshSectionIndex(-1);

					//Remove from growing nodes
					nodes.Add(growingNodeQueue[i]);
					growingNodeQueue.RemoveAt(i);
				}
			}
			else
			{
				//Reset Mesh Section
				growingNodeQueue[i]->SetMeshSectionIndex(-1);

				//Remove from growing nodes
				nodes.Add(growingNodeQueue[i]);
				growingNodeQueue.RemoveAt(i);
			}
		}
	}

	//If tree is all grown
	if(allGrown)
	{
		//Empty queue
		growingNodeQueue.Empty();
	}
}

//Create a Spherical Mesh Section
void ASpaceColonizationTreeSeed::CreateSphereMesh(ATreeNode* node)
{
	//Vertex and UV arrays
	TArray<FVector> vertices;
	TArray<FVector2D> uvs;

	//Mesh Triangle Indices
	TArray<int32> triangles;
	UKismetProceduralMeshLibrary::CreateGridMeshTriangles(levelOfDetail + 1, levelOfDetail + 1, true, triangles);

	//Calculate radius of sphere
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
			//Calculate vertex position
			FVector pos = node->GetActorLocation();
			pos.X = FMath::Sin(PI * m / levelOfDetail) * FMath::Cos(2 * PI * n / levelOfDetail) * (radius);
			pos.Y = FMath::Sin(PI * m / levelOfDetail) * FMath::Sin(2 * PI * n / levelOfDetail) * (radius);
			pos.Z = FMath::Cos(PI * m / levelOfDetail) * (radius);
			pos = node->GetActorTransform().TransformPosition(pos);
			pos -= GetActorLocation();

			vertices.Add(pos);

			//Calculate and save UV
			uvs.Add(FVector2D(m / levelOfDetail, n / levelOfDetail));
		}
	}

	//Set Material for Section
	MeshComponent->SetMaterial(currentMeshSection, Material);

	//Create Mesh Section
	MeshComponent->CreateMeshSection(currentMeshSection, vertices, triangles, TArray<FVector>(), uvs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
}


