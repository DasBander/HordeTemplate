

#include "BaseFirearm.h"
#include "Gameplay/GameplayStructures.h"
#include "Inventory/InventoryHelpers.h"
#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeBaseController.h"

/**
 * Constructor for ABaseFirearm
 *
 * @param
 * @return
 */
ABaseFirearm::ABaseFirearm()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetReplicates(true);

	/*
	Root Component
	*/
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Weapon->SetupAttachment(RootComponent);

	/*
	Particle Component ( For Muzzle Flash )
	*/
	MuzzleFlash = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Muzzle Flash"));
	MuzzleFlash->SetupAttachment(Weapon);
	MuzzleFlash->SetAutoActivate(false);

	/*
	Audio Component ( For Weapon Sound )
	*/
	WeaponSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Weapon Sound"));
	WeaponSound->SetupAttachment(Weapon);
	WeaponSound->SetAutoActivate(false);



}

/** ( Virtual ) 
 * Fires Weapon, spawns projectile and removes ammo. Also Plays Camera Shake.
 *
 * @param
 * @return void
 */
void ABaseFirearm::FireFirearm()
{
	if (LoadedAmmo > 0)
	{
		FItem CurrentWeaponItem = UInventoryHelpers::FindItemByID(FName(*WeaponID));

		// Fixed: Check if ProjectileClass is valid before spawning
		if (!CurrentWeaponItem.ProjectileClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("FireFirearm: ProjectileClass is null for weapon %s"), *WeaponID);
			return;
		}

		PlayFirearmFX();
		LoadedAmmo = FMath::Clamp<int32>((LoadedAmmo - 1), 0, 9999);

		FTransform SpawnTransform;
		FVector EyeViewPoint;
		FRotator EyeRotation;
		GetOwnerEyePoint(false, EyeViewPoint, EyeRotation);
		SpawnTransform.SetLocation(EyeViewPoint);
		SpawnTransform.SetRotation(EyeRotation.Quaternion());
		ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(CurrentWeaponItem.ProjectileClass, SpawnTransform, FActorSpawnParameters());
		if (Projectile)
		{
			Projectile->SetOwner(GetOwner());
		}

		AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());
		if (PLY)
		{
			AHordeBaseController* PC = Cast<AHordeBaseController>(PLY->GetController());
			if (PC && CurrentWeaponItem.VisualRecoilClass)
			{
				//PC->ClientPlayCameraShake(CurrentWeaponItem.VisualRecoilClass, 1.f, ECameraAnimPlaySpace::CameraLocal, FRotator::ZeroRotator);
			}
			PLY->Inventory->UpdateCurrentItemAmmo(LoadedAmmo);
		}
	}

	else {
		//No Ammo
	}
}


/** ( Overridden )
 * Defines Replicated Props
 *
 * @param
 * @output Lifetime Props as Array.
 * @return void
 */
void ABaseFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Optimization: Ammo count and fire mode only needed by owner for HUD display
	DOREPLIFETIME_CONDITION(ABaseFirearm, LoadedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(ABaseFirearm, WeaponID);
	DOREPLIFETIME(ABaseFirearm, ProjectileFromMuzzle);
	DOREPLIFETIME_CONDITION(ABaseFirearm, FireMode, COND_OwnerOnly);
}

/** ( Multicast )
 * Plays Muzzle Flash and Weapon Sound.
 *
 * @param
 * @return void
 */
void ABaseFirearm::PlayFirearmFX_Implementation()
{
	MuzzleFlash->Activate(true);
	WeaponSound->Play();
}

bool ABaseFirearm::PlayFirearmFX_Validate()
{
	return true;
}

/** ( Server )
 * Fires Weapon on Server.
 *
 * @param
 * @return void
 */
void ABaseFirearm::ServerFireFirearm_Implementation()
{
	FireFirearm();
}

bool ABaseFirearm::ServerFireFirearm_Validate()
{
	return true;
}

/** ( Server )
 * Toggles the current Fire mode of weapon.
 *
 * @param
 * @return void
 */
void ABaseFirearm::ServerToggleFireMode_Implementation()
{
	EFireMode CurrentFireMode = EFireMode(FireMode);
	FItem CurrentWeaponItem = UInventoryHelpers::FindItemByID(FName(*WeaponID));
	int32 CurrentSelectedIndex = CurrentWeaponItem.FireModes.Find(CurrentFireMode);
	if (CurrentSelectedIndex == (CurrentWeaponItem.FireModes.Num() - 1))
	{
		FireMode = (uint8)CurrentWeaponItem.FireModes[0];
	}
	else {
		FireMode = (uint8)CurrentWeaponItem.FireModes[CurrentSelectedIndex + 1];
	}
}

bool ABaseFirearm::ServerToggleFireMode_Validate()
{
	return true;
}

/**
 * Returns the Center Screen Rotation and View Location or the Weapon Muzzle Location and Rotation.
 *
 * @param bool if Location from Weapons Muzzle Flash
 * @output View Location and Rotation
 * @return void
 */
void ABaseFirearm::GetOwnerEyePoint(bool LocationFromWeapon, FVector& ViewLocation, FRotator& ViewRotation)
{
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());
	if (PLY)
	{
		FVector CopyLocationViewPoint;
		FRotator CopyRotationViewPoint;
		PLY->GetActorEyesViewPoint(CopyLocationViewPoint, CopyRotationViewPoint);
		ViewLocation = (LocationFromWeapon) ? Weapon->GetSocketLocation(FName("muzzle")) : PLY->GetCamera()->GetComponentLocation();
		ViewRotation = (LocationFromWeapon) ? Weapon->GetSocketRotation(FName("muzzle")) : CopyRotationViewPoint;
	}
	else {
		ViewLocation = FVector::ZeroVector;
		ViewRotation = FRotator::ZeroRotator;
	}

}

/** ( Virtual; Overridden )
 * Begin Play.
 *
 * @param
 * @return void
 */
void ABaseFirearm::BeginPlay()
{
	Super::BeginPlay();
	
}


