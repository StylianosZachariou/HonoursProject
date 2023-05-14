#include "AttractionNode.h"

//Constructor
AAttractionNode::AAttractionNode()
{
	PrimaryActorTick.bCanEverTick = true;

	//Component Initializations
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	Collider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
	Collider->SetupAttachment(RootComponent);

	///////////////// DEMONSTRATION PURPOSES /////////////////
/*	auto Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	auto Material = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Game/StarterContent/Materials/M_Metal_Gold.M_Metal_Gold'"));
	auto matInstance = UMaterialInstanceDynamic::Create(Material.Object, StaticMeshComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetStaticMesh(Mesh.Object);
	StaticMeshComponent->SetMaterial(0, matInstance);
	StaticMeshComponent->SetRelativeScale3D(FVector(0.1, 0.1, 0.1));
	StaticMeshComponent->SetupAttachment(RootComponent);*/
	/////////////////////////////////////////////////////////
}

