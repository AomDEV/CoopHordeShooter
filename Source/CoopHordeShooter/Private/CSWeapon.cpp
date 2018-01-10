#include "CSWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ACSWeapon::ACSWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
}

// Called when the game starts or when spawned
void ACSWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACSWeapon::Fire()
{
    // Trace the world from the pawn eyes to crosshair location
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector EyeLocation;
        FRotator EyeRotation;
        Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

        FVector ShotDirection = EyeRotation.Vector();
        FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Owner);
        QueryParams.AddIgnoredActor(this);
        QueryParams.bTraceComplex = true;

        FHitResult HitResult;
        // Blocking hit handler
        if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
        {
            AActor* HitActor = HitResult.GetActor();
            UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageType);
        }
    }
}

// Called every frame
void ACSWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
