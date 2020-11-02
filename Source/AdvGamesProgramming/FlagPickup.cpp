#include "FlagPickup.h"
#include "Engine/Engine.h"
#include "PlayerCharacter.h"


void AFlagPickup::Init()
{
	Super::Init();

#ifdef UE_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("Flag Instantiated"));
#endif

	SphereRadius = SphereRadius == 0 ? 1.0f : SphereRadius;
	SphereComponent->SetWorldScale3D(FVector(SphereRadius, SphereRadius, SphereRadius));
}

void AFlagPickup::OnPickupBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APlayerCharacter>(OtherActor))
	{
#ifdef UE_EDITOR
		if (OtherActor && (OtherActor != this) && OtherComp && HasAuthority())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Player Entered Finish"));
		}
#endif
	}
}