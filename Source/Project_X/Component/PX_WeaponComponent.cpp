// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PX_WeaponComponent.h"
#include "Entity/PX_Projectile.h"
#include "Entity/PX_Character.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UPX_WeaponComponent::UPX_WeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	//SetIsReplicatedByDefault(true);
	ProjectileClass = APX_Projectile::StaticClass();
}

// Called when the game starts
void UPX_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UPX_WeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPX_WeaponComponent::SetProjectileClass(TSubclassOf<APX_Projectile> NewProjectileClass)
{
	ProjectileClass = NewProjectileClass;
}

void UPX_WeaponComponent::Attack()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner<APawn>()->GetController());
	if (!PlayerController) return;

	// Get Camera Location & Rotation
	const FVector WorldLoc = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FRotator WorldRot = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector WorldDir = WorldRot.Vector().GetSafeNormal();

	//UE_LOG(LogTemp, Log, TEXT("Client Loc : %s, Client Dir : %s"), *WorldLoc.ToString(), *WorldDir.ToString());

	// Debug Draw
	const float TraceRange = 15000.f;
	const float Duration = 2.0f;
	const float Radius = 4.f;
	const float Thickness = 0.f;

	// Client Camera Hit Trance
	FHitResult HitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CrosshairTrace), true, GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, WorldLoc, WorldLoc + WorldDir * TraceRange, ECC_Visibility, Params);
	const FVector TargetPoint = bHit ? HitResult.ImpactPoint : WorldLoc + WorldDir * TraceRange;

	// Client Line Trace (Cyan)
	DrawDebugLine(GetWorld(), WorldLoc, WorldLoc + WorldDir * TraceRange, FColor::Cyan, false, Duration, 0, Thickness);

	// Client Target Point (Red)
	DrawDebugSphere(GetWorld(), TargetPoint, Radius, 16, FColor::Red, false, Duration);

	// Server Attack
	ServerAttack(WorldLoc, TargetPoint);
}

bool UPX_WeaponComponent::ServerAttack_Validate(const FVector_NetQuantize ClientLoc, const FVector_NetQuantize ClientDir)
{
	return true;
}

void UPX_WeaponComponent::ServerAttack_Implementation(const FVector_NetQuantize ClientLoc, const FVector_NetQuantize ClientTargetPoint)
{
	if (!ProjectileClass)	return;

	// Calculate Server Camera Location & Rotation
	const auto Pawn = GetOwner<APawn>();
	AController* Controller = Pawn->GetController();
	FVector ServerLoc;
	FRotator ServerRot;
	Controller ? Controller->GetPlayerViewPoint(ServerLoc, ServerRot) : Pawn->GetActorEyesViewPoint(ServerLoc, ServerRot);

	// Validate Client Camera Location & Rotation
	const float Dist = FVector::Dist(ServerLoc, ClientLoc);
	const bool  bCamDistOK = Dist <= 300.f;     // SpringArm Length + Tolerance
	const bool  bForwardOK = FVector::DotProduct((ClientTargetPoint - ClientLoc).GetSafeNormal(), ServerRot.Vector()) > 0.5f; // cos(60°)

	// Use Client Camera Location & Rotation
	if (bCamDistOK && bForwardOK)
	{
		ServerLoc = ClientLoc;
	}

	// Calculate Server Muzzle Location & Rotation
	const auto PX_Character = GetOwner<APX_Character>();
	FVector Bow_R = PX_Character->GetMesh()->GetSocketLocation(TEXT("Bow_R"));
	const FVector MuzzleDir = PX_Character->GetActorForwardVector();
	const FVector MuzzleLoc = Bow_R + MuzzleDir * 100.f + FVector(0.f, 0.f, 85.f);

	// Server Muzzle Trace
	FHitResult MuzzleHitResult;
	FCollisionQueryParams MuzzleParams(SCENE_QUERY_STAT(MuzzleTrace), true, Pawn);
	const bool bMuzzleHit = GetWorld()->LineTraceSingleByChannel(MuzzleHitResult, MuzzleLoc, ClientTargetPoint, ECC_Visibility, MuzzleParams);

	// Setup Final Target Point
	const FVector FinalTargetPoint = bMuzzleHit ? MuzzleHitResult.ImpactPoint : ClientTargetPoint;
	FVector ProjectileDir = (FinalTargetPoint - MuzzleLoc).GetSafeNormal();

	// Setup SpawnParams
	const auto Character = GetOwner();
	auto ProjectileSpawnParams = FActorSpawnParameters();
	ProjectileSpawnParams.Owner = Character;
	ProjectileSpawnParams.Instigator = Cast<APX_Character>(GetOwner());

	// Spawn Projectile
	APX_Projectile* SpawnedActor = GetWorld()->SpawnActor<APX_Projectile>(ProjectileClass, MuzzleLoc, ProjectileDir.Rotation(), ProjectileSpawnParams);
	if (SpawnedActor)
	{
		SpawnedActor->SetOwner(Character);
			
		// To Debug Projectile Camera, Uncomment below line
		//SpawnedActor->ClientCameraTransition(SpawnedActor);
	}

	// Debug Draw
	const float TraceRange = 15000.f;
	const float Duration = 2.0f;
	const float Radius = 4.f;
	const float Thickness = 0.f;

	// 투사체 스폰 지점 (초록 구)
	DrawDebugSphere(GetWorld(), MuzzleLoc, Radius, 16, FColor::Green, false, Duration);

	// 투사체 궤적 (노란 선)
	DrawDebugLine(GetWorld(), MuzzleLoc, ClientTargetPoint, FColor::Yellow, false, Duration, 0, Thickness);

	// Server Target Point (Red)
	DrawDebugSphere(GetWorld(), FinalTargetPoint, Radius, 16, FColor::Red, false, Duration);
}
