// Fill out your copyright notice in the Description page of Project Settings.


#include "Entity/PX_Projectile.h"
#include "Engine/StaticMesh.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"							// Debug Camera
#include "GameFramework/SpringArmComponent.h"				// Debug Camera
#include "Component/PX_WeaponComponent.h"
#include "Entity/PX_Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"
#include "AbilitySystem/Effects/PX_GE_Damage.h"
#include "AbilitySystem/Effects/PX_GE_StatusEffects.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Net/UnrealNetwork.h"								// Replication

// Sets default values
APX_Projectile::APX_Projectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup Sphere Component
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	//SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->SetNotifyRigidBodyCollision(true);		// OnHit()
	SphereCollision->SetSphereRadius(2.f);
	//SphereCollision->SetCollisionProfileName(TEXT("BlockAll"));
	SphereCollision->SetCollisionProfileName("PX_Combat");
	SphereCollision->OnComponentHit.AddDynamic(this, &APX_Projectile::OnHit);
	//SphereCollision->bHiddenInGame = false;
	RootComponent = SphereCollision;

	// Setup Static Mesh Component
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeLocation(FVector(-92.5f, 0.f, 0.f));
	Mesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(TEXT("/Game/Project_X/Character/Weapon/Bow/SM_Arrow.SM_Arrow"));
	if (StaticMeshAsset.Succeeded())
	{
		GetMesh()->SetStaticMesh(StaticMeshAsset.Object);
	}

	// Setup Projectile Movement Component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = SphereCollision;
	ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->InitialSpeed = 7500.f;
	ProjectileMovement->MaxSpeed = 7500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// Setup SpringArm Component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bDoCollisionTest = false;

	// Setup Camera Component
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	bReplicates = true;
	SetReplicateMovement(true);
	InitialLifeSpan = 10.0f;
	DamageGameplayEffectClass = UPX_GE_Damage::StaticClass();
}

void APX_Projectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APX_Projectile, bDisableProjectile);
}

// Called when the game starts or when spawned
void APX_Projectile::BeginPlay()
{
	Super::BeginPlay();

#if !UE_SERVER
	// Debug Camera
	if (!FollowCamera) return;

	// Direction of Projectile
	FVector ProjectileVec = GetProjectileMovement() ? GetProjectileMovement()->Velocity.GetSafeNormal() : GetActorForwardVector();
	// Rotation of Projectile
	const FRotator ProjectileRot = ProjectileVec.Rotation();

	// Set Camera Location Offset
	const FVector TranslationOffset = ProjectileVec * -200.f;
	const FVector RotationOffset = ProjectileRot.RotateVector(FVector(0.f, 150.f, 100.f));
	const FVector CameraLoc = GetActorLocation() + TranslationOffset + RotationOffset;

	// Set Camera Rotation Offset
	const FRotator CameraRot = (ProjectileRot + FRotator(-20.f, -20.f, 0.f)).GetNormalized();

	// Set Caemra Location & Rotation
	FollowCamera->SetWorldLocation(CameraLoc);
	FollowCamera->SetWorldRotation(CameraRot);
#endif
}

// Called every frame
void APX_Projectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APX_Projectile::OnHit(UPrimitiveComponent* Hitcomponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;
	if ( bDisableProjectile ) return;
	if ( !OtherActor || OtherActor == GetOwner() || OtherActor == this ) return;
	//UE_LOG(LogTemp, Log, TEXT("Projectile On Hit"));

	bDisableProjectile = true;
	DisableProjectile();					// Disable Collision at Server

	const bool bAppliedDamage = ApplyDamageGameplayEffect(OtherActor);
	ApplyStatusGameplayEffects(OtherActor);

	//AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform);

	if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(OtherComp))
	{
		//AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepWorldTransform, Hit.BoneName);
		UE_LOG(LogTemp, Log, TEXT("%s applies %s damage GE (%f) to %s at %s"), *this->GetName(), bAppliedDamage ? TEXT("valid") : TEXT("invalid"), DamageAmount, *OtherActor->GetName(), *Hit.BoneName.ToString());
	}
	else if (OtherComp)
	{
		AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform);
		UE_LOG(LogTemp, Log, TEXT("Attached to %s"), *OtherActor->GetName());
	}

	// (옵션) 5초 뒤 제거
	if ( APX_Character* PX_Character = Cast<APX_Character>(OtherActor) )
	{
		SetLifeSpan(0.1f);
	}

	/*
	* Blend Time이 끝나기 전에 PX_Character로 돌아가야 하는 상황에는 CurrentViewTarget이 PX_Character에서 Projectile로 아직 변하지 않았으므로
	* OnHit이 호출되어도 Projectile에서 PX_Character로 카메라 전환이 이루어지지 않음
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (PlayerController)
	{
		AActor* CurrentViewTarget = PlayerController->GetViewTarget();
		UE_LOG(LogTemp, Log, TEXT("OnHit"));
		UE_LOG(LogTemp, Log, TEXT("CurrentViewTarget: % s"), *CurrentViewTarget->GetName());
		if (CurrentViewTarget != Pawn)
		{
			ClientCameraTransition(GetOwner());
		}
	}
	*/

	//ClientCameraTransition(GetOwner());

	// 서버가 받은 클라이언트 조준의 충돌 지점 (빨간 구)
	//DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 4.0f, 16, FColor::Red, false, 2.f);
}

bool APX_Projectile::ApplyDamageGameplayEffect(AActor* TargetActor)
{
	if ( !HasAuthority() || !TargetActor || DamageAmount <= 0.f || !DamageGameplayEffectClass )
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if ( !TargetASC )
	{
		return false;
	}

	AActor* SourceActor = GetInstigator() ? Cast<AActor>(GetInstigator()) : GetOwner();
	UAbilitySystemComponent* SourceASC = SourceActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor) : nullptr;
	if ( !SourceASC )
	{
		SourceASC = TargetASC;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddInstigator(GetInstigator(), this);
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGameplayEffectClass, 1.0f, EffectContext);
	if ( !SpecHandle.IsValid() || !SpecHandle.Data.IsValid() )
	{
		return false;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(UPX_GE_Damage::DamageSetByCallerName, DamageAmount);
	const FActiveGameplayEffectHandle AppliedHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	return AppliedHandle.WasSuccessfullyApplied();
}

void APX_Projectile::ApplyStatusGameplayEffects(AActor* TargetActor)
{
	if ( !HasAuthority() || !TargetActor || StatusGameplayEffectClasses.IsEmpty() )
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if ( !TargetASC )
	{
		return;
	}

	AActor* SourceActor = GetInstigator() ? Cast<AActor>(GetInstigator()) : GetOwner();
	UAbilitySystemComponent* SourceASC = SourceActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor) : nullptr;
	if ( !SourceASC )
	{
		SourceASC = TargetASC;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddInstigator(GetInstigator(), this);
	EffectContext.AddSourceObject(this);

	for ( TSubclassOf<UGameplayEffect> EffectClass : StatusGameplayEffectClasses )
	{
		if ( !EffectClass )
		{
			continue;
		}

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
		if ( SpecHandle.IsValid() && SpecHandle.Data.IsValid() )
		{
			if ( EffectClass->IsChildOf(UPX_GE_BurningDamage::StaticClass()) )
			{
				SpecHandle.Data->DynamicGrantedTags.AddTag(PX_GameplayTags::State_Condition_Burning);
			}
			else if ( EffectClass->IsChildOf(UPX_GE_SlowDebuff::StaticClass()) )
			{
				SpecHandle.Data->DynamicGrantedTags.AddTag(PX_GameplayTags::State_Condition_Slowed);
			}
			else if ( EffectClass->IsChildOf(UPX_GE_StunDebuff::StaticClass()) )
			{
				SpecHandle.Data->DynamicGrantedTags.AddTag(PX_GameplayTags::State_Condition_Stunned);
			}

			const FActiveGameplayEffectHandle AppliedHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			UE_LOG(LogTemp, Log, TEXT("%s applies %s status GE %s to %s"), *GetName(), AppliedHandle.WasSuccessfullyApplied() ? TEXT("valid") : TEXT("invalid"), *GetNameSafe(EffectClass), *GetNameSafe(TargetActor));

			if ( AppliedHandle.WasSuccessfullyApplied() && EffectClass->IsChildOf(UPX_GE_StunDebuff::StaticClass()) )
			{
				if ( UPX_AbilitySystemComponent* PX_TargetASC = Cast<UPX_AbilitySystemComponent>(TargetASC) )
				{
					PX_TargetASC->CancelAbilitiesBlockedByTag(PX_GameplayTags::State_Condition_Stunned);
				}
			}
		}
	}
}

void APX_Projectile::SetStatusGameplayEffectClasses(const TArray<TSubclassOf<UGameplayEffect>>& InStatusGameplayEffectClasses)
{
	StatusGameplayEffectClasses = InStatusGameplayEffectClasses;
}

void APX_Projectile::ClientCameraTransition_Implementation(AActor* TargetActor)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!Pawn || !PlayerController)	return;

	//UE_LOG(LogTemp, Log, TEXT("ClientCameraTransition"));
	//UE_LOG(LogTemp, Log, TEXT("Owner: % s, PlayerController : %s, TargetActor : %s"), *Pawn->GetName(), *PlayerController->GetName(), *TargetActor->GetName());

	// Camera Transition to Target Actor
	const float TransitionTime = TargetActor->IsA<APX_Character>() ? 0.f : 0.25f;
	PlayerController->SetViewTargetWithBlend(TargetActor, TransitionTime, VTBlend_Cubic);
}

void APX_Projectile::OnRep_DisableProjectile()
{
	DisableProjectile();
}

void APX_Projectile::DisableProjectile()
{
	/*
	if (GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Log, TEXT("Collision Disabled in Server"));
	}
	else
	{
		FString Temp = GetWorld()->GetMapName();
		Temp.RemoveFromStart(TEXT("UEDPIE_"));

		FString Idx;
		if (Temp.Split(TEXT("_"), &Idx, nullptr))
		{
			UE_LOG(LogTemp, Log, TEXT("Collision Disabled in Client %d"), FCString::Atoi(*Idx));
		}
	}
	*/

	// Deactivate Projectile Movement
	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
	}

	// Set Sphere Collision to No Collision
	if (SphereCollision)
	{
		SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
