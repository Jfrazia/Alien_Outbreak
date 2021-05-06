// Fill out your copyright notice in the Description page of Project Settings.


#include "Alien_BreakOutBossOne.h"

#include "Components/StaticMeshComponent.h"

#include "BossHPWidget.h"

#include "Alien_OutbreakCharacter.h"

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
	teleportCoolDown = 5;

	rockLeft = 0;
	maxNumRock = 12;

	teleportLocation.Add(FVector(-60.0, -1630.0, 1938.0));
	teleportLocation.Add(FVector(-60.0, -1110.0, 2548.0));
	teleportLocation.Add(FVector(-60.0, -1630.0, 3038.0));
	teleportLocation.Add(FVector(-60.0, -1360.0, 3718.0));

	teleportLocation.Add(FVector(-60.0, 1790.0, 1938.0));
	teleportLocation.Add(FVector(-60.0, 1950.0, 3068.0));
	teleportLocation.Add(FVector(-60.0, 1920.0, 3818.0));
	//teleportLocation.Add(FVector(-60.0, 0.0, 2700.0));

	facingLeft = true;

	rushAttackCoolDown = 9;
	rushAttackWaitTime = 1;
	rushAttackDuration = 2.5;
	rushAttackSpeed = 30;
	rushAttackCD = false;
	rushAttacking = false;

	specialRushing = false;;
	specialRushStage = 0;
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
	if (specialRushing || rushAttacking) {
		GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &AAlien_BreakOutBossOne::Teleport, teleportCoolDown, false);
		return;
	}

	int index = 0;
	int i = 0;
	int previousDiff = 10000;
	FVector playerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	for (FVector teleLoc : teleportLocation) {
		int newDiff = sqrt(pow(playerLoc.Y - teleLoc.Y, 2) + pow(playerLoc.Z - teleLoc.Z, 2) * 2);
		//int newDiff = abs(playerLoc.Z - teleLoc.Z);
		if (newDiff < previousDiff) {
			index = i;
			previousDiff = newDiff;
		}
		i++;
	}

	if(!GetActorLocation().Equals(teleportLocation[index]))
		TeleportTo(index);

	GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &AAlien_BreakOutBossOne::Teleport, teleportCoolDown, false);

	// 25% Special Rush, 75% Normal Rush
	if (!rushAttackCD) {
		int rand = FMath::RandRange(1, 100);
		//rand = 100;
		if (rand > 75) 
			GetWorld()->GetTimerManager().SetTimer(RushAttackWaitTimerHandle, this, &AAlien_BreakOutBossOne::SpecialRush, rushAttackWaitTime, false);
		else
			GetWorld()->GetTimerManager().SetTimer(RushAttackWaitTimerHandle, this, &AAlien_BreakOutBossOne::RushAttack, rushAttackWaitTime, false);
	}
}

void AAlien_BreakOutBossOne::TeleportTo(int index) {
	SetActorLocation(teleportLocation[index]);
	if (teleportLocation[index].Y > 0) {
		if (facingLeft) {
			SetActorRotation(FRotator(0.0, 180.0, 0.0), ETeleportType::None);
		}
		facingLeft = false;
	}
	else {
		if (!facingLeft)
			SetActorRotation(FRotator(0.0, 0.0, 0.0), ETeleportType::None);
		facingLeft = true;
	}
}

void AAlien_BreakOutBossOne::RushAttack() {
	rushAttackCD = true;
	rushAttacking = true;
	timeTick = 0;

	GetWorld()->GetTimerManager().SetTimer(RushAttackTimerHandle, this, &AAlien_BreakOutBossOne::RushAttackDone, rushAttackCoolDown, false);
}

void AAlien_BreakOutBossOne::SpecialRush() {
	rushAttackCD = true;
	specialRushing = true;
	timeTick = 0;

	switch (specialRushStage) {
	case 0: TeleportTo(4); specialRushStage++; break;
	case 1: TeleportTo(1); specialRushStage++; break;
	case 2: TeleportTo(5); specialRushStage++; break;
	case 3: GetWorld()->GetTimerManager().SetTimer(RushAttackTimerHandle, this, &AAlien_BreakOutBossOne::RushAttackDone, rushAttackCoolDown, false);
		specialRushing = false; specialRushStage = 0; Teleport(); break;
	}
}

void AAlien_BreakOutBossOne::RushAttackDone() {
	rushAttackCD = false;
}

void AAlien_BreakOutBossOne::Summon() {
	if (specialRushing || rushAttacking) {
		GetWorld()->GetTimerManager().SetTimer(SummonTimerHandle, this, &AAlien_BreakOutBossOne::Summon, summonCoolDown, false);
		return;
	}
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
	if (specialRushing || rushAttacking) {
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::Attack, attackCoolDown, false);
		return;
	}
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

	// Rushing
	if (rushAttacking || specialRushing) {
		if (facingLeft)
			SetActorLocation(GetActorLocation() + FVector(0.0, rushAttackSpeed, 0.0));
		else
			SetActorLocation(GetActorLocation() + FVector(0.0, -rushAttackSpeed, 0.0));
		timeTick++;
		if (timeTick > fps * rushAttackDuration) {
			if (rushAttacking) {
				rushAttacking = false;
				Teleport();
			}
			else if (specialRushing) {
				SpecialRush();
			}
		}
	}
}

void AAlien_BreakOutBossOne::hitByPlayer(float minsHP) {
	this->HP -= minsHP;
	if (HP <= 0.f) {
		UGameplayStatics::OpenLevel(GetWorld(), "End");
		// Death
	}
}

void AAlien_BreakOutBossOne::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) {
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	//UE_LOG(LogTemp, Warning, TEXT("HIT!"));
	if (Other->IsA(AAlien_OutbreakCharacter::StaticClass())) {
		AAlien_OutbreakCharacter* player = (AAlien_OutbreakCharacter*)GetWorld()->GetFirstPlayerController()->GetPawn();
		if (!player->Avoiding && !player->Invincible) {
			player->beingHit(0.08f, GetActorLocation().Y);
		}
	}
}