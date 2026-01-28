

#include "Med_VAC.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/HordeBaseCharacter.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "ConstructorHelpers.h"

/** ( Overridden )
 * Defines Replicated Props
 *
 * @param
 * @output Lifetime Props as Array.
 * @return void
 */
void AMed_VAC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMed_VAC, IsHealing);
}

/**
 * Constructor for AMed_VAC
 *
 * @param
 * @return
 */
AMed_VAC::AMed_VAC()
{
	Weapon->SetSkeletalMesh(nullptr);
	VACMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Vac Mesh"));
	VACMesh->SetupAttachment(RootComponent);
	VACMesh->SetRelativeScale3D(FVector(.5f, .5f, .5f));

	const ConstructorHelpers::FObjectFinder<UStaticMesh>  VACMeshAsset(TEXT("StaticMesh'/Game/HordeTemplateBP/Assets/Meshes/Misc/SM_VAC.SM_VAC'"));
	if (VACMeshAsset.Succeeded())
	{
		VACMesh->SetStaticMesh(VACMeshAsset.Object);
		VACMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

/** ( Virtual; Overridden )
 * Starts the Healing Process.
 *
 * @param
 * @return
 */
void AMed_VAC::FireFirearm()
{
	if (HasAuthority())
	{
		AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());
		if (PLY && PLY->GetHealth() < 100.f && LoadedAmmo > 0 && !IsHealing)
		{
			IsHealing = true;

			//Simulate Character Reloading so he can't drop the weapon or switch to something.
			PLY->Reloading = true;

			StartHealing();
		}
	}

}

/** ( Server )
 * Finishes the Healing Process and drops the current item.
 *
 * @param
 * @return void
 */
void AMed_VAC::FinishHealing_Implementation()
{
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());
	if (PLY)
	{
		IsHealing = false;
		PLY->AddHealth(15.f);
		LoadedAmmo--;
		PLY->Reloading = false;
		PLY->Inventory->UpdateCurrentItemAmmo(LoadedAmmo);
		PLY->Inventory->ServerDropItem(this);
	}
}

bool AMed_VAC::FinishHealing_Validate()
{
	return true;
}

/** ( Multicast )
 * Plays Healing Animation and finishes healing after Animation is done.
 *
 * @param
 * @return void
 */
void AMed_VAC::StartHealing_Implementation()
{
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());
	if (PLY)
	{
		UAnimMontage* VACHealAnimation = ObjectFromPath<UAnimMontage>(TEXT("AnimMontage'/Game/HordeTemplateBP/Assets/Animations/AM_VAC_Insert.AM_VAC_Insert'"));
		// Fixed: Added null check for GetAnimInstance()
		if (VACHealAnimation && PLY->GetMesh() && PLY->GetMesh()->GetAnimInstance())
		{
			PLY->GetMesh()->GetAnimInstance()->Montage_Play(VACHealAnimation);
			FTimerHandle FinishTimer;
			FTimerDelegate FinishTimerDelegate;
			FinishTimerDelegate.BindLambda([this] {
				FinishHealing();
			});
			GetWorld()->GetTimerManager().SetTimer(FinishTimer, FinishTimerDelegate, VACHealAnimation->CalculateSequenceLength() / 2.f, false);
		}
	}
}

bool AMed_VAC::StartHealing_Validate()
{
	return true;
}
