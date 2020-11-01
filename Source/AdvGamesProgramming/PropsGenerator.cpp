#include "EngineUtils.h"
#include "PropsGenerator.h"

APropsGenerator::APropsGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	static ConstructorHelpers::FClassFinder<AActor> BottleBlueprint(TEXT("/Game/Blueprints/Bottle_DM_Blueprint"));
	if (!BottleBlueprint.Class)
	{
#ifdef UE_EDITOR
		UE_LOG(LogTemp, Error, TEXT("Bottle Blueprint is NULL"));
#endif
		return;
	}
	BottleToSpawn = BottleBlueprint.Class;

	static ConstructorHelpers::FClassFinder<AActor> LampBlueprint(TEXT("/Game/Blueprints/LampBlueprint"));
	if (!LampBlueprint.Class)
	{
#ifdef UE_EDITOR
		UE_LOG(LogTemp, Error, TEXT("Lamp Blueprint is NULL"));
#endif
		return;
	}
	LampToSpawn = LampBlueprint.Class;
}

void APropsGenerator::BeginPlay()
{
	Super::BeginPlay();

	/**
	 * Make sure it is generated once on the server
	 */
	if (!HasAuthority()) return;

	/**
	 * Make sure ALevelGenManager Finish its BeginPlay() function
	 */
	TActorIterator<ALevelGenManager> It(GetWorld());
	if (!It) return;

	if (!It->HasActorBegunPlay())
	{
		It->DispatchBeginPlay();
	}

	LevelGenManager = *It;

	SpawnProps(BottleToSpawn, BottleNum);
	SpawnProps(LampToSpawn, LampNum);
}

void APropsGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/**
 * Function to spawn passed subclass (preferably BPs)
 */
void APropsGenerator::SpawnProps(TSubclassOf<AActor> A, uint8 N)
{
	TArray<ARoom*> Rooms = LevelGenManager->Rooms;

	for (int i = 0; i < N; ++i)
	{
		auto* RandRoom = Rooms[FMath::RandRange(0, Rooms.Num() - 1)];

		auto HalfFloorOffset = RandRoom->FloorOffset / 2;
		auto W = float(RandRoom->Right - RandRoom->Left);
		auto H = float(RandRoom->Top - RandRoom->Bottom);
		auto XCoord = FMath::RandRange(-W, W) * HalfFloorOffset;
		auto YCoord = FMath::RandRange(-H, H) * HalfFloorOffset;

		auto SpawnLocation = RandRoom->CenterLocation + FVector(XCoord, YCoord, 20);

		GetWorld()->SpawnActor<AActor>(A, SpawnLocation, FRotator::ZeroRotator);
	}
}