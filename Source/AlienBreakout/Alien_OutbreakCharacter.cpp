// Copyright Epic Games, Inc. All Rights Reserved.

#include "Alien_OutbreakCharacter.h"
#include "Alien_BreakOutBossOne.h"
#include "PAttackHitbox.h"
#include "ThrownItem.h"

#include "Components/PrimitiveComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerHPWidget.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include "GameFramework/CharacterMovementComponent.h"


AAlien_OutbreakCharacter::AAlien_OutbreakCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 1500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.5f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f * 1.65;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f * 1.4;
	GetCharacterMovement()->MaxFlySpeed = 600.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	// Setting up Knock Back Variables
	knockToLeft = true;
	knockingBack = false;
	knockBackTime = 0.05;
	knockBackSpeed = 100.f;
	fps = 60;

	attackTimerConst = 20;
	attackTimer = 0;

	grabTimerConst = 75;
	grabTimer = 0;

	throwTimerConst = 75;
	throwTimer = 0;

	// Loading Hurt Sounds
	static ConstructorHelpers::FObjectFinder<USoundWave> Hurt1(TEXT("SoundWave'/Game/Sounds/Hurt1'"));
	HurtSound1 = Hurt1.Object;
	static ConstructorHelpers::FObjectFinder<USoundWave> Hurt2(TEXT("SoundWave'/Game/Sounds/Hurt2'"));
	HurtSound2 = Hurt2.Object;
	static ConstructorHelpers::FObjectFinder<USoundWave> Hurt3(TEXT("SoundWave'/Game/Sounds/Hurt3'"));
	HurtSound3 = Hurt3.Object;
	static ConstructorHelpers::FObjectFinder<USoundWave> Hurt4(TEXT("SoundWave'/Game/Sounds/Hurt4'"));
	HurtSound4 = Hurt4.Object;
	static ConstructorHelpers::FObjectFinder<USoundWave> Hurt5(TEXT("SoundWave'/Game/Sounds/Hurt5'"));
	HurtSound5 = Hurt5.Object;

	AttackCD = 1.0f;
	isAttacking = false;
	DashCD = 1.0f;
	Dashing = false;
	DashCDing = false;
	dashSpeed = 30.f;
	AvoidTime = 0.5;
	Avoiding = false;

	invincibleTime = 0.5;
	Invincible = false;

	isHolding = false;

	isThrowing = false;

	isGrabbing = false;
}

void AAlien_OutbreakCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AAlien_OutbreakCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	
	
	// Keep player in right Axis
	FVector playerLoc = GetActorLocation();
	if (playerLoc.X != -60.0) {
		playerLoc.X = -60.0;
		SetActorLocation(playerLoc);
	}

	if (knockingBack) {
		if (knockToLeft)
			SetActorLocation(playerLoc + FVector(0, 1, 0) * knockBackSpeed);
		else
			SetActorLocation(playerLoc + FVector(0, -1, 0) * knockBackSpeed);
		knockBackCount--;
		if (knockBackCount <= 0)
			knockingBack = false;
	}

	if(attackTimer >= 0) 
		attackTimer--;

	if (attackTimer <= 0 && isAttacking)
		FSMUpdate(IDLE);


	if (grabTimer >= 0 && isGrabbing)
		grabTimer--;

	if (grabTimer <= 0)
		isGrabbing = false;

	if (throwTimer >= 0 && isThrowing)
		throwTimer--;

	if (throwTimer <= 0)
		FSMUpdate(IDLE);
}
//////////////////////////////////////////////////////////////////////////
// Input

void AAlien_OutbreakCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &AAlien_OutbreakCharacter::MoveRight);
	PlayerInputComponent->BindAction("AirDash", IE_Pressed, this, &AAlien_OutbreakCharacter::AirDash);
	PlayerInputComponent->BindAction("PAttack", IE_Pressed, this, &AAlien_OutbreakCharacter::PAttack);
	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &AAlien_OutbreakCharacter::playerCheckGrab);
	FInputActionBinding& pause = PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AAlien_OutbreakCharacter::Pause);
	pause.bExecuteWhenPaused = true;


	PlayerInputComponent->BindTouch(IE_Pressed, this, &AAlien_OutbreakCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AAlien_OutbreakCharacter::TouchStopped);
}

void AAlien_OutbreakCharacter::MoveRight(float Value)
{
	// add movement in that direction
	AddMovementInput(FVector(0.f, -1.f, 0.f), Value);
	if (Value < 0)
		facingRight = false;
	else if (Value > 0)
		facingRight = true;
}

void AAlien_OutbreakCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// jump on any touch
	Jump();
}

void AAlien_OutbreakCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	StopJumping();
}

void AAlien_OutbreakCharacter::PlayerHP_Setter(float new_HP) {
	HP = new_HP;
}

void AAlien_OutbreakCharacter::beingHit(float minsHP, float rockY) {
	// Reduce player hp on hit
	this->HP -= minsHP;
	FSMUpdate(HURT);
	if (HP <= 0.f) {
		FSMUpdate(DEATH);
		// Player Death
	}

	// Player hurt sound
	playHurtSound(FMath::RandRange(1, 5));

	// Set up knockback
	float Y = this->GetActorLocation().Y;
	if (Y > rockY)
		knockToLeft = true;
	else
		knockToLeft = false;
	knockingBack = true;
	knockBackCount = knockBackTime * fps;

	Invincible = true;
	//this->SetActorEnableCollision(false);
	GetWorld()->GetTimerManager().SetTimer(InvincibleTimerHandle, this, &AAlien_OutbreakCharacter::InvincibleStop, invincibleTime, false);
}

void AAlien_OutbreakCharacter::playHurtSound(int num) {
	// Random Play hurt sound
	switch (num) {
	case 1: UGameplayStatics::PlaySound2D(this, HurtSound1); break;
	case 2: UGameplayStatics::PlaySound2D(this, HurtSound2); break;
	case 3: UGameplayStatics::PlaySound2D(this, HurtSound3); break;
	case 4: UGameplayStatics::PlaySound2D(this, HurtSound4); break;
	case 5: UGameplayStatics::PlaySound2D(this, HurtSound5); break;
	default: UGameplayStatics::PlaySound2D(this, HurtSound1); break;
	}
}

void AAlien_OutbreakCharacter::AirDash()
{
	FSMUpdate(DASH);
}

void AAlien_OutbreakCharacter::InvincibleStop()
{
	//this->SetActorEnableCollision(true);
	Invincible = false;
}

void AAlien_OutbreakCharacter::AvoidStop()
{
	//this->SetActorEnableCollision(true);
	Avoiding = false;
	SetFSMState(DASH);
}

void AAlien_OutbreakCharacter::AirDashCD()
{
	DashCDing = false;
	SetFSMState(DASH);
}

void AAlien_OutbreakCharacter::AirDashStop()
{
	SetFSMState(DASH);
}

void AAlien_OutbreakCharacter::PAttack()
{
	if(!isHolding)
		FSMUpdate(ATTACK);
	else
		FSMUpdate(THROW);
}

void AAlien_OutbreakCharacter::playerCheckGrab()
{
	//handles grabbing said item, and setting the character to the grab state
	if(!isGrabbing && !isHolding)
		FSMUpdate(GRAB);


}

void AAlien_OutbreakCharacter::Pause() 
{
	FSMUpdate(PAUSE);
}



void AAlien_OutbreakCharacter::damagePlayer(float damageTaken, float knockback) 
{
	((AAlien_OutbreakCharacter*)GetWorld()->GetFirstPlayerController()->GetPawn())->beingHit(damageTaken, knockback);
}

void AAlien_OutbreakCharacter::processHit(float damageTaken, float knockback)
{
	if (isGrabbing && !isHolding) {
		isHolding = true;
		UE_LOG(LogTemp, Warning, TEXT("Hands Got Caught"));
	}
	else {
		damagePlayer(damageTaken, knockback);
	}
}

/// 
/// State Machine
/// 
void AAlien_OutbreakCharacter::FSMUpdate(GameStates nState)
{
	// List all expected states and call relevant state functions
	if (nState == GameStates::IDLE)
	{
		if (Event == GameEvents::ON_ENTER) {
			Idle_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Idle_Update();
		}
	}

	if (nState == GameStates::MOVE)
	{
		if (Event == GameEvents::ON_ENTER) {
			Move_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Move_Update();
		}
	}

	if (nState == GameStates::JUMP)
	{
		if (Event == GameEvents::ON_ENTER) {
			Jump_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Jump_Update();
		}
	}

	if (nState == GameStates::DASH)
	{
		if (Event == GameEvents::ON_ENTER) {
			Dash_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Dash_Update();
		}
	}

	if (nState == GameStates::ATTACK)
	{
		if (Event == GameEvents::ON_ENTER) {
			Attack_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Attack_Update();
		}
	}

	if (nState == GameStates::HURT)
	{
		if (Event == GameEvents::ON_ENTER) {
			Hurt_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Hurt_Update();
		}
	}

	if (nState == GameStates::DEATH)
	{
		if (Event == GameEvents::ON_ENTER) {
			Death_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Death_Update();
		}
	}

	if (nState == GameStates::GRAB)
	{
		if (Event == GameEvents::ON_ENTER) {
			Grab_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Grab_Update();
		}
	}
	if (nState == GameStates::THROW)
	{
		if (Event == GameEvents::ON_ENTER) {
			Throw_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Throw_Update();
		}
	}

	if (nState == GameStates::PAUSE)
	{
		if (Event == GameEvents::ON_ENTER) {
			Pause_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Pause_Update();
		}
	}

	// Append any GameStates you add to this example..
}

void AAlien_OutbreakCharacter::SetFSMState(GameStates newState)
{
	// Append any GameStates you add to this example to this switch statement...
	switch (State)
	{
	case GameStates::IDLE:
		Idle_Exit();
		break;
	case GameStates::MOVE:
		Move_Exit();
		break;
	case GameStates::JUMP:
		Jump_Exit();
		break;
	case GameStates::DASH:
		Dash_Exit();
		break;
	case GameStates::ATTACK:
		Attack_Exit();
		break;
	case GameStates::HURT:
		Hurt_Exit();
		break;
	case GameStates::DEATH:
		Death_Exit();
		break;
	case GameStates::GRAB:
		Grab_Exit();
		break;
	case GameStates::THROW:
		Throw_Exit();
		break;
	case GameStates::PAUSE:
		Pause_Exit();
		break;
	default:
		return;
	}

	// Set new GameStates state and begin OnEnter of that state
	State = newState;
	Event = GameEvents::ON_ENTER;
}

void AAlien_OutbreakCharacter::Idle_Enter()
{

	// Change to GameEvents to Update when called
	Event = GameEvents::ON_UPDATE;
}

void AAlien_OutbreakCharacter::Idle_Update()
{

	isAttacking = false;
	isThrowing = false;
	
	// Called once a frame when in the IDLE GameStates state
	// Implement functionality for Idle...
}

void AAlien_OutbreakCharacter::Idle_Exit()
{
	// Implement any functionality for leaving the Idle state
}

void AAlien_OutbreakCharacter::Move_Enter()
{
	// Change to GameEvents to Update when called
	Event = GameEvents::ON_UPDATE;
}

void AAlien_OutbreakCharacter::Move_Update()
{
	
}

void AAlien_OutbreakCharacter::Move_Exit()
{

}

void AAlien_OutbreakCharacter::Jump_Enter()
{

	Event = GameEvents::ON_UPDATE;
}

void AAlien_OutbreakCharacter::Jump_Update()
{
	

}

void AAlien_OutbreakCharacter::Jump_Exit()
{
}

void AAlien_OutbreakCharacter::Dash_Enter()
{
	Event = GameEvents::ON_UPDATE;
}

void AAlien_OutbreakCharacter::Dash_Update()
{

	Avoiding = true;


	FVector playerLoc = GetActorLocation();
	if(facingRight)
		LaunchCharacter(FVector(0, -1000, 0), false, true);
	else
		LaunchCharacter(FVector(0, 1000, 0), false, true);


	GetWorld()->GetTimerManager().SetTimer(AirDashCDTimerHandle, this, &AAlien_OutbreakCharacter::AirDashCD, DashCD, false);

	//this->SetActorEnableCollision(false);
	GetWorld()->GetTimerManager().SetTimer(AvoidTimerHandle, this, &AAlien_OutbreakCharacter::AvoidStop, AvoidTime, false);
	SetFSMState(DASH);
	


}

void AAlien_OutbreakCharacter::Dash_Exit()
{
	// Implement any functionality for leaving the Retreat state
	Avoiding = false;
	FSMUpdate(IDLE);


}

void AAlien_OutbreakCharacter::Attack_Enter()
{
	// Change to GameEvents to Update when called
	Event = GameEvents::ON_UPDATE;
	//Turn the Gun the right opacity
}

void AAlien_OutbreakCharacter::Attack_Update()
{
	attackTimer = attackTimerConst;
	// Called once a frame when in the RETREAT GameStates state
	// Implement functionality for Retreat...
	FVector loc = GetActorLocation();
	if (facingRight)
		loc.Y += -70.f;
	else
		loc.Y += 70.f;

	isAttacking = true;
	//Creates the sphere infront of the player. 
	//can use facing right to make it face the right way.
	//When it collides with the boss, it'll do damage.
	//I have to make a timer, that starts when created and deletes after it is gone.


	APAttackHitbox* a = GetWorld()->SpawnActor<APAttackHitbox>(loc, GetActorRotation());
}

void AAlien_OutbreakCharacter::Attack_Exit()
{

	//Hide the gun
	// 
	// 
	// Implement any functionality for leaving the Retreat state

}

void AAlien_OutbreakCharacter::Hurt_Enter()
{
	// Change to GameEvents to Update when called
	Event = GameEvents::ON_UPDATE;
}

void AAlien_OutbreakCharacter::Hurt_Update()
{
	// Called once a frame when in the RETREAT GameStates state
	// Implement functionality for Retreat...
}

void AAlien_OutbreakCharacter::Hurt_Exit()
{
	// Implement any functionality for leaving the Retreat state
}

void AAlien_OutbreakCharacter::Death_Enter()
{

	// Change to GameEvents to Update when called
	Event = GameEvents::ON_UPDATE;
}

void AAlien_OutbreakCharacter::Death_Update()
{
	// Called once a frame when in the RETREAT GameStates state
	// Implement functionality for Retreat...
	UGameplayStatics::OpenLevel(GetWorld(), "End");
}

void AAlien_OutbreakCharacter::Death_Exit()
{
	// Implement any functionality for leaving the Retreat state
}

void AAlien_OutbreakCharacter::Grab_Enter()
{
	// Change to GameEvents to Update when called
	Event = GameEvents::ON_UPDATE;
}

void AAlien_OutbreakCharacter::Grab_Update()
{
	// Called once a frame when in the RETREAT GameStates state
	// Implement functionality for Retreat...
	UE_LOG(LogTemp, Warning, TEXT("Catch these hands"));
	isAttacking = false;
	grabTimer = grabTimerConst;
	isGrabbing = true;
}

void AAlien_OutbreakCharacter::Grab_Exit()
{
	// Implement any functionality for leaving the Retreat state'

}

void AAlien_OutbreakCharacter::Throw_Enter()
{

}

void AAlien_OutbreakCharacter::Throw_Update()
{
	throwTimer = throwTimerConst;
	isHolding = false;
	isThrowing = true;
	FVector loc = GetActorLocation();
	if (facingRight)
		loc.Y += -70.f;
	else
		loc.Y += 70.f;
	//Creates the sphere infront of the player. 
	//can use facing right to make it face the right way.
	//When it collides with the boss, it'll do damage.
	//I have to make a timer, that starts when created and deletes after it is gone.

	UE_LOG(LogTemp, Warning, TEXT("Thrown these hands"));

	AThrownItem* i = GetWorld()->SpawnActor<AThrownItem>(loc, GetActorRotation());

}

void AAlien_OutbreakCharacter::Throw_Exit()
{

	FSMUpdate(IDLE);

}

void AAlien_OutbreakCharacter::Pause_Enter()
{
	// Change to GameEvents to Update when called

}

void AAlien_OutbreakCharacter::Pause_Update()
{
	// Called once a frame when in the RETREAT GameStates state
	// Implement functionality for Retreat...
	if (UGameplayStatics::IsGamePaused(GetWorld()) == false) {
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	else {
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		SetFSMState(IDLE);
	}

	
}

void AAlien_OutbreakCharacter::Pause_Exit()
{
	// Implement any functionality for leaving the Retreat state
}