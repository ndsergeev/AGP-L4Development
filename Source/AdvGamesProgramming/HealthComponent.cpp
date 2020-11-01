#include "HealthComponent.h"
#include "Engine/GameEngine.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerHUD.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    MaxHealth = 100.0f;
}


void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
}


void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //Health Debug messages
    /*
    if (GEngine && GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Current Health: %f"), CurrentHealth));
    }
    */
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

void UHealthComponent::OnTakeDamage(float Damage)
{
    CurrentHealth -= Damage;
    if (CurrentHealth <= 0)
    {
        CurrentHealth = 0;
        OnDeath();
    }

    UpdateHealthBar();
}

void UHealthComponent::OnDeath()
{
    auto* OwningPlayerCharacter = Cast<APlayerCharacter>(GetOwner());
    if (OwningPlayerCharacter)
    {
        OwningPlayerCharacter->OnDeath();
        return;
    }

    auto* EnemyCharacter = Cast<AEnemyCharacter>(GetOwner());
	if (EnemyCharacter)
    {
#ifdef UE_EDITOR
        UE_LOG(LogTemp, Warning, TEXT("ENEMY IS DEAD"));
#endif
        EnemyCharacter->Manager->AllAgents.Remove(EnemyCharacter);
        EnemyCharacter->Destroy();
    }
}

float UHealthComponent::HealthPercentageRemaining()
{
    return CurrentHealth / MaxHealth;
}

void UHealthComponent::UpdateHealthBar()
{
    auto* Player = Cast<APlayerCharacter>(GetOwner());
    if (Player)
    {
        if (Player->IsLocallyControlled())
        {
            //Find the hud associated to this player
            auto* HUD = Cast<APlayerHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
            if (HUD)
            {
                //Update the progress bar widget on the players hud.
                HUD->SetPlayerHealthBarPercent(HealthPercentageRemaining());
                return;
            }
        }
    }

	auto* Enemy = Cast<AEnemyCharacter>(GetOwner());
	if (Enemy)
    {
        Enemy->SetEnemyHealthBarPercent(HealthPercentageRemaining());
        UE_LOG(LogTemp, Warning, TEXT("Current Health: %f AND MAX %f"), CurrentHealth, MaxHealth);
    }
}
