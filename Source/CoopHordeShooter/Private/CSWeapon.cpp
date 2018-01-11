#include "CSWeapon.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ACSWeapon::ACSWeapon():
MuzzleSocketName("MuzzleSocket"),
TracerTargetName("BeamEnd")
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

        // The particle "Target" parameter
        FVector TracerEndPoint = TraceEnd;

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

            if (ImpactEffect)
            {
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
            }

            TracerEndPoint = HitResult.ImpactPoint;
        }

        DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
        
        if (MuzzleEffect)
        {
            UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
        }

        if (TracerEffect)
        {
            FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
            UParticleSystemComponent* TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);


            if (TracerComponent)
            {
                TracerComponent->SetVectorParameter(TracerTargetName, TracerEndPoint);
            }
        }
    }
}

// Called every frame
void ACSWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
