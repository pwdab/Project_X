// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PX_WeaponComponent.h"
#include "Entity/PX_Projectile.h"
#include "Entity/PX_Character.h"
#include "DrawDebugHelpers.h"
#include "Project_X.h"
#include "Entity/PX_Weapon.h"
#include "Entity/Client/PX_CharacterAnimInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UPX_WeaponComponent::UPX_WeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	//SetIsReplicatedByDefault(true);
	ProjectileClass = APX_Projectile::StaticClass();

}

// Called when the game starts
void UPX_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	// Spawn and Attach Bow to PX_Character
	const auto PX_Character = GetOwner<APX_Character>();
	Weapon = GetWorld()->SpawnActor<APX_Weapon>(FVector::ZeroVector, FRotator::ZeroRotator);
	if ( Weapon ) {
		Weapon->AttachToComponent(PX_Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Bow_L"));
	}	
}

// Called every frame
void UPX_WeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	const auto PX_Character = GetOwner<APX_Character>();
	if ( !PX_Character )	return;

	if ( !PX_Character->IsLocallyControlled() ) return;

	if ( !PX_Character->GetMesh() )	return;

	UPX_CharacterAnimInstance* AnimInstance = Cast<UPX_CharacterAnimInstance>(PX_Character->GetMesh()->GetAnimInstance());
	if ( !AnimInstance )	return;

	USkeletalMeshComponent* WeaponMesh = Weapon->GetMesh();
	if ( !WeaponMesh ) return;

	APlayerController* PlayerController = Cast<APlayerController>(PX_Character->GetController());
	if ( !PlayerController || !PlayerController->PlayerCameraManager )	return;

	if ( !AnimInstance->GetIsAiming() )	return;

	// 소켓 위치
	const FTransform BowStringXf = WeaponMesh->GetSocketTransform("bow_string", RTS_World);
	const FTransform MuzzleXf = WeaponMesh->GetSocketTransform("bow_string_origin", RTS_World);
	//const FVector BowStringLoc = WeaponMesh->GetSocketLocation(TEXT("bow_string"));
	//const FVector MuzzleLoc = WeaponMesh->GetSocketLocation(TEXT("bow_string_origin"));
	const FVector BowStringLoc = BowStringXf.GetLocation();
	const FVector MuzzleLoc = MuzzleXf.GetLocation();
	const FVector BowAimDir = (MuzzleLoc - BowStringLoc).GetSafeNormal();

	// Client Weapon Hit Trance
	const float TraceRange = 15000.f;
	FHitResult HitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponMuzzleTrace), true, GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, BowStringLoc, BowStringLoc + BowAimDir * TraceRange, ECC_PX_CombatTrace, Params);
	const FVector TargetPoint = bHit ? HitResult.ImpactPoint : BowStringLoc + BowAimDir * TraceRange;

	// 활 조준 방향
	const float Duration = 0.f;   // 매 프레임 갱신
	const float Thickness = 0.f;
	const float Radius = 4.f;
	//DrawDebugLine(GetWorld(), BowStringLoc, TargetPoint, FColor::Red, false, Duration, 0, Thickness);

	if ( bHit )
	{
		// Client Camera Target Point (Cyan)
		//DrawDebugSphere(GetWorld(), TargetPoint, Radius, 16, FColor::Red, false, Duration);
	}

	// 카메라
	// Get Camera Location & Rotation
	//const FVector CameraLoc = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FRotator CameraRot = PlayerController->PlayerCameraManager->GetCameraRotation();
	//const FVector CameraDir = CameraRot.Vector().GetSafeNormal();

	// 크로스헤어
	int32 SizeX, SizeY; PlayerController->GetViewportSize(SizeX, SizeY);
	FVector CameraLoc, CameraDir;
	UGameplayStatics::DeprojectScreenToWorld(PlayerController, FVector2D(SizeX * 0.5f, SizeY * 0.5f), CameraLoc, CameraDir);

	// Client Camera Hit Trance
	FHitResult CameraHitResult;
	FCollisionQueryParams CameraParams(SCENE_QUERY_STAT(CameraTrace), true, GetOwner());
	const bool bCameraHit = GetWorld()->LineTraceSingleByChannel(CameraHitResult, CameraLoc, CameraLoc + CameraDir * TraceRange, ECC_PX_CombatTrace, CameraParams);
	const FVector CameraTargetPoint = bCameraHit ? CameraHitResult.ImpactPoint : CameraLoc + CameraDir * TraceRange;
	//DrawDebugLine(GetWorld(), CameraLoc, CameraTargetPoint, FColor::Cyan, false, Duration, 0, Thickness);	// BowStringLoc부터 Intersection까지의 선분

	const FVector CameraUp = CameraRot.RotateVector(FVector::UpVector).GetSafeNormal();   // 카메라 위쪽 (법선으로 사용)
	const FVector CameraRight = CameraRot.RotateVector(FVector::RightVector).GetSafeNormal(); // 카메라 오른쪽 (원 반지름 방향 중 하나)

	const float t = FVector::DotProduct(BowStringLoc - CameraLoc, CameraDir);
	const FVector Intersection = CameraLoc + t * CameraDir;

	// BowStringLoc을 지나는 평면
	const float WeaponRadius = (Intersection - BowStringLoc).Length();
	//DrawDebugCircle(GetWorld(), BowStringLoc, WeaponRadius, 32, FColor::Cyan, false, Duration, 0, Thickness, CameraUp, CameraRight);

	//DrawDebugLine(GetWorld(), CameraLoc, CameraTargetPoint, FColor::Cyan, false, Duration, 0, Thickness);	// BowStringLoc부터 Intersection까지의 선분
	//DrawDebugLine(GetWorld(), BowStringLoc, Intersection, FColor::Cyan, false, Duration, 0, Thickness);	// BowStringLoc부터 Intersection까지의 선분
	//DrawDebugSphere(GetWorld(), Intersection, Radius, 16, FColor::Cyan, false, Duration);				// Intersection

	
	// 무기가 바라볼 지점. 고정 길이
	const float WeaponLookDistance = 1000.f; // 고정 거리
	const FVector FixedTarget = CameraTargetPoint;
	//const FVector FixedTarget = CameraLoc + CameraDir * WeaponLookDistance;

	// bow_string → FixedTarget 방향
	FVector AimDir = (FixedTarget - BowStringLoc).GetSafeNormal();
	if ( !AimDir.Normalize() )
	{
		AimDir = PX_Character->GetActorForwardVector(); // 0 보호
	}
	FRotator AimRot = AimDir.Rotation();

	//const float WeaponRotDistance = FVector::DotProduct((CameraLoc + CameraDir.GetSafeNormal() * WeaponLookDistance - BowStringLoc), CameraDir.GetSafeNormal()) / FVector::DotProduct(AimDir, CameraDir.GetSafeNormal());
	//DrawDebugLine(GetWorld(), BowStringLoc, BowStringLoc + AimDir * WeaponRotDistance, FColor::Magenta, false, Duration, 0, Thickness);

	FHitResult Block;
	FCollisionQueryParams Params2(SCENE_QUERY_STAT(MuzzleToPlane), true, GetOwner());
	const FVector HitOnPlane = BowStringLoc + AimDir * FVector::DotProduct(CameraTargetPoint - BowStringLoc, CameraDir.GetSafeNormal()) / FVector::DotProduct(AimDir, CameraDir.GetSafeNormal());
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Block, BowStringLoc, HitOnPlane, ECC_PX_CombatTrace, Params2);
	const FVector FinalPoint = bBlocked ? Block.ImpactPoint : HitOnPlane;

	//DrawDebugLine(GetWorld(), BowStringLoc, HitOnPlane, FColor::Magenta, false, Duration, 0, Thickness);

	if ( bBlocked )
	{
		//DrawDebugSphere(GetWorld(), FinalPoint, Radius, 16, FColor::Cyan, false, Duration);
	}

	// 5) 로컬 즉시 반영(지연 없는 화면) + 서버 보고(쿨다운/임계치 적용 권장)
	//const uint16 YawB = FRotator::CompressAxisToShort(AimRot.Yaw);
	//const uint16 PitchB = FRotator::CompressAxisToShort(AimRot.Pitch);

	//PX_Character->ServerSetAimRotation(YawB, PitchB);

	//AnimInstance->SetAimTarget(FinalPoint);

	//UE_LOG(LogTemp, Log, TEXT("WeaponComponent TargetPoint : %s"), *FinalPoint.ToString());

	const FVector BowForward = (MuzzleLoc - BowStringLoc).GetSafeNormal(); // 활 ‘실제’ 조준축
	const FVector Desired = (FinalPoint - BowStringLoc).GetSafeNormal(); // 마젠타(원하는 축)

	const float CosAngle = FVector::DotProduct(BowForward, Desired);
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAngle, -1.f, 1.f)));
	//UE_LOG(LogTemp, Warning, TEXT("Bow vs Aim angle: %.3f deg"), AngleDeg);

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
	const FVector CameraLoc = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FRotator CameraRot = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector CameraDir = CameraRot.Vector().GetSafeNormal();

	// Get Muzzle Location
	const FVector MuzzleLoc = Weapon->GetMesh()->GetSocketLocation(TEXT("bow_string"));

	//UE_LOG(LogTemp, Log, TEXT("Client Loc : %s, Client Dir : %s"), *WorldLoc.ToString(), *WorldDir.ToString());

	// Debug Draw
	const float TraceRange = 15000.f;
	const float Duration = 2.0f;
	const float Radius = 4.f;
	const float Thickness = 0.f;

	// Client Camera Hit Trance
	FHitResult HitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CrosshairTrace), true, GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLoc, CameraLoc + CameraDir * TraceRange, ECC_PX_CombatTrace, Params);
	/*
	FCollisionObjectQueryParams HitQuery;
	//HitQuery.AddObjectTypesToQuery(ECC_Pawn);					// 캐릭터
	HitQuery.AddObjectTypesToQuery(ECC_PX_SkelMesh);			// PX_SkelMesh
	HitQuery.AddObjectTypesToQuery(ECC_WorldStatic);			// 벽/지형
	HitQuery.AddObjectTypesToQuery(ECC_WorldDynamic);			// 문/움직이는 오브젝트
	HitQuery.AddObjectTypesToQuery(ECC_PhysicsBody);
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, CameraLoc, CameraLoc + CameraDir * TraceRange, HitQuery, Params);
	*/
	const FVector TargetPoint = bHit ? HitResult.ImpactPoint : CameraLoc + CameraDir * TraceRange;

	// Client Camera Line Trace (Cyan)
	DrawDebugLine(GetWorld(), CameraLoc, TargetPoint, FColor::Cyan, false, Duration, 0, Thickness);

	// Client Camera Target Point (Cyan)
	DrawDebugSphere(GetWorld(), TargetPoint, Radius, 16, FColor::Cyan, false, Duration);

	// Server Attack
	ServerAttack(MuzzleLoc, CameraLoc, TargetPoint);
}

bool UPX_WeaponComponent::ServerAttack_Validate(const FVector_NetQuantize ClientMuzzleLoc, const FVector_NetQuantize ClientCamLoc, const FVector_NetQuantize ClientDir)
{
	return true;
}

void UPX_WeaponComponent::ServerAttack_Implementation(const FVector_NetQuantize ClientMuzzleLoc, const FVector_NetQuantize ClientCamLoc, const FVector_NetQuantize ClientTargetPoint)
{
	if (!ProjectileClass)	return;

	// Calculate Server Camera Location & Rotation
	const auto Pawn = GetOwner<APawn>();
	AController* Controller = Pawn->GetController();
	FVector ServerCamLoc;
	FRotator ServerCamRot;
	Controller ? Controller->GetPlayerViewPoint(ServerCamLoc, ServerCamRot) : Pawn->GetActorEyesViewPoint(ServerCamLoc, ServerCamRot);

	// Validate Client Camera Location & Rotation
	const float Dist = FVector::Dist(ServerCamLoc, ClientCamLoc);
	const bool  bCamDistOK = Dist <= 300.f;     // SpringArm Length + Tolerance
	const bool  bForwardOK = FVector::DotProduct((ClientTargetPoint - ClientCamLoc).GetSafeNormal(), ServerCamRot.Vector()) > 0.5f; // cos(60°)

	// Use Client Camera Location & Rotation
	if (bCamDistOK && bForwardOK)
	{
		ServerCamLoc = ClientCamLoc;
	}

	// Calculate Server Muzzle Location & Rotation
	//const auto PX_Character = GetOwner<APX_Character>();
	//FVector Bow_R = PX_Character->GetMesh()->GetSocketLocation(TEXT("Bow_R"));
	//const FVector MuzzleDir = PX_Character->GetActorForwardVector();
	//const FVector MuzzleLoc = Bow_R + MuzzleDir * 100.f + FVector(0.f, 0.f, 85.f);
	const FVector MuzzleDir = ClientCamLoc - ClientMuzzleLoc;
	const FVector MuzzleLoc = ClientMuzzleLoc;
	
	// Server Muzzle Trace
	FHitResult MuzzleHitResult;
	FCollisionQueryParams MuzzleParams(SCENE_QUERY_STAT(MuzzleTrace), true, Pawn);
	const bool bMuzzleHit = GetWorld()->LineTraceSingleByChannel(MuzzleHitResult, ClientMuzzleLoc, ClientTargetPoint, ECC_PX_CombatTrace, MuzzleParams);
	/*
	FCollisionObjectQueryParams MuzzleHitQuery;
	//MuzzleHitQuery.AddObjectTypesToQuery(ECC_Pawn);               // 캐릭터
	MuzzleHitQuery.AddObjectTypesToQuery(ECC_PX_SkelMesh);			// PX_SkelMesh
	MuzzleHitQuery.AddObjectTypesToQuery(ECC_WorldStatic);          // 벽/지형
	MuzzleHitQuery.AddObjectTypesToQuery(ECC_WorldDynamic);         // 문/움직이는 오브젝트
	MuzzleHitQuery.AddObjectTypesToQuery(ECC_PhysicsBody);
	const bool bMuzzleHit = GetWorld()->LineTraceSingleByObjectType(MuzzleHitResult, ClientMuzzleLoc, ClientTargetPoint, MuzzleHitQuery, MuzzleParams);
	*/
	// Setup Final Target Point
	const FVector FinalTargetPoint = bMuzzleHit ? MuzzleHitResult.ImpactPoint : ClientTargetPoint;
	FVector ProjectileDir = (FinalTargetPoint - MuzzleLoc).GetSafeNormal();

	// Setup SpawnParams
	const auto Character = GetOwner();
	auto ProjectileSpawnParams = FActorSpawnParameters();
	ProjectileSpawnParams.Owner = Character;
	ProjectileSpawnParams.Instigator = Cast<APX_Character>(GetOwner());

	// Spawn Projectile
	APX_Projectile* SpawnedActor = GetWorld()->SpawnActor<APX_Projectile>(ProjectileClass, MuzzleLoc + ProjectileDir * 100.0f, ProjectileDir.Rotation(), ProjectileSpawnParams);
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

	// 투사체 스폰 지점 (노란 구)
	DrawDebugSphere(GetWorld(), MuzzleLoc, Radius, 16, FColor::Yellow, false, Duration);

	// 투사체 궤적 (노란 선)
	DrawDebugLine(GetWorld(), MuzzleLoc, ClientTargetPoint, FColor::Yellow, false, Duration, 0, Thickness);

	// Server Target Point (Yellow)
	DrawDebugSphere(GetWorld(), FinalTargetPoint, Radius, 16, FColor::Yellow, false, Duration);
}
