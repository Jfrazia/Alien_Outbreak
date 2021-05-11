// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RockProjectileActor.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "Alien_BreakOutBossOne.generated.h"

UCLASS()
class ALIENBREAKOUT_API AAlien_BreakOutBossOne : public ACharacter
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	AAlien_BreakOutBossOne();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	int timeTick;
	int fps;
	double attackCoolDown;
	double teleportCoolDown;

	double rushAttackCoolDown;
	double rushAttackWaitTime;
	double rushAttackSpeed;
	double rushAttackDuration;
	bool rushAttackCD;
	bool rushAttacking;

	bool specialRushing;
	int specialRushStage;
	
	int maxNumRock;
	int rockLeft;
	TArray<ARockProjectileActor*> rocks;

	TArray<FVector> teleportLocation;

	FTimerHandle TeleportTimerHandle;
	FTimerHandle AttackTimerHandle;
	FTimerHandle SqeuenceThrowTimerHandle;
	FTimerHandle SpreadThrowTimerHandle;
	FTimerHandle RushAttackWaitTimerHandle;
	FTimerHandle RushAttackCDTimerHandle;
	FTimerHandle RushAttackTimerHandle;

	void Teleport();
	void TeleportTo(FVector teleportLocation);
	FVector GetCloseLocation();
	void Summon();

	void RushAttack();
	void NormalRush();
	void RushAttackDone();
	void RushAttackCD();
	void SpecialRush();

	void RockAttack();
	void NormalThrow();
	void ShotGunThrow();
	void SequenceThrow();
	void SpreadThrow();
	int SequenceThrowTimes;
	int SequenceThrowCounts;
	
	bool specialRockAttacking;

	int SpreadThrowTimes;
	int SpreadThrowCounts;

	bool facingLeft;

public:
	UPROPERTY(BlueprintReadOnly)
		float HP = 1.0f;
	UPROPERTY(EditAnywhere)
		TSubclassOf<class UBossHPWidget> WidgetClass;

	void hitByPlayer(float minsHP);

	UParticleSystem* ParticleTeleport;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	
};
