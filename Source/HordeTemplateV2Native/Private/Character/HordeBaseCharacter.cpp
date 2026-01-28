

#include "HordeBaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Inventory/InventoryHelpers.h"
#include "Gameplay/HordeGameMode.h"
#include "Weapons/BaseFirearm.h"
#include "HUD/Widgets/PlayerHeadDisplay.h"
#include "HordePlayerState.h"
#include "AIModule/Classes/Perception/AISense_Sight.h"
#include "FX/Camera/CameraShake_Damage.h"



/** ( Virtual; Overridden )
 *	Sets given variables as Replicated.
 *
 * @param TArray - FLifetimeProperty ( Out Lifetime Props ) const
 * @return void
 */
void AHordeBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHordeBaseCharacter, Stamina);
	DOREPLIFETIME(AHordeBaseCharacter, Health);
	DOREPLIFETIME(AHordeBaseCharacter, AnimMode);
	DOREPLIFETIME(AHordeBaseCharacter, IsSprinting);
	DOREPLIFETIME(AHordeBaseCharacter, IsDead);
	DOREPLIFETIME(AHordeBaseCharacter, Reloading);
	DOREPLIFETIME(AHordeBaseCharacter, CurrentSelectedFirearm);
}


/**
 *	Constructor
 *
 * @param
 * @return
 */
AHordeBaseCharacter::AHordeBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;


	const ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerMeshAsset(TEXT("SkeletalMesh'/Game/HordeTemplateBP/Assets/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (PlayerMeshAsset.Succeeded()) {
		GetMesh()->SetSkeletalMesh(PlayerMeshAsset.Object);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f).Quaternion());
		GetMesh()->SetCollisionProfileName(FName(TEXT("Ragdoll")));
	}

	
	// static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(TEXT("/Game/HordeTemplateBP/Assets/Mannequin/Animations/ABP_ThirdPerson"));
	// if (AnimBP.Succeeded() && GetMesh())
	// {
	// 	GetMesh()->SetAnimInstanceClass(AnimBP.Class);
	// }

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 90.f;
	CameraBoom->SetRelativeLocation(FVector(0.f, 49.f, 70.f));
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritRoll = true;
	CameraBoom->bInheritYaw = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;


	PlayerNameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Player 3D Name Widget"));
	PlayerNameWidget->SetupAttachment(RootComponent);
	PlayerNameWidget->SetDrawAtDesiredSize(true);
	PlayerNameWidget->SetDrawSize(FVector2D(400.f, 50.f));
	PlayerNameWidget->SetWidgetSpace(EWidgetSpace::Screen);
	PlayerNameWidget->SetPivot(FVector2D(0.f, 0.f));
	PlayerNameWidget->SetRelativeLocation(FVector(0.f, 6.f, 81.f));

	static ConstructorHelpers::FClassFinder<UUserWidget> NameWidgetAsset(TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Misc/WBP_3d_PlayerView.WBP_3d_PlayerView_C'"));
	if (NameWidgetAsset.Succeeded())
	{
		PlayerNameWidget->SetWidgetClass(NameWidgetAsset.Class);
	}

	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("AI Stimuli Source"));
	StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());


	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Player Inventory"));
	


}

void AHordeBaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// If a BP child swapped/removed the pointer, fall back to whatever exists
	if (!Inventory)
	{
		Inventory = FindComponentByClass<UInventoryComponent>();
	}

	if (Inventory)
	{
		// Ensure component replication (BP can toggle it off in editor)
		Inventory->SetIsReplicated(true);

		// Bind once (ctor binds can be skipped in BP construction)
		if (!Inventory->OnActiveItemChanged.IsAlreadyBound(this, &AHordeBaseCharacter::ActiveItemChanged))
		{
			Inventory->OnActiveItemChanged.AddDynamic(this, &AHordeBaseCharacter::ActiveItemChanged);
		}

		// Ensure DataTable is set if the BP didn’t assign one
		if (!Inventory->DataTable)
		{
			static ConstructorHelpers::FObjectFinder<UDataTable> InvDT(INVENTORY_DATATABLE_PATH);
			if (InvDT.Succeeded())
			{
				Inventory->DataTable = InvDT.Object;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[CHAR] Inventory DataTable missing for character! That's really really bad!"))
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CHAR] Inventory component missing on %s"), *GetName());
	}
}

/**
 * Hacky methode for GetBaseAimRotation() (Pitch).
 * Pitch is somehow 2 instead of 0 by default which means it doesn't use the replicated value for it even if it should at that point inside the anim bp.
 * That's why we get it here and transform it into the value we need. We have no idea why the pitch is by default 2. If we find a fix for it we can remove that.
 *
 * @param
 * @return Remote View Pitch of Character Camera.
 */
float AHordeBaseCharacter::GetRemotePitch()
{
	return RemoteViewPitch * 360.0f / 255.0f;
}


/** ( Virtual; Overridden )
 *	Starts Interaction & HeadDisplay Trace Timer. Also Updates on server the HeadDisplayWidget.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	
	
}

void AHordeBaseCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// Called on the owning client after possession & input are ready
	if (!IsLocallyControlled())
	{
		return;
	}

	// Start local-only periodic tasks
	GetWorld()->GetTimerManager().SetTimer(
		InteractionDetectionTimer,
		this, &AHordeBaseCharacter::InteractionDetection,
		0.05f, /*bLoop=*/ true);

	// If this widget exists locally, you can alter it now
	if (IsValid(PlayerNameWidget))
	{
		// Your original code cleared the widget — keep if intended
		PlayerNameWidget->SetWidget(nullptr);
	}

	GetWorld()->GetTimerManager().SetTimer(
		HeadDisplayTraceTimer,
		this, &AHordeBaseCharacter::HeadDisplayTrace,
		0.05f, /*bLoop=*/ true);
}

void AHordeBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server-side initial UI/data push (PlayerState is usually valid here on the server)
	const APlayerState* PS = GetPlayerState();
	const FString Name = PS ? PS->GetPlayerName() : TEXT("");
	UpdateHeadDisplayWidget(Name, Health);
}

void AHordeBaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client reacts when PlayerState arrives/changes (handles late joiners etc.)
	const APlayerState* PS = GetPlayerState();
	const FString Name = PS ? PS->GetPlayerName() : TEXT("");
	UpdateHeadDisplayWidget(Name, Health);
}


/** ( Server )
 *	Interacts with given Actor and makes an Interface Call. Plays Interaction Sound as well.
 *
 * @param AActor ( Actor To Interact With )
 * @return void
 */
void AHordeBaseCharacter::ServerInteract_Implementation(AActor* ActorToInteractWith)
{
	if (ActorToInteractWith)
	{
		IInteractionInterface::Execute_Interact(ActorToInteractWith, this);
		FInteractionInfo InterInf = IInteractionInterface::Execute_GetInteractionInfo(ActorToInteractWith);
		if (InterInf.InteractionSound)
		{
			PlaySoundOnAllClients(InterInf.InteractionSound, GetMesh()->GetComponentLocation());
		}
	}
}

bool AHordeBaseCharacter::ServerInteract_Validate(AActor* ActorToInteractWith)
{
	return true;
}

/** ( Multicast )
 *	Plays sound on all clients on given Location.
 *
 * @param The Sound Cue to be played and the location.
 * @return void
 */
void AHordeBaseCharacter::PlaySoundOnAllClients_Implementation(USoundCue* Sound, FVector Location)
{
	if (Sound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location, FRotator::ZeroRotator, 1.f, 1.f, 0.f, nullptr, nullptr, true);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Could not PlaySoundOnAllClients! SoundCue not valid."));
	}
	
}

bool AHordeBaseCharacter::PlaySoundOnAllClients_Validate(USoundCue* Sound, FVector Location)
{
	return true;
}


/** ( Virtual; Overridden )
 *	Function to receive damage. Removes Health depending on the Damage. Runs CharacterDie() if Health is <= 0.f . Plays Camerashake on Client and Damage Sound on all clients.
 *
 * @param  float ( Damage ), FDamageEvent ( Damage Event ), AController ( Event Instigator ), AActor ( Damage Causer )
 * @return float ( Health - Damage )
 */
float AHordeBaseCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (HasAuthority())
	{
		if (!IsDead)
		{
			if (RemoveHealth(Damage))
			{
				CharacterDie();
			}
			else {
				APlayerController* PC = Cast<APlayerController>(GetController());
				if (PC)
				{
					//PC->ClientPlayCameraShake(UCameraShake_Damage::StaticClass(), 2.f);
				}
				USoundCue* DamageSound = ObjectFromPath<USoundCue>(FName(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_PlayerDamage.A_PlayerDamage'")));
				if (DamageSound)
				{
					PlaySoundOnAllClients(DamageSound, GetMesh()->GetComponentLocation());
				}
			}
		}
	}
	
	return Health;
}

/** ( Multicast )
 *	Plays Character Animation on all Clients.
 *
 * @param The Animation that should be played on character.
 * @return void
 */
void AHordeBaseCharacter::PlayAnimationAllClients_Implementation(class UAnimMontage* Montage)
{
	// Use the passed Montage parameter, fallback to default reload animation if null
	UAnimMontage* MontageToPlay = Montage;
	if (!MontageToPlay)
	{
		MontageToPlay = ObjectFromPath<UAnimMontage>(TEXT("AnimMontage'/Game/HordeTemplateBP/Assets/Animations/AM_Reload_Rifle.AM_Reload_Rifle'"));
	}

	if (MontageToPlay && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay, 1.f, EMontagePlayReturnType::Duration, 0.f, false);
	}
}

bool AHordeBaseCharacter::PlayAnimationAllClients_Validate(class UAnimMontage* Montage)
{
	return true;
}


/**
 *	Updates Health and returns if the Character is dead or not.
 *
 * @param Health to Remove
 * @return bool ( true if Health is <= 0.f )
 */
bool AHordeBaseCharacter::RemoveHealth(float HealthToRemove)
{
	Health = FMath::Clamp<float>((Health - HealthToRemove), 0.f, 100.f);
	const APlayerState* PS = GetPlayerState();
	const FString PlayerName = PS ? PS->GetPlayerName() : TEXT("");
	UpdateHeadDisplayWidget(PlayerName, Health);
	return (Health <= 0.f);
}


/**
 *	Drops Current Item and lets Character Die. Sets lifespan to 20 seconds and enables physics on character. Spawns Spectator on server to possess with. 
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::CharacterDie()
{
	IsDead = true;
	if (CurrentSelectedFirearm)
	{
		Inventory->ServerDropItem(CurrentSelectedFirearm);
	}
	AHordePlayerState* PS = Cast<AHordePlayerState>(GetPlayerState());
	if (PS)
	{
		PS->bIsDead = true;
	}

	AHordeGameMode* GM = Cast<AHordeGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			GM->SpawnSpectator(PC);
		}
	}
	RagdollPlayer();
	SetLifeSpan(20.f);
}

/**
 *	Starts Interaction with Last Interaction Actor that you where facing.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::StartInteraction()
{
	if (!IsInteracting && LastInteractionActor)
	{
		InteractionProgress = 0.f;
		FInteractionInfo InteractionInfo = IInteractionInterface::Execute_GetInteractionInfo(LastInteractionActor);
		if (InteractionInfo.AllowedToInteract)
		{
			GetHUD()->GetHUDWidget()->IsInteracting = true;
			IsInteracting = true;
			TargetInteractionTime = InteractionInfo.InteractionTime;
			GetWorld()->GetTimerManager().SetTimer(InteractionTimer, this, &AHordeBaseCharacter::ProcessInteraction, .01f, true);
		}
	}
}


/**
 *	Stops Interaction.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::StopInteraction()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(InteractionTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(InteractionTimer);
	}
	IsInteracting = false;
	GetHUD()->GetHUDWidget()->IsInteracting = false;
	InteractionTime = 0.f;
	TargetInteractionTime = 0.f;
	InteractionProgress = 0.f;
}

/**
 *	Removes time from the Interaction. If Character is Dead, Not Allowed to Interact or Last Interaction Actor not valid the Interaction will be stopped. If Interaction Time is over calls Interact on Actor.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::ProcessInteraction()
{
	if (LastInteractionActor)
	{
		if (InteractionTime >= TargetInteractionTime)
		{
			StopInteraction();
			if (LastInteractionActor)
			{
				ServerInteract(LastInteractionActor);
			}
		}
		else {
			InteractionTime = InteractionTime + 0.01f;
			InteractionProgress = FMath::GetMappedRangeValueClamped(FVector2D(0.f, TargetInteractionTime), FVector2D(0.f, 100.f), InteractionTime);
		}
		FInteractionInfo InteractionInfo = IInteractionInterface::Execute_GetInteractionInfo(LastInteractionActor);
		if (!InteractionInfo.AllowedToInteract || IsDead)
		{
			StopInteraction();
		}
	}
	else {
		StopInteraction();
	}
	
}


/**
 *	Trace for Head Display Widget. Gets Information for other Players.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::HeadDisplayTrace()
{
	FHitResult HitResult(ForceInit);

	const TArray<AActor*> IgnoringActors = {this};


	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), FollowCamera->GetComponentLocation(), FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 500.f), 64.f, ETraceTypeQuery::TraceTypeQuery3, false, IgnoringActors, EDrawDebugTrace::None, HitResult, true))
	{
		AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(HitResult.GetActor());
		if (PLY)
		{
			LastFacingCharacter = PLY;
			UPlayerHeadDisplay* PLYWidget = Cast<UPlayerHeadDisplay>(LastFacingCharacter->GetHeadDisplayWidget());
			if (PLYWidget)
			{
				PLYWidget->OnShowWidgetDelegate.Broadcast();
			}
		}
		else {
			if (LastFacingCharacter)
			{
				UPlayerHeadDisplay* PLYWidget = Cast<UPlayerHeadDisplay>(LastFacingCharacter->GetHeadDisplayWidget());
				if (PLYWidget)
				{
					PLYWidget->OnHideWidgetDelegate.Broadcast();
					LastFacingCharacter = nullptr;
				}
			}
		}
	}
	else {
		if (LastFacingCharacter)
		{
			UPlayerHeadDisplay* PLYWidget = Cast<UPlayerHeadDisplay>(LastFacingCharacter->GetHeadDisplayWidget());
			if (PLYWidget)
			{
				PLYWidget->OnHideWidgetDelegate.Broadcast();
				LastFacingCharacter = nullptr;
			}
		}
	}
}

/**
 *	Trace for Items to Interact with. Also Updates HUD with text to Display.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::InteractionDetection()
{
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("ItemTrace")), true, GetOwner());
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(GetOwner());
	FHitResult HitResult(ForceInit);
	

	const TArray<AActor*> IgnoringActors;

	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), FollowCamera->GetComponentLocation(), FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 300.f), 5.f, ETraceTypeQuery::TraceTypeQuery1, false, IgnoringActors, EDrawDebugTrace::None, HitResult, true))
	{
		if (HitResult.GetActor())
		{
			if (HitResult.GetActor()->Implements<UInteractionInterface>())
			{
				LastInteractionActor = HitResult.GetActor();
				FInteractionInfo NewInfo = IInteractionInterface::Execute_GetInteractionInfo(LastInteractionActor);
				AHordeBaseHUD* HUD = GetHUD();
				UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());

				if (HUD && HUD->GetHUDWidget() && Settings)
				{
					TArray<FInputActionKeyMapping> UseKeys;
					Settings->GetActionMappingByName("Use", UseKeys);
					FString InteractionText;
					if (UseKeys.IsValidIndex(0) && !NewInfo.HideKeyInText)
					{
						InteractionText = "[" + UseKeys[0].Key.GetDisplayName().ToString() + "] " + NewInfo.InteractionText.ToString();
					}
					else
					{
						InteractionText = NewInfo.InteractionText.ToString();
					}
					HUD->GetHUDWidget()->OnSetInteractionText.Broadcast(FText::FromString(*InteractionText));
				}

			}
			else {
				LastInteractionActor = nullptr;
				AHordeBaseHUD* HUD = GetHUD();
				if (HUD && HUD->GetHUDWidget())
				{
					HUD->GetHUDWidget()->OnHideInteractionText.Broadcast();
				}
			}
		}
		
	}
	else {
		LastInteractionActor = nullptr;
		AHordeBaseHUD* HUD = GetHUD();
		if (HUD && HUD->GetHUDWidget())
		{
			HUD->GetHUDWidget()->OnHideInteractionText.Broadcast();
		}
	}
}


/** ( Multicast )
 *	Enables Physics for Character Mesh on all Clients.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::RagdollPlayer_Implementation()
{
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionProfileName(FName(TEXT("DeadAI")));
}

bool AHordeBaseCharacter::RagdollPlayer_Validate()
{
	return true;
}


/** ( Multicast )
 *	Updates Characters Movement Speed on all clients.
 *
 * @param New Movement Speed
 * @return void
 */
void AHordeBaseCharacter::UpdatePlayerMovementSpeed_Implementation(float NewMovementSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewMovementSpeed;
}

bool AHordeBaseCharacter::UpdatePlayerMovementSpeed_Validate(float NewMovementSpeed)
{
	return true;
}


/** ( Multicast )
 *	Updates Player Head Display Information on all clients.
 *
 * @param The Current Player Name and Player Health
 * @return void
 */
void AHordeBaseCharacter::UpdateHeadDisplayWidget_Implementation(const FString& PlayerName, float PlayerHealth)
{
	UPlayerHeadDisplay* PHD = Cast<UPlayerHeadDisplay>(PlayerNameWidget->GetUserWidgetObject());
	if (PHD)
	{
		PHD->PlayerName = PlayerName;
		PHD->Health = PlayerHealth;
	}
}

bool AHordeBaseCharacter::UpdateHeadDisplayWidget_Validate(const FString& PlayerName, float PlayerHealth)
{
	return true;
}


/** ( Server )
 *	Starts Sprinting Timer and Decreases Stamina. Also Updates Movement Speed on all Clients
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::ServerStartSprinting_Implementation()
{
	if (!IsSprinting && GetVelocity().SizeSquared() > 10.f && GetCharacterMovement()->IsMovingOnGround())
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(TimerIncreaseStamina))
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerIncreaseStamina);
		}
		GetWorld()->GetTimerManager().SetTimer(TimerDecreaseStamina, this, &AHordeBaseCharacter::DecreaseStamina, .1f, true);
		IsSprinting = true;
		UpdatePlayerMovementSpeed(600.f);
	}
}

bool AHordeBaseCharacter::ServerStartSprinting_Validate()
{
	return true;
}


/** ( Server )
 *	Stops Sprinting Timer and starts Increasing Stamina. Updates Player Movement Speed on all Clients.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::ServerStopSprinting_Implementation()
{
	if(IsSprinting)
	{
		if(GetWorld()->GetTimerManager().IsTimerActive(TimerDecreaseStamina))
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerDecreaseStamina);
		}
		IsSprinting = false;
		GetWorld()->GetTimerManager().SetTimer(TimerIncreaseStamina, this, &AHordeBaseCharacter::IncreaseStamina, .1f, true);
		UpdatePlayerMovementSpeed(300.f);
	}
}

bool AHordeBaseCharacter::ServerStopSprinting_Validate()
{
	return true;
}

/**
 *	Decreases Stamina. If Velocity is under 10.f and stamina 0.f then it stops sprinting.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::DecreaseStamina()
{
	if (Stamina > .0f && GetVelocity().SizeSquared() > 10.f)
	{
		Stamina = FMath::Clamp<float>((Stamina - StaminaDecreaseRate), 0.f, 100.f);
	}
	else {
		ServerStopSprinting();
	}
}


/**
 *	Increases Stamina with given Increase Rate.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::IncreaseStamina()
{
	if (Stamina < 100.f && !IsSprinting)
	{
		Stamina = FMath::Clamp<float>((Stamina + StaminaRefreshRate), 0.f, 100.f);
	}
}

/**
 *	Function that gets called if the Active Item has been changed. Stops Weapon Fire and Destroys Firearm. Spawns new Firearm by given Item ID and updates Animation Mode.
 *
 * @param The Item ID, Item Index inside Inventory and the amount of Loaded Ammo.
 * @return void
 */
void AHordeBaseCharacter::ActiveItemChanged(FString ItemID, int32 ItemIndex, int32 LoadedAmmo)
{
	StopWeaponFire();
	if (CurrentSelectedFirearm)
	{
		CurrentSelectedFirearm->Destroy();
	}

	FItem NewFirearmItem = UInventoryHelpers::FindItemByID(FName(*ItemID));
	if (NewFirearmItem.ItemID != "None" && NewFirearmItem.FirearmClass != nullptr)
	{
		FTransform NullTransform;
		CurrentSelectedFirearm = GetWorld()->SpawnActorDeferred<ABaseFirearm>(NewFirearmItem.FirearmClass, NullTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (CurrentSelectedFirearm)
		{
			CurrentSelectedFirearm->WeaponID = ItemID;
			CurrentSelectedFirearm->LoadedAmmo = LoadedAmmo;
			CurrentSelectedFirearm->FireMode = (uint8)NewFirearmItem.DefaultFireMode;
			CurrentSelectedFirearm->FinishSpawning(NullTransform);
			CurrentSelectedFirearm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NewFirearmItem.AttachmentPoint);
		}
	}
	AnimMode = NewFirearmItem.AnimType;
}

/**
 *	Returns HUD Object.
 *
 * @param
 * @return AHordeBaseHUD ( HUD Object )
 */
AHordeBaseHUD* AHordeBaseCharacter::GetHUD()
{
	AHordeBaseHUD* returnObj = nullptr;
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC) {
		returnObj = Cast<AHordeBaseHUD>(PC->GetHUD());
	}
	return returnObj;
}


/**
 *	Stops Weapon Fire Timer.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::StopWeaponFire()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(WeaponFireTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(WeaponFireTimer);
	}
}


/**
 *	Starts Weapon Fire depending on the FireMode of the Current Selected Weapon.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::TriggerWeaponFire()
{
	if (!(Reloading || IsBursting) && !IsDead)
	{
		if (CurrentSelectedFirearm)
		{
			CurrentWeaponInfo = UInventoryHelpers::FindItemByID(FName(*CurrentSelectedFirearm->WeaponID));
			if (CurrentWeaponInfo.FirearmClass != nullptr)
			{
				switch ((EFireMode)CurrentSelectedFirearm->FireMode) 
				{

				case EFireMode::EFireModeBurst:
					GetWorld()->GetTimerManager().SetTimer(BurstTimer, this, &AHordeBaseCharacter::BurstWeapon, CurrentWeaponInfo.FireRate, true);
					IsBursting = true;
					break;

				case EFireMode::EFireModeFull:
					CurrentSelectedFirearm->ServerFireFirearm();
					GetWorld()->GetTimerManager().SetTimer(WeaponFireTimer, this, &AHordeBaseCharacter::AutoFireWeapon, CurrentWeaponInfo.FireRate, true);
					break;

				case EFireMode::EFireModeSingle:
					CurrentSelectedFirearm->ServerFireFirearm();
				break;

				default:
					break;
				}
			}
		}
	}
}


/**
 *	Lets the Weapon burst depending on the Amount of Bursts defined in the Item Definition.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::BurstWeapon()
{
	if (NumberOfBursts >= CurrentWeaponInfo.BurstFireAmount || IsDead)
	{
		GetWorld()->GetTimerManager().ClearTimer(BurstTimer);
		NumberOfBursts = 0.f;
		IsBursting = false;
	}
	else {
		if (CurrentSelectedFirearm)
		{
			CurrentSelectedFirearm->ServerFireFirearm();
			NumberOfBursts = NumberOfBursts + 1.f;
		}
	}
}


/**
 *	Lets the Weapon Automatic Fire and stops if reloading or dead.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::AutoFireWeapon()
{
	if (CurrentSelectedFirearm)
	{
		CurrentSelectedFirearm->ServerFireFirearm();
		if (Reloading || IsDead)
		{
			StopWeaponFire();
		}
	}
}

/**
 *	Switches Firemode in Firearm and plays sound on all clients.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::ToggleFiremode()
{
	if(CurrentSelectedFirearm)
	{
		CurrentSelectedFirearm->ServerToggleFireMode();

		USoundCue* ToggleFireModeSound = ObjectFromPath<USoundCue>(FName(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_ToggleFiremode_Cue.A_ToggleFiremode_Cue'")));
		if (ToggleFireModeSound)
		{
			PlaySoundOnAllClients(ToggleFireModeSound, GetMesh()->GetComponentLocation());
		}
	}
}

/**
 *	Finishes Reload by clearing all timers and updates the Ammo.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::FinishReload()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ReloadTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}
	if (CurrentSelectedFirearm)
	{
		FItem TempItem = UInventoryHelpers::FindItemByID(FName(*CurrentSelectedFirearm->WeaponID));
		int32 AmmoIndex;
		int32 AmmoAmount = Inventory->CountAmmo(TempItem.AmmoType, AmmoIndex);
		if (AmmoAmount >= (TempItem.MaximumLoadedAmmo - CurrentSelectedFirearm->LoadedAmmo))
		{
			Inventory->RemoveAmmoByType(TempItem.AmmoType, (TempItem.MaximumLoadedAmmo - CurrentSelectedFirearm->LoadedAmmo));
			CurrentSelectedFirearm->LoadedAmmo = TempItem.MaximumLoadedAmmo;
		}
		else
		{
			CurrentSelectedFirearm->LoadedAmmo = AmmoAmount;
			Inventory->RemoveAmmoByType(TempItem.AmmoType, AmmoAmount);
		}
		Reloading = false;
		Inventory->UpdateCurrentItemAmmo(CurrentSelectedFirearm->LoadedAmmo);
	}
}

/** ( Server )
 *	Initiates Reload for the Weapon and plays Animation.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::ServerReload_Implementation()
{
	if (!Reloading)
	{
		if (CurrentSelectedFirearm)
		{
			FItem TempItem = UInventoryHelpers::FindItemByID(FName(*CurrentSelectedFirearm->WeaponID));
			int32 AmmoIndex;
			int32 AmmoAmount = Inventory->CountAmmo(TempItem.AmmoType, AmmoIndex);
			// Fixed: Compare against MaximumLoadedAmmo to allow reload when magazine isn't full
			if (AmmoAmount > 0 && (TempItem.MaximumLoadedAmmo != CurrentSelectedFirearm->LoadedAmmo))
			{
				Reloading = true;
				if (TempItem.PlayerAnimationData.CharacterReloadAnimation && GetMesh()->GetAnimInstance())
				{
					float AnimationDuration = TempItem.PlayerAnimationData.CharacterReloadAnimation->CalculateSequenceLength();
					PlayAnimationAllClients(TempItem.PlayerAnimationData.CharacterReloadAnimation);
					GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &AHordeBaseCharacter::FinishReload, AnimationDuration, false);
				}
				else {
					FinishReload();
				}
			}
		}
	}
}

bool AHordeBaseCharacter::ServerReload_Validate()
{
	return true;
}

/**
 *	Adds Health and updates Head Display Widget.
 *
 * @param Health To Add
 * @return void
 */
void AHordeBaseCharacter::AddHealth(float InHealth)
{
	if (HasAuthority())
	{
		Health = FMath::Clamp<float>(Health + InHealth, 0.f, 100.f);
		const APlayerState* PS = GetPlayerState();
		const FString PlayerName = PS ? PS->GetPlayerName() : TEXT("");
		UpdateHeadDisplayWidget(PlayerName, Health);
	}
}

/**
 *	Returns HeadDisplayWidget Object
 *
 * @param
 * @return UUserWidget ( Head Display Widget )
 */
UUserWidget* AHordeBaseCharacter::GetHeadDisplayWidget()
{
	return PlayerNameWidget->GetUserWidgetObject();
}

/**
 *	Returns current Firearm Object.
 *
 * @param
 * @return ABaseFirearm ( Firearm Object )
 */
ABaseFirearm* AHordeBaseCharacter::GetCurrentFirearm()
{
	return CurrentSelectedFirearm;
}

/**
 *	Sets Character Inputs and binds Functions to them.
 *
 * @param UInputComponent ( Player Input Component )
 * @return void
 */
void AHordeBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("DropItem", IE_Pressed, this, &AHordeBaseCharacter::DropCurrentItem);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AHordeBaseCharacter::ServerReload);

	PlayerInputComponent->BindAction("MouseUp", IE_Pressed, this, &AHordeBaseCharacter::ScrollUpItems);
	PlayerInputComponent->BindAction("MouseDown", IE_Pressed, this, &AHordeBaseCharacter::ScrollDownItems);

	PlayerInputComponent->BindAction("Primary", IE_Pressed, this, &AHordeBaseCharacter::SwitchToPrimary);
	PlayerInputComponent->BindAction("Secondary", IE_Pressed, this, &AHordeBaseCharacter::SwitchToSecondary);
	PlayerInputComponent->BindAction("Healing", IE_Pressed, this, &AHordeBaseCharacter::SwitchToHealing);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHordeBaseCharacter::TriggerWeaponFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AHordeBaseCharacter::StopWeaponFire);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AHordeBaseCharacter::ServerStartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AHordeBaseCharacter::ServerStopSprinting);

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &AHordeBaseCharacter::StartInteraction);
	PlayerInputComponent->BindAction("Use", IE_Released, this, &AHordeBaseCharacter::StopInteraction);

	PlayerInputComponent->BindAction("Toggle Firemode", IE_Pressed, this, &AHordeBaseCharacter::ToggleFiremode);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHordeBaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHordeBaseCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

}


/**
 *	Stops Weapon Fire and Drops Current Item. 
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::DropCurrentItem()
{
	if (CurrentSelectedFirearm && !Reloading)
	{
		StopWeaponFire();
		Inventory->ServerDropItem(CurrentSelectedFirearm);
	}
}

/**
 *	Switches Weapon to Primary.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::SwitchToPrimary()
{
	if (Inventory)
	{
		Inventory->SwitchWeapon(EActiveType::EActiveRifle);
	}
}

/**
 *	Switches Weapon to Secondary.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::SwitchToSecondary()
{
	if (Inventory)
	{
		Inventory->SwitchWeapon(EActiveType::EActivePistol);
	}
}

/**
 *	Switches to Healing Item.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::SwitchToHealing()
{
	if (Inventory)
	{
		Inventory->SwitchWeapon(EActiveType::EActiveMed);
	}
}

/**
 * Function for Mouse Wheel Up to scroll through Inventory.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::ScrollUpItems()
{
	if (Inventory)
	{
		Inventory->ScrollItems(true);
	}
}

/**
 *	Function for Mouse Wheel Down to scroll through Inventory.
 *
 * @param
 * @return void
 */
void AHordeBaseCharacter::ScrollDownItems()
{
	if (Inventory)
	{
		Inventory->ScrollItems(false);
	}
}

/**
 *	Moves Character Forward and Backwards
 *
 * @param float ( Forward / Backward Axis )
 * @return void
 */
void AHordeBaseCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

/**
 *	Moves Character Left and Right.
 *
 * @param float ( Left / Right Axis )
 * @return void
 */
void AHordeBaseCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}