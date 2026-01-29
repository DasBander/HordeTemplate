// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "HordeCharacterAnimInstance.h"

/**
 * @file HordeCharacterAnimInstance.cpp
 * @brief Implementation of character animation instance with state updates per frame
 * @author Marc Fraedrich
 */

/**
 *	Constructor for UHordeCharacterAnimInstance
 *
 * @param
 * @return
 */
void UHordeCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Character = Cast<AHordeBaseCharacter>(TryGetPawnOwner());
}

/** ( Virtual ; Overridden)
 *	Sets the Character Animation Variables.
 *
 * @param DeltaSeconds
 * @return void
 */
void UHordeCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character)
	{
		bIsInAir = Character->GetCharacterMovement()->IsFalling();
		Speed = Character->GetVelocity().Size();
		AnimationType = Character->AnimMode;

		// Use GetBaseAimRotation() which properly handles both local and remote players
		// This internally uses GetControlRotation() for local, and replicated values for remote
		float AimPitch = Character->GetBaseAimRotation().Pitch;

		// Normalize pitch to -180 to 180 range (handles 0-360 wrapping)
		AimPitch = FMath::UnwindDegrees(AimPitch);

		// Convert to aim offset range:
		// In UE, negative pitch = looking up, positive pitch = looking down
		// For aim offset blend space, we typically want the inverse
		float NewAimRotation = -AimPitch;

		// Clamp to valid aim offset range
		NewAimRotation = FMath::Clamp(NewAimRotation, -90.f, 90.f);

		// For locally controlled characters, use the value directly (already smooth)
		// For remote characters, interpolate to smooth out network jitter
		if (Character->IsLocallyControlled())
		{
			AimRotation = NewAimRotation;
		}
		else
		{
			// Interpolate smoothly towards the target rotation for remote players
			TargetAimRotation = NewAimRotation;
			AimRotation = FMath::FInterpTo(AimRotation, TargetAimRotation, DeltaSeconds, AimInterpSpeed);
		}
	}
}
