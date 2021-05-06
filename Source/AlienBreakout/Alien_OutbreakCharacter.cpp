// Copyright Epic Games, Inc. All Rights Reserved.

#include "Alien_OutbreakCharacter.h"
#include "PAttackHitbox.h"



#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerHPWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>


AAlien_OutbreakCharacter::AAlien_OutbreakCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 75.f);
	CameraBoom->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 1500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.5f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f * 1.6;
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

	AttackCD = 253999999999.75f;
	Attacking = false;
	DashCD = 1.0f;
	Dashing = false;
	DashCDing = false;
	dashTime = 0.15;
	dashSpeed = 50.f;
	AvoidTime = 0.15;
	Avoiding = false;

	invincibleTime = 0.5;
	Invincible = false;
}

void AAlien_OutbreakCharacter::BeginPlay()
{
	Super::BeginPlay();

	//auto Widget = CreateWidget<UPlayerHPWidget>(UGameplayStatics::GetPlayerController(this, 0), WidgetClass);
	//Widget->Player = this;
	//Widget->AddToViewport();
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

	if (Dashing) {
		if (dashToRight)
			SetActorLocation(playerLoc + FVector(0, -1, 0) * dashSpeed);
		else
			SetActorLocation(playerLoc + FVector(0, 1, 0) * dashSpeed);
	}
	//HP -= 0.0001f;
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


	PlayerInputComponent->BindTouch(IE_Pressed, this, &AAlien_OutbreakCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AAlien_OutbreakCharacter::TouchStopped);
}

void AAlien_OutbreakCharacter::MoveRight(float Value)
{
	FSMUpdate(MOVE);
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
	if (DashCDing || knockingBack)
		return;
	Avoiding = true;

	Dashing = true;
	dashToRight = facingRight;
	dashCount = dashTime * fps;

	GetWorld()->GetTimerManager().SetTimer(AirDashCDTimerHandle, this, &AAlien_OutbreakCharacter::AirDashCD, DashCD, false);
	GetWorld()->GetTimerManager().SetTimer(AirDashingTimerHandle, this, &AAlien_OutbreakCharacter::AirDashStop, dashTime, false);

	//this->SetActorEnableCollision(false);
	GetWorld()->GetTimerManager().SetTimer(AvoidTimerHandle, this, &AAlien_OutbreakCharacter::AvoidStop, AvoidTime, false);
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
}

void AAlien_OutbreakCharacter::AirDashCD()
{
	DashCDing = false;
}

void AAlien_OutbreakCharacter::AirDashStop()
{
	Dashing = false;
	SetFSMState(DASH);

}

void AAlien_OutbreakCharacter::PAttack()
{
	Attacking = true;
	//Creates the sphere infront of the player. 
	//can use facing right to make it face the right way.
	//When it collides with the boss, it'll do damage.
	//I have to make a timer, that starts when created and deletes after it is gone.
	if (Attacking) 
		FSMUpdate(ATTACK);

	FVector loc = GetActorLocation();
	if (facingRight)
		loc.Y += -70.f;

	APAttackHitbox* a = GetWorld()->SpawnActor<APAttackHitbox>(loc, GetActorRotation());
	GetWorld()->GetTimerManager().SetTimer(PAttackTimerHandle, this, &AAlien_OutbreakCharacter::PAttackStop, AttackCD, false);
}

void AAlien_OutbreakCharacter::PAttackStop()
{

	Attacking = false;
	SetFSMState(ATTACK);

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
	UE_LOG(LogTemp, Warning, TEXT("fdsfd"));

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
	// Called once a frame when in the IDLE GameStates state
	// Implement functionality for Idle...
	UE_LOG(LogTemp, Warning, TEXT("SAW"));

}

void AAlien_OutbreakCharacter::Jump_Exit()
{
	// Implement any functionality for leaving the Idle state
}

void AAlien_OutbreakCharacter::Dash_Enter()
{
	// Change to GameEvents to Update when called
	Event = GameEvents::ON_UPDATE;
}

void AAlien_OutbreakCharacter::Dash_Update()
{
	// Called once a frame when in the RETREAT GameStates state
	// Implement functionality for Retreat...
	UE_LOG(LogTemp, Warning, TEXT("Stink"));

}

void AAlien_OutbreakCharacter::Dash_Exit()
{
	// Implement any functionality for leaving the Retreat state
	UE_LOG(LogTemp, Warning, TEXT("Stink"));
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
	// Called once a frame when in the RETREAT GameStates state
	// Implement functionality for Retreat...
	UE_LOG(LogTemp, Warning, TEXT("Teste"));
}

void AAlien_OutbreakCharacter::Attack_Exit()
{

	//Hide the gun
	// 
	// 
	// Implement any functionality for leaving the Retreat state
	UE_LOG(LogTemp, Warning, TEXT("Bye Bye"));
	FSMUpdate(IDLE);


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
}

void AAlien_OutbreakCharacter::Grab_Exit()
{
	// Implement any functionality for leaving the Retreat state
}