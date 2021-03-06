#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "CSCharacter.generated.h"


class ACSWeapon;
class UCameraComponent;
class USpringArmComponent;
class UCSHealthComponent;

UCLASS()
class COOPHORDESHOOTER_API ACSCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    // Sets default values for this character's properties
    ACSCharacter();
protected:
    float DefaultFOV;
    bool bWantsToZoom;

    UPROPERTY(Replicated, BlueprintReadOnly)
    ACSWeapon* CurrentWeapon;

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Keyboard moving WSAD
    void MoveForward(float Value);
    void MoveRight(float Value);

    void BeginCrouch();
    void EndCrouch();

    void BeginZoom();
    void EndZoom();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UCSHealthComponent* HealthComponent;

    UPROPERTY(EditDefaultsOnly, Category="Player")
    float ZoomedFOV;

    UPROPERTY(EditDefaultsOnly, Category="Player", meta = (ClampMin = 0.1, ClampMax = 100))
    float ZoomInterpolateSpeed;

    UPROPERTY(EditDefaultsOnly, Category="Player")
    TSubclassOf<ACSWeapon> DefaultWeaponClass;

    UPROPERTY(EditDefaultsOnly, Category="Player")
    FName WeaponAttackSocketName;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Player")
    bool bDied;

    UFUNCTION()
    void OnHealthChanged(UCSHealthComponent* HealthComp, float Health,
                        float HealthDelta, const class UDamageType* DamageType, 
                        class AController* InstigatedBy, AActor* DamageCauser);
public:	
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual FVector GetPawnViewLocation() const override;

    UFUNCTION(BlueprintCallable, Category="Player")
    void StartFire();

    UFUNCTION(BlueprintCallable, Category="Player")
    void StopFire();
};
