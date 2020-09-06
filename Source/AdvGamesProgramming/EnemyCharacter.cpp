#include "EnemyCharacter.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentAgentState = AgentState::PATROL;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	PerceptionComponent = FindComponentByClass<UAIPerceptionComponent>();
	if (!PerceptionComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("NO PERCEPTION COMPONENT FOUND"));
	}
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyCharacter::SensePlayer);

	HealthComponent = FindComponentByClass<UHealthComponent>();

	DetectedActor = nullptr;
	bCanSeeActor = false;
	bHeardActor = false;
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentAgentState) {
	case (AgentState::PATROL):
		AgentPatrol();

		if (bCanSeeActor)
		{
			if (HealthComponent->HealthPercentageRemaining() < 0.4f)
			{
				CurrentAgentState = AgentState::EVADE;
			}
			else
			{
				CurrentAgentState = AgentState::ENGAGE;
			}
		}
		else if (bHeardActor)
		{
			CurrentAgentState = AgentState::SEARCH;
		}

		break;

	case (AgentState::ENGAGE):
		AgentEngage();

		if (bCanSeeActor)
		{
			if (HealthComponent->HealthPercentageRemaining() < 0.4f)
			{
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
			if (HealthComponent->HealthPercentageRemaining() >= 0.4f)
			{
				CurrentAgentState = AgentState::ENGAGE;
				Path.Empty();
			}
		}
		else
		{
			CurrentAgentState = AgentState::PATROL;
		}
		break;

	case (AgentState::SEARCH):
		AgentSearch();

		if (bCanSeeActor)
		{
			if (HealthComponent->HealthPercentageRemaining() < 0.4f)
			{
				CurrentAgentState = AgentState::EVADE;
			}
			else
			{
				CurrentAgentState = AgentState::ENGAGE;
			}
		}

		// if nothing was found:
		// CurrentAgentState = AgentState::PATROL;
		break;
	}

	MoveAlongPath();
}

void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyCharacter::AgentPatrol()
{
	if (Path.Num() == 0 && Manager != NULL)
	{
		Path = Manager->GeneratePath(CurrentNode, Manager->AllNodes[FMath::RandRange(0, Manager->AllNodes.Num() - 1)]);
	}
}

void AEnemyCharacter::AgentEngage()
{
	if (bCanSeeActor)
	{
		FVector DirectionToTarget = DetectedActor->GetActorLocation() - GetActorLocation();
		Fire(DirectionToTarget);

		if (Path.Num() == 0)
		{
			FVector ActorLocation = DetectedActor->GetActorLocation();
			ANavigationNode* NearestActorNode = Manager->FindNearestNode(ActorLocation);
			Path = Manager->GeneratePath(CurrentNode, NearestActorNode);
		}
	}
}

void AEnemyCharacter::AgentEvade()
{
	if (bCanSeeActor)
	{
		FVector DirectionToTarget = DetectedActor->GetActorLocation() - GetActorLocation();
		Fire(DirectionToTarget);

		if (Path.Num() == 0)
		{
			FVector ActorLocation = DetectedActor->GetActorLocation();
			ANavigationNode* FurthestActorNode = Manager->FindFurthestNode(ActorLocation);
			Path = Manager->GeneratePath(CurrentNode, FurthestActorNode);
		}
	}
}

void AEnemyCharacter::AgentSearch()
{
	// Pathfind to the last known sound location
}

void AEnemyCharacter::MoveAlongPath()
{
	if (Path.Num() > 0 && Manager != NULL)
	{
		//UE_LOG(LogTemp, Display, TEXT("Current Node: %s"), *CurrentNode->GetName());
		if ((GetActorLocation() - CurrentNode->GetActorLocation()).IsNearlyZero(100.0f))
		{
			UE_LOG(LogTemp, Display, TEXT("At Node %s"), *CurrentNode->GetName());
			CurrentNode = Path.Pop();
			UE_LOG(LogTemp, Display, TEXT("Going to Node %s"), *CurrentNode->GetName());
			DrawDebugLine(GetWorld(), GetActorLocation(), CurrentNode->GetActorLocation(), FColor::Red, true, 3.0f, '\000', 12.0f);
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
	UE_LOG(LogTemp, Display, TEXT("Stimulus Type: %s"), *Stimulus.Type.Name.ToString());

	StimulusType = Stimulus.Type; // Will be 'Default__AISense_Sight' or 'Default__AISense_Hearing'

	if (Stimulus.WasSuccessfullySensed())
	{
		DetectedActor = ActorSensed;

		if (StimulusType.Name.ToString() == "Default__AISense_Hearing")
		{
			UE_LOG(LogTemp, Warning, TEXT("Player Heard"));
			bHeardActor = true;
		}
		else if (StimulusType.Name.ToString() == "Default__AISense_Sight")
		{
			UE_LOG(LogTemp, Warning, TEXT("Player Seen"));
			bCanSeeActor = true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Player Lost"));
		bCanSeeActor = false;
	}
}

