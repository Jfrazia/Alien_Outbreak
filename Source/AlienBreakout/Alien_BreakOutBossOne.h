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
	FTimerHandle RushAttackWaitTimerHandle;
	FTimerHandle RushAttackTimerHandle;

	void Teleport();
	void TeleportTo(int index);
	void Summon();

	void RushAttack();
	void RushAttackDone();
	void SpecialRush();

	void RockAttack();
	void NormalThrow();
	void ShotGunRock();
	void SequenceThrow();
	int SequenceThrowTimes;
	int SequenceThrowCounts;

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
