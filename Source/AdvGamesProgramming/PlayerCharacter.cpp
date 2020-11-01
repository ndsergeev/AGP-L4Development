#include "PlayerCharacter.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "HealthComponent.h"
#include "MultiplayerGameMode.h"
#include "PlayerHUD.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//AutoPossessPlayer = EAutoReceiveInput::Player0;
	bUseControllerRotationPitch = true;

	LookSensitivity = 1.0f;
	SprintMultiplier = 1.5f;

	//Set the normal and sprint movement speeds
	NormalMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SprintMovementSpeed = GetCharacterMovement()->MaxWalkSpeed * SprintMultiplier;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	Camera = FindComponentByClass<UCameraComponent>();
	HealthComponent = FindComponentByClass<UHealthComponent>();
	if (HealthComponent)
	{
		HealthComponent->SetIsReplicated(true);
	}

	//Set the rounds remaining and health hud components
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Strafe"), this, &APlayerCharacter::Strafe);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APlayerCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APlayerCharacter::Turn);

	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Pressed, this, &APlayerCharacter::SprintStart);
	PlayerInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Released, this, &APlayerCharacter::SprintEnd);
}

void APlayerCharacter::MoveForward(float Value)
{
	FRotator ForwardRotation = GetControlRotation();
	ForwardRotation.Roll = 0.0f;
	ForwardRotation.Pitch = 0.0f;
	AddMovementInput(ForwardRotation.Vector(), Value);
}

void APlayerCharacter::Strafe(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void APlayerCharacter::LookUp(float Value)
{
	if (Camera)
	{
		FRotator DeltaRotation = FRotator::ZeroRotator;
		DeltaRotation.Pitch = Value * LookSensitivity;
		//Bonus Task - Removing Stutter by only adding relative rotation if it does not push pitch above or below 90 or -90 respectively
		if (DeltaRotation.Pitch + Camera->RelativeRotation.Pitch < 90.0f && DeltaRotation.Pitch + Camera->RelativeRotation.Pitch > -90.0f)
		{
			Camera->AddRelativeRotation(DeltaRotation);
		}
		//Need to make sure that the camera is not rolling or yawing when the pitch is
		//trying to pitch greater than 90 or less than -90. AddRelativeRotation starts
		//adding things to roll and yaw at these extremes.
		Camera->RelativeRotation.Yaw = 0.0f;
		Camera->RelativeRotation.Roll = 0.0f;
	}
}

void APlayerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value * LookSensitivity);
}

void APlayerCharacter::SprintStart()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintMovementSpeed;
	ServerSprintStart();
}

void APlayerCharacter::SprintEnd()
{
	GetCharacterMovement()->MaxWalkSpeed = NormalMovementSpeed;
	ServerSprintEnd();
}

void APlayerCharacter::OnDeath()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		AMultiplayerGameMode* GameMode = Cast<AMultiplayerGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->Respawn(GetController());
		}
	}

}

void APlayerCharacter::HidePlayerHUD_Implementation(bool bSetHUDVisibility)
{
	//Get the player controller then the player hud of the autonomous proxy
	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (APlayerHUD* HUD = Cast<APlayerHUD>(PlayerController->GetHUD()))
			{
				HUD->SetHideWidgets(bSetHUDVisibility);
			}
		}
	}
}

void APlayerCharacter::ServerSprintStart_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintMovementSpeed;
}

void APlayerCharacter::ServerSprintEnd_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = NormalMovementSpeed;
}

