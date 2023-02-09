// Fill out your copyright notice in the Description page of Project Settings.


#include "TreeSeed.h"

// Sets default values
ATreeSeed::ATreeSeed()
{
	PrimaryActorTick.bCanEverTick = true;

	TimeOfGrowth = 10;
	RateOfGrowth = 10;
}

// Called when the game starts or when spawned
void ATreeSeed::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ATreeSeed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}


