#include "EnemyCharacter.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ProgressBar.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    CharacterSpeed = 1.0f;
    CurrentAgentState = AgentState::PATROL;

    HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Health Widget");

    static ConstructorHelpers::FClassFinder<UUserWidget> EnemyHealthWidgetComponent(TEXT("/Game/Widgets/EnemyHUDWidget"));

	if (EnemyHealthWidgetComponent.Succeeded())
	{
		HealthWidgetComponent->SetWidgetClass(EnemyHealthWidgetComponent.Class);
	}
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    PerceptionComponent = FindComponentByClass<UAIPerceptionComponent>();
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyCharacter::SensePlayer);
    }
    DetectedActor = nullptr;
    bCanSeeActor = false;
    bHeardActor = false;

    HealthComponent = FindComponentByClass<UHealthComponent>();

    if (HealthWidgetComponent)
    {
        /**
         * However, some of these might be moved to the constructor
         */
        HealthWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
        HealthWidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        HealthWidgetComponent->SetDrawSize(FVector2D(100, 40));
        HealthWidgetComponent->SetRelativeLocation(FVector(0,0,120));
        HealthWidgetComponent->SetVisibility(true);
        HealthWidgetComponent->RegisterComponent();

        HealthProgressBar = Cast<UProgressBar>(HealthWidgetComponent->GetUserWidgetObject()->GetWidgetFromName("HealthPercentage"));

#ifdef UE_EDITOR
        if (!HealthProgressBar)
        {
            UE_LOG(LogTemp, Error, TEXT("HealthProgressBar WAS NOT FOUND"));
        }
#endif
    }

    auto* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        PlayerCameraManager = PlayerController->PlayerCameraManager;
    }
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);


	if (PlayerCameraManager && HealthWidgetComponent)
	{
		auto RotationTowardsCamera = UKismetMathLibrary::FindLookAtRotation(HealthWidgetComponent->GetComponentLocation(),
																			PlayerCameraManager->GetCameraLocation());
		HealthWidgetComponent->SetWorldRotation(RotationTowardsCamera);
	}

	/**
	 * Make sure it is generated once on the server
	 */
	if (!HasAuthority()) return;

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

    //UE_LOG(LogTemp, Warning, TEXT("New Agent State: %s"), *State);
    CurrentAgentState = NewState;
}

void AEnemyCharacter::AgentPatrol()
{
    CharacterSpeed = 1.0f;

    if (Path.Num() == 0 && Manager)
    {
        //UE_LOG(LogTemp, Error, TEXT("AgentPatrol - New Path"));
        Path = Manager->GeneratePath(CurrentNode, Manager->AllNodes[FMath::RandRange(0, Manager->AllNodes.Num() - 1)]);
    }
}

void AEnemyCharacter::AgentEngage()
{
    CharacterSpeed = 0.3f;

    if (bCanSeeActor && DetectedActor)
    {
        FVector DirectionToTarget = DetectedActor->GetActorLocation() - GetActorLocation();
        Fire(DirectionToTarget);

        if (Path.Num() == 0)
        {
            //UE_LOG(LogTemp, Error, TEXT("AgentEngage - New Path"));
            FVector ActorLocation = DetectedActor->GetActorLocation();
            auto* NearestActorNode = Manager->FindNearestNode(ActorLocation);
            Path = Manager->GeneratePath(CurrentNode, NearestActorNode);
        }
    }
}

void AEnemyCharacter::AgentEvade()
{
    CharacterSpeed = 0.3f;

    if (bCanSeeActor && DetectedActor)
    {
        FVector DirectionToTarget = DetectedActor->GetActorLocation() - GetActorLocation();
        Fire(DirectionToTarget);

        if (Path.Num() == 0)
        {
            //UE_LOG(LogTemp, Error, TEXT("AgentEvade - New Path"));
            FVector ActorLocation = DetectedActor->GetActorLocation();
            auto* FurthestActorNode = Manager->FindFurthestNode(ActorLocation);
            Path = Manager->GeneratePath(CurrentNode, FurthestActorNode);
        }
    }
}

void AEnemyCharacter::AgentSearch()
{
    CharacterSpeed = 1.0f;

    if (Path.Num() == 0) // && bHeardActor
    {
        //UE_LOG(LogTemp, Error, TEXT("AgentSearch - New Path"));
        //UE_LOG(LogTemp, Error, TEXT("AgentSearch - NoisePosition: %s"), *LastNoisePosition.ToString());
        auto* NearestNoiseNode = Manager->FindNearestNode(LastNoisePosition);
        Path = Manager->GeneratePath(CurrentNode, NearestNoiseNode);
        bHeardActor = false;
    }
}

void AEnemyCharacter::MoveAlongPath()
{
    if (Path.Num() > 0 && Manager)
    {
        //UE_LOG(LogTemp, Display, TEXT("Current Node: %s"), *CurrentNode->GetName());
        if ((GetActorLocation() - CurrentNode->Location).IsNearlyZero(100.0f))
        {
            CurrentNode = Path.Pop();
            //DrawDebugLine(GetWorld(), GetActorLocation(), CurrentNode->Location, FColor::Red, true, 1.0f, '\000', 8.0f);
        }
        else
        {
            FVector WorldDirection = CurrentNode->Location - GetActorLocation();
            WorldDirection.Normalize();

            AddMovementInput(WorldDirection, CharacterSpeed);

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
            //UE_LOG(LogTemp, Warning, TEXT("Player Heard"));
            bHeardActor = true;
        }
        else if (StimulusType.Name.ToString() == "Default__AISense_Sight")
        {
            //UE_LOG(LogTemp, Warning, TEXT("Player Seen"));
            bCanSeeActor = true;
        }
    }
    else
    {
        //UE_LOG(LogTemp, Log, TEXT("Player Lost"));
        bCanSeeActor = false;
    }
}

void AEnemyCharacter::SetEnemyHealthBarPercent(float Percent)
{
    if (HealthProgressBar) {
        //HealthProgressBar->SetPercent(Percent);
        HealthProgressBar->Percent = Percent;

#ifdef UE_EDITOR
        /**
         * Andrew, you can see from this output that the Progress bar receives the correct value
         * If put "HealthProgressBar->Percent = 0.5f;", it shows
         */
        UE_LOG(LogTemp, Error, TEXT("PROGRESS: %s:\t %f"), *HealthProgressBar->GetName(), HealthProgressBar->Percent);
#endif
    }
}