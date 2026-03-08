// Project_X - Bow driver implementation

#include "PX_BowAnimDriver.h"

#include "DrawDebugHelpers.h"
#include "Entity/PX_Character.h"
#include "Entity/PX_Weapon.h"
#include "Entity/Client/PX_CharacterAnimInstance.h"
#include "Kismet/GameplayStatics.h"

/*
void UPX_BowAnimDriver::OnTick_Implementation(float DeltaSeconds, const FPXWeaponDriverContext& Context)
{
    APX_Character* PX_Character = Context.Character.Get();
    APX_Weapon* Weapon = Context.Weapon.Get();
    if (!PX_Character || !Weapon) return;
    if (!PX_Character->IsLocallyControlled()) return;
    if (!PX_Character->GetMesh()) return;

    UPX_CharacterAnimInstance* AnimInstance = Cast<UPX_CharacterAnimInstance>(PX_Character->GetMesh()->GetAnimInstance());
    if (!AnimInstance) return;
    if (!AnimInstance->GetIsAiming()) return; // only run while aiming

    USkeletalMeshComponent* WeaponMesh = Weapon->GetMesh();
    if (!WeaponMesh) return;

    APlayerController* PlayerController = Cast<APlayerController>(PX_Character->GetController());
    if (!PlayerController || !PlayerController->PlayerCameraManager) return;

    // --- Weapon sockets ------------------------------------------------
    const FVector BowStringLoc = WeaponMesh->GetSocketTransform(BowStringSocket, RTS_World).GetLocation();
    const FVector MuzzleLoc = WeaponMesh->GetSocketTransform(BowMuzzleSocket, RTS_World).GetLocation();
    const FVector BowAimDir = (MuzzleLoc - BowStringLoc).GetSafeNormal();

    // Weapon forward trace (from bow string towards bow muzzle direction)
    FHitResult WeaponHit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(BowWeaponTrace), true, PX_Character);
    const bool bWeaponHit = PX_Character->GetWorld()->LineTraceSingleByChannel(
        WeaponHit,
        BowStringLoc,
        BowStringLoc + BowAimDir * TraceRange,
        ECC_PX_CombatTrace,
        Params
    );

    const FVector WeaponTarget = bWeaponHit ? WeaponHit.ImpactPoint : (BowStringLoc + BowAimDir * TraceRange);

    // --- Camera / crosshair ray ---------------------------------------
    int32 SizeX = 0, SizeY = 0;
    PlayerController->GetViewportSize(SizeX, SizeY);

    FVector CameraLoc, CameraDir;
    UGameplayStatics::DeprojectScreenToWorld(PlayerController, FVector2D(SizeX * 0.5f, SizeY * 0.5f), CameraLoc, CameraDir);
    CameraDir = CameraDir.GetSafeNormal();

    FHitResult CameraHit;
    FCollisionQueryParams CamParams(SCENE_QUERY_STAT(BowCameraTrace), true, PX_Character);
    const bool bCameraHit = PX_Character->GetWorld()->LineTraceSingleByChannel(
        CameraHit,
        CameraLoc,
        CameraLoc + CameraDir * TraceRange,
        ECC_PX_CombatTrace,
        CamParams
    );

    const FVector CameraTarget = bCameraHit ? CameraHit.ImpactPoint : (CameraLoc + CameraDir * TraceRange);

    // --- Choose target & compute rotation -----------------------------
    // This is intentionally conservative: use camera target (what player wants)
    // but ensure we don't aim backwards.
    FVector AimDir = (CameraTarget - BowStringLoc);
    if (!AimDir.Normalize())
    {
        AimDir = PX_Character->GetActorForwardVector();
    }

    // If the camera direction and AimDir are too opposed, fall back to weapon forward.
    const float ForwardDot = FVector::DotProduct(AimDir, CameraDir);
    if (ForwardDot < 0.1f)
    {
        AimDir = BowAimDir;
    }

    LastTargetPoint = CameraTarget;
    LastAimRotation = AimDir.Rotation();

    if (bDrawDebug)
    {
        const float Duration = 0.f;
        DrawDebugLine(PX_Character->GetWorld(), BowStringLoc, WeaponTarget, FColor::Red, false, Duration, 0, 0.f);
        DrawDebugLine(PX_Character->GetWorld(), CameraLoc, CameraTarget, FColor::Cyan, false, Duration, 0, 0.f);
        DrawDebugSphere(PX_Character->GetWorld(), CameraTarget, 4.f, 12, FColor::Cyan, false, Duration);
    }
}
*/
