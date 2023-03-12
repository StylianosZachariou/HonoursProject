// Fill out your copyright notice in the Description page of Project Settings.


#include "AttractionNode.h"

// Sets default values
AAttractionNode::AAttractionNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	collider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
	collider->SetupAttachment(RootComponent);

/*	auto Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	auto Material = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Game/StarterContent/Materials/M_Metal_Gold.M_Metal_Gold'"));
	auto matInstance = UMaterialInstanceDynamic::Create(Material.Object, StaticMeshComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetStaticMesh(Mesh.Object);
	StaticMeshComponent->SetMaterial(0, matInstance);
	StaticMeshComponent->SetRelativeScale3D(FVector(0.1, 0.1, 0.1));
	StaticMeshComponent->SetupAttachment(RootComponent);*/
}

// Called when the game starts or when spawned
void AAttractionNode::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAttractionNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

