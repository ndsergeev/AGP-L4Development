// Fill out your copyright notice in the Description page of Project Settings.

#include "Room.h"
#include "Engine/World.h"
#include "LevelGenManager.h"

// Sets default values
ALevelGenManager::ALevelGenManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevelGenManager::BeginPlay()
{
	Super::BeginPlay();
	GenerateLevel();
}

// Called every frame
void ALevelGenManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelGenManager::GenerateLevel()
{
	ARoom* InitialRoom = GetWorld()->SpawnActor<ARoom>();
	InitialRoom->SetSize(0, 60, 60, 0);
	InitialRoom->Split();
	InitialRoom->AddCorridors();
	InitialRoom->DrawRoom();
}
