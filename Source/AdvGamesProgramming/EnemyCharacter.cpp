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

	switch (CurrentAgentState)
	{
		case (AgentState::PATROL):
		{
			AgentPatrol();

			if (bCanSeeActor)
			{
				if (HealthComponent->HealthPercentageRemaining() < 0.4f)
				{
					UpdateState(AgentState::EVADE);
				}
				else
				{
					UpdateState(AgentState::ENGAGE);
				}
			}
			else if (bHeardActor)
			{
				UpdateState(AgentState::SEARCH);
			}

			break;
		}

		case (AgentState::ENGAGE):
		{
			AgentEngage();

			if (bCanSeeActor)
			{
				if (HealthComponent->HealthPercentageRemaining() < 0.4f)
				{
					UpdateState(AgentState::EVADE);
					Path.Empty();
				}
			}
			else
			{
				UpdateState(AgentState::PATROL);
			}
			break;
		}
		case (AgentState::EVADE):
		{
			AgentEvade();

			if (bCanSeeActor)
			{
				if (HealthComponent->HealthPercentageRemaining() >= 0.4f)
				{
					UpdateState(AgentState::ENGAGE);
					Path.Empty();
				}
			}
			else
			{
				UpdateState(AgentState::PATROL);
			}
			break;
		}
		case (AgentState::SEARCH):
		{
			AgentSearch();

			if (bCanSeeActor)
			{
				if (HealthComponent->HealthPercentageRemaining() < 0.4f)
				{
					UpdateState(AgentState::EVADE);
					bHeardActor = false;
				}
				else
				{
					UpdateState(AgentState::ENGAGE);
					bHeardActor = false;
				}
			}

			if (Path.Num() == 0)
			{
				UpdateState(AgentState::PATROL);
				bHeardActor = false;
			}
			break;
		}
		default: // Just in case!
		{
			UpdateState(AgentState::PATROL);
		}
	}

	MoveAlongPath();
}

void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemyCharacter::UpdateState(AgentState NewState)
{
	FString State = "";

	switch (CurrentAgentState)
	{
		case (AgentState::PATROL):
			State = "PATROL";
			break;
		case (AgentState::ENGAGE):
			State = "ENGAGE";
			break;
		case (AgentState::EVADE):
			State = "EVADE";
			break;
		case (AgentState::SEARCH):
			State = "SEARCH";
			break;
		default:
			State = "Other";
			break;
	}

	UE_LOG(LogTemp, Warning, TEXT("New Agent State: %s"), *State);
	CurrentAgentState = NewState;
}

void AEnemyCharacter::AgentPatrol()
{
	if (Path.Num() == 0 && Manager != NULL)
	{
		//UE_LOG(LogTemp, Error, TEXT("AgentPatrol - New Path"));
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
			//UE_LOG(LogTemp, Error, TEXT("AgentEngage - New Path"));
			FVector ActorLocation = DetectedActor->GetActorLocation();
			auto NearestActorNode = Manager->FindNearestNode(ActorLocation);
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
			//UE_LOG(LogTemp, Error, TEXT("AgentEvade - New Path"));
			FVector ActorLocation = DetectedActor->GetActorLocation();
			auto FurthestActorNode = Manager->FindFurthestNode(ActorLocation);
			Path = Manager->GeneratePath(CurrentNode, FurthestActorNode);
		}
	}
}

void AEnemyCharacter::AgentSearch()
{
	if (Path.Num() == 0) // && bHeardActor
	{
		//UE_LOG(LogTemp, Error, TEXT("AgentSearch - New Path"));
		//UE_LOG(LogTemp, Error, TEXT("AgentSearch - NoisePosition: %s"), *LastNoisePosition.ToString());
		auto NearestNoiseNode = Manager->FindNearestNode(LastNoisePosition);
		Path = Manager->GeneratePath(CurrentNode, NearestNoiseNode);
		bHeardActor = false;
	}
}

void AEnemyCharacter::MoveAlongPath()
{
	if (Path.Num() > 0 && Manager != NULL)
	{
		//UE_LOG(LogTemp, Display, TEXT("Current Node: %s"), *CurrentNode->GetName());
		if ((GetActorLocation() - CurrentNode->Location).IsNearlyZero(100.0f))
		{
			// THESE IS NO NAME IN NavNode auto UE_LOG(LogTemp, Display, TEXT("At Node %s"), *CurrentNode->GetName());
			CurrentNode = Path.Pop();
			// THESE IS NO NAME IN NavNode UE_LOG(LogTemp, Display, TEXT("Going to Node %s"), *CurrentNode->GetName());
			//DrawDebugLine(GetWorld(), GetActorLocation(), CurrentNode->Location, FColor::Red, true, 1.0f, '\000', 8.0f);
		}
		else
		{
			FVector WorldDirection = CurrentNode->Location - GetActorLocation();
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
	//UE_LOG(LogTemp, Display, TEXT("Stimulus Type: %s"), *Stimulus.Type.Name.ToString());
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

