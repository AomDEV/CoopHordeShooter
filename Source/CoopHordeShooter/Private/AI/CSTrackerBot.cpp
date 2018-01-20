#include "CSTrackerBot.h"
#include "DrawDebugHelpers.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "CSCharacter.h"
#include "CSHealthComponent.h"


static int32 DebugTrackerBotAI = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotAI (
    TEXT("COOP.DebugAIMovement"), 
    DebugTrackerBotAI, 
    TEXT("Draw debug components for AI path points"), 
    ECVF_Cheat
);


// Sets default values
ACSTrackerBot::ACSTrackerBot():
MovementForce(1000.0f),
DistanceDelta(100.0f),
bUseVelocityChange(true),
ExplosionDamage(40.0f),
ExplosionRadius(200.0f),
SelfDamageInterval(0.5f),
bExploded(false),
bStartedSelfDestruction(false)
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetCanEverAffectNavigation(false);
    MeshComponent->SetSimulatePhysics(true);
    RootComponent = MeshComponent;

    HealthComponent = CreateDefaultSubobject<UCSHealthComponent>(TEXT("HealthComponent"));
    HealthComponent->OnHealthChanged.AddDynamic(this, &ACSTrackerBot::HandleTakeDamage);

    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    SphereComponent->SetSphereRadius(200.0f);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SphereComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACSTrackerBot::BeginPlay()
{
    Super::BeginPlay();

    if (Role == ROLE_Authority)
    {
        NextPathPoint = GetNextPathPoint();
    }
}

void ACSTrackerBot::HandleTakeDamage(UCSHealthComponent* HealthComp, float Health, 
                                    float HealthDelta, const UDamageType* DamageType, 
                                    AController* InstigatedBy, AActor* DamageCauser)
{
    if (MaterialInstance == nullptr)
    {
        MaterialInstance = MeshComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComponent->GetMaterial(0));   
    }

    if (MaterialInstance)
    {
        MaterialInstance->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
    }   

    // Explode when no health
    if (Health <= 0.0f)
    {
        SelfDestruct();
    }
}

void ACSTrackerBot::SelfDestruct()
{
    if (bExploded)
    {
        return;
    }

    bExploded = true;
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

    MeshComponent->SetVisibility(false, true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (Role == ROLE_Authority)
    {
        TArray<AActor*> IgnoredActors;
        IgnoredActors.Add(this);
        UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), 
                                            ExplosionRadius, nullptr, IgnoredActors, 
                                            this, GetInstigatorController(), true);

        SetLifeSpan(3.0f);
    }
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

void ACSTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (!bStartedSelfDestruction && !bExploded)
    {
        // Overlapped with a player?
        ACSCharacter* PlayerPawn = Cast<ACSCharacter>(OtherActor);
        if (PlayerPawn)
        {
            bStartedSelfDestruction = true;
            UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);

            if (Role == ROLE_Authority)
            {
                GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ACSTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);   
            }            
        }
    }
}

void ACSTrackerBot::DamageSelf()
{
    UGameplayStatics::ApplyDamage(this, 20.0f, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ACSTrackerBot::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Role == ROLE_Authority && !bExploded)
    {
        float DistanceToPlayer = (GetActorLocation() - NextPathPoint).Size();

        // Build a new path to the player
        if (DistanceToPlayer <= DistanceDelta)
        {
            NextPathPoint = GetNextPathPoint();
    
            if (DebugTrackerBotAI > 0)
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
        

            if (DebugTrackerBotAI > 0)
            {
                DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
            }
        }

        if (DebugTrackerBotAI > 0)
        {
            DrawDebugSphere(GetWorld(), NextPathPoint, 20.0f, 12, FColor::Yellow, false, 5.0f, 1);
        }
    }
}
