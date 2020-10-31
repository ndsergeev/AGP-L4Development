#include "FlagPickup.h"


void AFlagPickup::Init()
{
    Super::Init();

#ifdef UE_EDITOR
    UE_LOG(LogTemp, Warning, TEXT("ENTERED FINISH AREA"));
#endif

    SphereRadius = SphereRadius == 0 ? 1.0f : SphereRadius;
    SphereComponent->SetWorldScale3D(FVector(SphereRadius, SphereRadius, SphereRadius));
}
