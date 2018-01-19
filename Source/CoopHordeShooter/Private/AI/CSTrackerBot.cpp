#include "CSTrackerBot.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "CSHealthComponent.h"


static int32 DebugAIMovement = 0;
FAutoConsoleVariableRef CVARDebugAIMovement (
    TEXT("COOP.DebugAIMovement"), 
    DebugAIMovement, 
    TEXT("Draw debug components for AI path points"), 
    ECVF_Cheat
);


// Sets default values
ACSTrackerBot::ACSTrackerBot():
MovementForce(1000.0f),
DistanceDelta(100.0f),
bUseVelocityChange(false)
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetCanEverAffectNavigation(false);
    MeshComponent->SetSimulatePhysics(true);
    RootComponent = MeshComponent;

    HealthComponent = CreateDefaultSubobject<UCSHealthComponent>(TEXT("HealthComponent"));
    HealthComponent->OnHealthChanged.AddDynamic(this, &ACSTrackerBot::HandleTakeDamage);
}

// Called when the game starts or when spawned
void ACSTrackerBot::BeginPlay()
{
    Super::BeginPlay();
    NextPathPoint = GetNextPathPoint();
}

void ACSTrackerBot::HandleTakeDamage(UCSHealthComponent* HealthComp, float Health, 
                                    float HealthDelta, const UDamageType* DamageType, 
                                    AController* InstigatedBy, AActor* DamageCauser)
{
    // Explode when no health
}

FVector ACSTrackerBot::GetNextPathPoint()
{
    ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

    // Try to build a path to a player and return the first available from collection
    UNavigationPath* NavigationPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn); 
    if (NavigationPath->PathPoints.Num() > 1)
    {
        return NavigationPath->PathPoints[1];
    }

    // Path to a player doesn't exist. Return the current actor location
    return GetActorLocation();
}

// Called every frame
void ACSTrackerBot::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    float DistanceToPlayer = (GetActorLocation() - NextPathPoint).Size();

    // Build a new path to the player
    if (DistanceToPlayer <= DistanceDelta)
    {
        NextPathPoint = GetNextPathPoint();
    
        if (DebugAIMovement > 0)
        {
            DrawDebugString(GetWorld(), GetActorLocation(), "Target reached.");
        }
    }
    // Move until reached the goal
    else
    {
        FVector ForceDirection = NextPathPoint - GetActorLocation();
        ForceDirection.Normalize();
        ForceDirection *= MovementForce;

        MeshComponent->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

        if (DebugAIMovement > 0)
        {
            DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
        }
    }

    if (DebugAIMovement > 0)
    {
        DrawDebugSphere(GetWorld(), NextPathPoint, 20.0f, 12, FColor::Yellow, false, 5.0f, 1);
    }
}
