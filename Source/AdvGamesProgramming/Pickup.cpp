#include "Pickup.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    NetDormancy = DORM_Initial;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Pickup Root"));

    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Bounding Sphere"));
    SphereComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

    SphereComponent->SetGenerateOverlapEvents(true);
    SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnPickupBeginOverlap);
    //SphereComponent->OnComponentEndOverlap.AddDynamic(this, &APickup::OnPickupEndOverlap);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

    /**
     * Make sure it is generated once on the server
     */
    if (!HasAuthority()) return;

    Init();
}

void APickup::Init()
{
}

void APickup::OnPickupBeginOverlap(class UPrimitiveComponent* OverlappedComp,
                                   class AActor* OtherActor,
                                   class UPrimitiveComponent* OtherComp,
                                   int32 OtherBodyIndex,
                                   bool bFromSweep,
                                   const FHitResult& SweepResult)
{
#ifdef UE_EDITOR
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Parent Overlap"));
    }
#endif
}

//void APickup::OnPickupEndOverlap(class UPrimitiveComponent* OverlappedComp,
//                                 class AActor* OtherActor,
//                                 class UPrimitiveComponent* OtherComp,
//                                 int32 OtherBodyIndex)
//{
//#ifdef UE_EDITOR
//    if (OtherActor && (OtherActor != this) && OtherComp)
//    {
//        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Overlap End"));
//    }
//#endif
//}