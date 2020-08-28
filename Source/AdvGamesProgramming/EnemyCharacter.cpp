// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "EngineUtils.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CurrentAgentState = AgentState::PATROL;
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	PerceptionComponent = FindComponentByClass<UAIPerceptionComponent>();
	if (!PerceptionComponent)
	{
	    UE_LOG(LogTemp, Error, TEXT("NO PERCEPTION COMPONENT FOUND"))
	}
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyCharacter::SensePlayer);

    HealthComponent = FindComponentByClass<UHealthComponent>();

    DetectedActor = nullptr;
	bCanSeeActor = false;
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    switch (CurrentAgentState) {
        case (AgentState::PATROL):
            AgentPatrol();

            if (bCanSeeActor)
            {
                if (HealthComponent->HealthPercentageRemaining() < 0.4f) { CurrentAgentState = AgentState::EVADE; }
                else { CurrentAgentState = AgentState::ENGAGE; }
            }
            break;
        case (AgentState::ENGAGE):
            AgentEngage();

            if (bCanSeeActor)
            {
                if (HealthComponent->HealthPercentageRemaining() < 0.4f) {
                    CurrentAgentState = AgentState::EVADE;
                    Path.Empty();
                }
            }
            else
            {
                CurrentAgentState = AgentState::PATROL;
            }
            break;
        case (AgentState::EVADE):
            AgentEvade();

            if (bCanSeeActor)
            {
                if (HealthComponent->HealthPercentageRemaining() >= 0.4f) {
                    CurrentAgentState = AgentState::ENGAGE;
                    Path.Empty();
                }
            }
            else
            {
                CurrentAgentState = AgentState::PATROL;
            }
            break;
    }

	MoveAlongPath();
}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyCharacter::AgentPatrol()
{
    if (Path.Num() == 0 && Manager != NULL)
    {
        Path = Manager->GeneratePath(CurrentNode, Manager->AllNodes[FMath::RandRange(0, Manager->AllNodes.Num()-1)]);
    }
}
void AEnemyCharacter::AgentEngage()
{
    if (bCanSeeActor)
    {
        auto DirectionToTarget = DetectedActor->GetActorLocation() - GetActorLocation();
        Fire(DirectionToTarget);

        if (Path.Num() == 0 && Manager != NULL)
        {
            auto NearToPlayerNode = Manager->FindNearestNode(DetectedActor->GetActorLocation());
            Path = Manager->GeneratePath(NearToPlayerNode, Manager->AllNodes[FMath::RandRange(0, Manager->AllNodes.Num()-1)]);
        }
    }
}
void AEnemyCharacter::AgentEvade()
{
    if (bCanSeeActor)
    {
        auto DirectionToTarget = DetectedActor->GetActorLocation() - GetActorLocation();
        Fire(DirectionToTarget);

        if (Path.Num() == 0 && Manager != NULL)
        {
            auto NearToPlayerNode = Manager->FindFurthestNode(DetectedActor->GetActorLocation());
            Path = Manager->GeneratePath(NearToPlayerNode, Manager->AllNodes[FMath::RandRange(0, Manager->AllNodes.Num()-1)]);
        }
    }
}

void AEnemyCharacter::MoveAlongPath()
{
    if (Path.Num() > 0 && Manager != NULL)
    {
        //UE_LOG(LogTemp, Display, TEXT("Current Node: %s"), *CurrentNode->GetName())
        if ((GetActorLocation() - CurrentNode->GetActorLocation()).IsNearlyZero(100.0f))
        {
            UE_LOG(LogTemp, Display, TEXT("At Node %s"), *CurrentNode->GetName())
            CurrentNode = Path.Pop();
        }
        else
        {
            FVector WorldDirection = CurrentNode->GetActorLocation() - GetActorLocation();
            WorldDirection.Normalize();
            AddMovementInput(WorldDirection, 1.0f);

            //Get the AI to face in the direction of travel.
            FRotator FaceDirection = WorldDirection.ToOrientationRotator();
            FaceDirection.Roll = 0.f;
            FaceDirection.Pitch = 0.f;
            //FaceDirection.Yaw -= 90.0f;
            SetActorRotation(FaceDirection);
        }
    }
}

void AEnemyCharacter::SensePlayer(AActor* ActorSensed, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        UE_LOG(LogTemp, Log, TEXT("Player Detected"))
        DetectedActor = ActorSensed;
        bCanSeeActor = true;
    } else {
        UE_LOG(LogTemp, Log, TEXT("Player Lost"))
//        DetectedActor = nullptr;
        bCanSeeActor = false;
    }
}

