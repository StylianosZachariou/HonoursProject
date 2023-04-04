// Fill out your copyright notice in the Description page of Project Settings.


#include "TreeSeed.h"

// Sets default values
ATreeSeed::ATreeSeed()
{
	PrimaryActorTick.bCanEverTick = true;

	TimeOfGrowth = 10;
	RateOfGrowth = 10;
}

void ATreeSeed::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATreeSeed::BeginPlay()
{
	Super::BeginPlay();
}

