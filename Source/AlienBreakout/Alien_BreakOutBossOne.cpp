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

	attackCoolDown = 3;
	teleportCoolDown = 8;

	rockLeft = 0;
	maxNumRock = 12;

	teleportLocation.Add(FVector(-60.0, -1210.0, 1938.0));
	teleportLocation.Add(FVector(-60.0, -930.0, 2658.0));
	teleportLocation.Add(FVector(-60.0, -1520.0, 2978.0));
	teleportLocation.Add(FVector(-60.0, -1030.0, 3788.0));

	teleportLocation.Add(FVector(-60.0, 1660.0, 1938.0));
	teleportLocation.Add(FVector(-60.0, 1730.0, 3238.0));
	teleportLocation.Add(FVector(-60.0, 1560.0, 3788.0));

	facingLeft = true;

	rushAttackCoolDown = 10;
	rushAttackWaitTime = 1;
	rushAttackDuration = 2.0;
	rushAttackSpeed = 30;
	rushAttackCD = false;
	rushAttacking = false;

	specialRushing = false;;

	SequenceThrowTimes = 5;

	SpreadThrowTimes = 16;

	specialRockAttacking = false;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> Particle(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	ParticleTeleport = Particle.Object;


	// Loading  Sounds
	static ConstructorHelpers::FObjectFinder<USoundWave> TeleportSound1(TEXT("SoundWave'/Game/Objects/Sounds/Telekinesis1.Telekinesis1'"));
	TeleportSound1 = TeleportSound1.Object;
	static ConstructorHelpers::FObjectFinder<USoundWave> TeleportSound2(TEXT("SoundWave'/Game/Objects/Sounds/Telekinesis2.Telekinesis2'"));
	TeleportSound2 = TeleportSound2.Object;
	static ConstructorHelpers::FObjectFinder<USoundWave> TeleportSound3(TEXT("SoundWave'/Game/Objects/Sounds/Telekinesis3.Telekinesis3'"));
	TeleportSound3 = TeleportSound3.Object;
	static ConstructorHelpers::FObjectFinder<USoundWave> TeleportSound4(TEXT("SoundWave'/Game/Objects/Sounds/Telekinesis4.Telekinesis4'"));
	TeleportSound4 = TeleportSound4.Object;
	static ConstructorHelpers::FObjectFinder<USoundWave> TeleportSound5(TEXT("SoundWave'/Game/Objects/Sounds/Telekinesis5.Telekinesis5'"));
	TeleportSound5 = TeleportSound5.Object;
}

void AAlien_BreakOutBossOne::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAlien_BreakOutBossOne::playTeleportSound(int num)
{
	switch (num) 
	{
	case 1:  UGameplayStatics::PlaySound2D(this, TeleportSound1); break;
	case 2:  UGameplayStatics::PlaySound2D(this, TeleportSound2); break;
	case 3:  UGameplayStatics::PlaySound2D(this, TeleportSound3); break;
	case 4:  UGameplayStatics::PlaySound2D(this, TeleportSound4); break;
	case 5:  UGameplayStatics::PlaySound2D(this, TeleportSound5); break;
	default:  UGameplayStatics::PlaySound2D(this, TeleportSound1); break;
	}

}

// Called when the game starts or when spawned
void AAlien_BreakOutBossOne::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &AAlien_BreakOutBossOne::Teleport, teleportCoolDown, false);
	Summon();
	Summon();
	Summon();
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::RockAttack, attackCoolDown, false);
}

void AAlien_BreakOutBossOne::Teleport() {
	if (specialRushing || rushAttacking || specialRockAttacking) {
		GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &AAlien_BreakOutBossOne::Teleport, 2.0, false);
		return;
	}
	if (FMath::RandRange(1,100) < 80) {

		TeleportTo(GetCloseLocation());

		GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &AAlien_BreakOutBossOne::Teleport, teleportCoolDown, false);

		GetWorld()->GetTimerManager().SetTimer(RushAttackWaitTimerHandle, this, &AAlien_BreakOutBossOne::RushAttack, rushAttackWaitTime, false);
	}
	else {
		int index = FMath::RandRange(0, 6);
		TeleportTo(teleportLocation[index]);
		GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &AAlien_BreakOutBossOne::Teleport, teleportCoolDown, false);
	}
}

void AAlien_BreakOutBossOne::TeleportTo(FVector Location) {
	if (GetActorLocation().Equals(Location))
		return;

	if (ParticleTeleport)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleTeleport, GetActorLocation());

	SetActorLocation(Location);

	if (Location.Y > 0) {
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

FVector AAlien_BreakOutBossOne::GetCloseLocation() {
	int index = 0;
	int i = 0;
	int previousDiff = INT_MAX;
	FVector playerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	for (FVector teleLoc : teleportLocation) {
		int newDiff = sqrt(pow(playerLoc.Y - teleLoc.Y, 2) + pow(playerLoc.Z - teleLoc.Z, 2) * 10);
		if (newDiff < previousDiff && sqrt(pow(playerLoc.Y - teleLoc.Y, 2) + pow(playerLoc.Z - teleLoc.Z, 2)) > 500) {
			index = i;
			previousDiff = newDiff;
		}
		i++;
	}
	return teleportLocation[index];
}

void AAlien_BreakOutBossOne::RushAttack() {
	// 25% Special Rush, 75% Normal Rush
	if (!rushAttackCD) {
		if (FMath::RandRange(1, 100) > 75)
			SpecialRush();
		else
			NormalRush();
	}
}

void AAlien_BreakOutBossOne::NormalRush() {
	rushAttackCD = true;
	rushAttacking = true;
	GetWorld()->GetTimerManager().SetTimer(RushAttackCDTimerHandle, this, &AAlien_BreakOutBossOne::RushAttackCD, rushAttackCoolDown, false);
	GetWorld()->GetTimerManager().SetTimer(RushAttackTimerHandle, this, &AAlien_BreakOutBossOne::RushAttackDone, rushAttackDuration, false);
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::RockAttack, attackCoolDown, false);
}

void AAlien_BreakOutBossOne::SpecialRush() {
	rushAttackCD = true;
	specialRushing = true;
	GetWorld()->GetTimerManager().SetTimer(RushAttackTimerHandle, this, &AAlien_BreakOutBossOne::RushAttackDone, rushAttackDuration, false);

	specialRushStage++;
	switch (specialRushStage) {
	case 1: TeleportTo(teleportLocation[4]); break;
	case 2: TeleportTo(teleportLocation[1]); break;
	case 3: TeleportTo(teleportLocation[5]); break;
	case 4: GetWorld()->GetTimerManager().SetTimer(RushAttackCDTimerHandle, this, &AAlien_BreakOutBossOne::RushAttackCD, rushAttackCoolDown, false);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::RockAttack, attackCoolDown, false);
		specialRushing = false; specialRushStage = 0; TeleportTo(GetCloseLocation()); break;
	}
}

void AAlien_BreakOutBossOne::RushAttackDone() {
	if (rushAttacking) {
		rushAttacking = false;
		TeleportTo(GetCloseLocation());
	}
	else if (specialRushing) {
		SpecialRush();
	}
}

void AAlien_BreakOutBossOne::RushAttackCD() {
	rushAttackCD = false;
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

		rocks[rockLeft]->axis = FVector(0, FMath::Rand() % 100 - 50, FMath::Rand() % 100 - 50).GetSafeNormal();
		rocks[rockLeft]->angleAxis = 180.f;
		rocks[rockLeft]->rotateSpeed = 80.f;
		rocks[rockLeft]->dimention = FVector(330, 0, 0);

		rocks[rockLeft + 1]->axis = FVector(0, FMath::Rand() % 100 - 50, FMath::Rand() % 100 - 50).GetSafeNormal();
		rocks[rockLeft + 1]->angleAxis = 90.f;
		rocks[rockLeft + 1]->rotateSpeed = 80.f;
		rocks[rockLeft + 1]->dimention = FVector(240, 0, 0);

		rocks[rockLeft + 2]->axis = FVector(0, FMath::Rand() % 100 - 50, FMath::Rand() % 100 - 50).GetSafeNormal();
		rocks[rockLeft + 2]->angleAxis = -90.f;
		rocks[rockLeft + 2]->rotateSpeed = 80.f;
		rocks[rockLeft + 2]->dimention = FVector(180, 0, 0);

		rockLeft += 3;
	}
}

void AAlien_BreakOutBossOne::RockAttack() {
	if (specialRushing || rushAttacking) {
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::RockAttack, attackCoolDown, false);
		return;
	}

	Summon();
	int rand = FMath::RandRange(1, 125);
	if (rand < 30) {
		NormalThrow();
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::RockAttack, attackCoolDown, false);
	}
	else if (rand < 75) {
		ShotGunThrow();
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::RockAttack, attackCoolDown, false);
	}
	else if (rand <= 110) {
		specialRockAttacking = true;
		SequenceThrow();
	}
	else if (rand <= 125) {
		specialRockAttacking = true;
		SpreadThrow();
	}
	else {
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::RockAttack, attackCoolDown, false);
	}
	
}

void AAlien_BreakOutBossOne::NormalThrow() {
	if (rockLeft != 0) {
		int rand = FMath::RandRange(0, rockLeft - 1);
		rocks[rand]->readyFire();
		rocks.RemoveAt(rand);
		rockLeft--;
	}
}

void AAlien_BreakOutBossOne::ShotGunThrow() {
	for (int i = 0; i < 3; i++) {
		int rand = FMath::RandRange(0, rockLeft - 1);
		switch (i) {
		case 0: if (facingLeft) rocks[rand]->direction = FVector(0.0, 4.0, 1.0); else rocks[rand]->direction = FVector(0.0, -4.0, 1.0); break;
		case 1: if (facingLeft) rocks[rand]->direction = FVector(0.0, 4.0, 0.0); else rocks[rand]->direction = FVector(0.0, -4.0, 0.0); break;
		case 2: if (facingLeft) rocks[rand]->direction = FVector(0.0, 4.0, -1.0); else rocks[rand]->direction = FVector(0.0, -4.0, -1.0); break;
		}
		rocks[rand]->readyFire();
		rocks.RemoveAt(rand);
		rockLeft--;
	}
}

void AAlien_BreakOutBossOne::SequenceThrow() {
	Summon();
	SequenceThrowCounts++;
	NormalThrow();
	
	if(SequenceThrowCounts >= SequenceThrowTimes){
		SequenceThrowCounts = 0;
		specialRockAttacking = false;
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::RockAttack, attackCoolDown, false);
	}
	else
		GetWorld()->GetTimerManager().SetTimer(SqeuenceThrowTimerHandle, this, &AAlien_BreakOutBossOne::SequenceThrow, 0.5f, false);
}


void AAlien_BreakOutBossOne::SpreadThrow() {
	TeleportTo(FVector(-60.0, 200.0, 2600.0));

	Summon();
	SpreadThrowCounts++;

	int rand = FMath::RandRange(0, rockLeft - 1);
	switch (SpreadThrowCounts % 16) {
	case 15:  rocks[rand]->direction = FVector(0.0, -1.0, 0.0); break;
	case 0:  rocks[rand]->direction = FVector(0.0, -2.0, -1.0); break;
	case 1:  rocks[rand]->direction = FVector(0.0, -1.0, -1.0); break;
	case 2:  rocks[rand]->direction = FVector(0.0, -1.0, -2.0); break;
	case 3:  rocks[rand]->direction = FVector(0.0, 0.0, -1.0); break;
	case 4:  rocks[rand]->direction = FVector(0.0, 1.0, -2.0); break;
	case 5:  rocks[rand]->direction = FVector(0.0, 1.0, -1.0); break;
	case 6:  rocks[rand]->direction = FVector(0.0, 2.0, -1.0); break;
	case 7:  rocks[rand]->direction = FVector(0.0, 1.0, 0.0); break;
	case 8:  rocks[rand]->direction = FVector(0.0, 2.0, -1.0); break;
	case 9:  rocks[rand]->direction = FVector(0.0, 1.0, -1.0); break;
	case 10:  rocks[rand]->direction = FVector(0.0, 1.0, -2.0); break;
	case 11:  rocks[rand]->direction = FVector(0.0, 0.0, -1.0); break;
	case 12:  rocks[rand]->direction = FVector(0.0, -1.0, -2.0); break;
	case 13:  rocks[rand]->direction = FVector(0.0, -1.0, -1.0); break;
	case 14:  rocks[rand]->direction = FVector(0.0, -2.0, -1.0); break;

	}
	rocks[rand]->readyFire();
	rocks.RemoveAt(rand);
	rockLeft--;

	if (SpreadThrowCounts >= SpreadThrowTimes) {
		SpreadThrowCounts = 0;
		specialRockAttacking = false;
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AAlien_BreakOutBossOne::RockAttack, attackCoolDown, false);
		GetWorld()->GetTimerManager().SetTimer(TeleportTimerHandle, this, &AAlien_BreakOutBossOne::Teleport, 3, false);
	}
	else
		GetWorld()->GetTimerManager().SetTimer(SpreadThrowTimerHandle, this, &AAlien_BreakOutBossOne::SpreadThrow, 0.2f, false);
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
	}
}

void AAlien_BreakOutBossOne::hitByPlayer(float minsHP) {
	this->HP -= minsHP;
	if (HP <= 0.f) {
		UGameplayStatics::OpenLevel(GetWorld(), "Credits");
		// Death
	}
}

void AAlien_BreakOutBossOne::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) {
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	UE_LOG(LogTemp, Warning, TEXT("HIT!"));
	if (Other->IsA(AAlien_OutbreakCharacter::StaticClass())) {
		AAlien_OutbreakCharacter* player = (AAlien_OutbreakCharacter*)GetWorld()->GetFirstPlayerController()->GetPawn();
		if (!player->Avoiding && !player->Invincible) {
			player->beingHit(0.08f, GetActorLocation().Y);
		}
	}
}