// Fill out your copyright notice in the Description page of Project Settings.


#include "Alien_BreakOutBossOne.h"

#include "Components/StaticMeshComponent.h"

#include "BossHPWidget.h"

#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>




// Sets default values
AAlien_BreakOutBossOne::AAlien_BreakOutBossOne()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	timeTick = 0;
	fps = 60;

	summonCoolDown = 5;
	attackCoolDown = 2;
	teleportCoolDown = 15;

	rockLeft = 0;
	maxNumRock = 12;

	teleportLocation.Add(FVector(-60.0, -1170.0, 2668.0));
	teleportLocation.Add(FVector(-60.0, -1710.0, 1938.0));
	teleportLocation.Add(FVector(-60.0, -1570.0, 3708.0));

}

void AAlien_BreakOutBossOne::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Called when the game starts or when spawned
void AAlien_BreakOutBossOne::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &AAlien_BreakOutBossOne::Teleport, teleportCoolDown, false);
	Summon();
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::Attack, attackCoolDown, false);
}

void AAlien_BreakOutBossOne::Teleport() {
	int rand = FMath::RandRange(0, 2);
	FVector teleportLoc = teleportLocation[rand];
	while (this->GetActorLocation().Equals(teleportLoc)) {
		rand = FMath::RandRange(0, 2);
		teleportLoc = teleportLocation[rand];
	}
	SetActorLocation(teleportLocation[rand]);
	GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &AAlien_BreakOutBossOne::Teleport, teleportCoolDown, false);
}

void AAlien_BreakOutBossOne::Summon() {
	if (rockLeft <= maxNumRock - 3) {
		FVector loc = GetActorLocation();

		// Prevent overlap at spawn
		loc.X += 200;
		rocks.Add(GetWorld()->SpawnActor<ARockProjectileActor>(loc, GetActorRotation()));
		loc.X += 200;
		rocks.Add(GetWorld()->SpawnActor<ARockProjectileActor>(loc, GetActorRotation()));
		loc.X += 200;
		rocks.Add(GetWorld()->SpawnActor<ARockProjectileActor>(loc, GetActorRotation()));

		rocks[rockLeft]->axis = FVector(0, 0.5, 0.5);
		rocks[rockLeft]->angleAxis = 180.f;
		rocks[rockLeft]->rotateSpeed = 80.f;
		rocks[rockLeft]->dimention = FVector(330, 0, 0);

		rocks[rockLeft + 1]->axis = FVector(0, -0.5, 0.5);
		rocks[rockLeft + 1]->angleAxis = 90.f;
		rocks[rockLeft + 1]->rotateSpeed = 80.f;
		rocks[rockLeft + 1]->dimention = FVector(240, 0, 0);

		rocks[rockLeft + 2]->axis = FVector(0, 0, 1);
		rocks[rockLeft + 2]->angleAxis = -90.f;
		rocks[rockLeft + 2]->rotateSpeed = 80.f;
		rocks[rockLeft + 2]->dimention = FVector(180, 0, 0);

		rockLeft += 3;
	}
	GetWorld()->GetTimerManager().SetTimer(SummonTimerHandle, this, &AAlien_BreakOutBossOne::Summon, summonCoolDown, false);
}

void AAlien_BreakOutBossOne::Attack() {
	if (rockLeft != 0) {
		int rand = FMath::RandRange(0, rockLeft - 1);
		rocks[rand]->readyFire();
		rocks.RemoveAt(rand);
		rockLeft--;
	}
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::Attack, attackCoolDown, false);
}

// Called every frame
void AAlien_BreakOutBossOne::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//SetFSMState(GameStates::IDLE);

	//timeTick++;

	//FSMUpdate();
}

void AAlien_BreakOutBossOne::FSMUpdate()
{
	// List all expected states and call relevant state functions
	if (State == GameStates::IDLE)
	{
		if (Event == GameEvents::ON_ENTER) {
			Idle_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Idle_Update();
		}
	}

	if (State == GameStates::SUMMON)
	{
		if (Event == GameEvents::ON_ENTER) {
			Summon_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Summon_Update();
		}
	}
	if (State == GameStates::ATTACK)
	{
		if (Event == GameEvents::ON_ENTER) {
			Attack_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Attack_Update();
		}
	}
	if (State == GameStates::TELEPORT)
	{
		if (Event == GameEvents::ON_ENTER) {
			Teleport_Enter();
		}
		if (Event == GameEvents::ON_UPDATE) {
			Teleport_Update();
		}
	}

	// Append any GameStates you add to this example..
}

void AAlien_BreakOutBossOne::SetFSMState(GameStates newState)
{
	// Append any GameStates you add to this example to this switch statement...
	switch (State)
	{
	case GameStates::IDLE:
		Idle_Exit();
		break;
	case GameStates::SUMMON:
		Summon_Exit();
		break;
	case GameStates::ATTACK:
		Attack_Exit();
		break;
	case GameStates::TELEPORT:
		Teleport_Exit();
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("Unexpected state has not been implemented!"), newState);
		return;
	}

	// Set new GameStates state and begin OnEnter of that state
	State = newState;
	Event = GameEvents::ON_ENTER;
}

void AAlien_BreakOutBossOne::Idle_Enter()
{
	Event = GameEvents::ON_UPDATE;
}

void AAlien_BreakOutBossOne::Idle_Update()
{
}

void AAlien_BreakOutBossOne::Idle_Exit()
{
}

void AAlien_BreakOutBossOne::Summon_Enter()
{
	Event = GameEvents::ON_UPDATE;

	rockLeft = 3;

	FVector loc = GetActorLocation();

	// Prevent overlap at spawn
	loc.X += 200;
	rocks[0] = GetWorld()->SpawnActor<ARockProjectileActor>(loc, GetActorRotation());
	loc.X += 200;
	rocks[1] = GetWorld()->SpawnActor<ARockProjectileActor>(loc, GetActorRotation());
	loc.X += 200;
	rocks[2] = GetWorld()->SpawnActor<ARockProjectileActor>(loc, GetActorRotation());

	rocks[0]->axis = FVector(0, 0.5, 0.5);
	rocks[0]->angleAxis = 180.f;
	rocks[0]->rotateSpeed = 80.f;
	rocks[0]->dimention = FVector(330, 0, 0);

	rocks[1]->axis = FVector(0, -0.5, 0.5);
	rocks[1]->angleAxis = 90.f;
	rocks[1]->rotateSpeed = 80.f;
	rocks[1]->dimention = FVector(240, 0, 0);

	rocks[2]->axis = FVector(0, 0, 1);
	rocks[2]->angleAxis = -90.f;
	rocks[2]->rotateSpeed = 80.f;
	rocks[2]->dimention = FVector(180, 0, 0);
}

void AAlien_BreakOutBossOne::Summon_Update()
{

}

void AAlien_BreakOutBossOne::Summon_Exit()
{
}

void AAlien_BreakOutBossOne::Attack_Enter()
{
	Event = GameEvents::ON_UPDATE;

	rocks[rockLeft - 1]->readyFire();
	//rocks[rockLeft - 1] = NULL;
}

void AAlien_BreakOutBossOne::Attack_Update()
{
}

void AAlien_BreakOutBossOne::Attack_Exit()
{
	rockLeft--;
}

void AAlien_BreakOutBossOne::Teleport_Enter()
{
	Event = GameEvents::ON_UPDATE;
	int rand = FMath::RandRange(0, 2);
	SetActorLocation(teleportLocation[rand]);
}

void AAlien_BreakOutBossOne::Teleport_Update()
{
}

void AAlien_BreakOutBossOne::Teleport_Exit()
{
}

void AAlien_BreakOutBossOne::hitByPlayer(float minsHP) {
	this->HP -= minsHP;
	if (HP <= 0.f) {
		UGameplayStatics::OpenLevel(GetWorld(), "End");
		// Death
	}
}