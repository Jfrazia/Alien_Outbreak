// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RockProjectileActor.h"
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
	int summonCoolDown;
	int attackCoolDown;
	int teleportCoolDown;
	
	int maxNumRock;
	int rockLeft;
	TArray<ARockProjectileActor*> rocks;

	TArray<FVector> teleportLocation;

	FTimerHandle TeleportTimerHandle;
	FTimerHandle SummonTimerHandle;
	FTimerHandle AttackTimerHandle;

	void Teleport();
	void Summon();
	void Attack();

public:
	UPROPERTY(BlueprintReadOnly)
		float HP = 1.0f;
	UPROPERTY(EditAnywhere)
		TSubclassOf<class UBossHPWidget> WidgetClass;

	void hitByPlayer(float minsHP);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	enum GameStates { IDLE, SUMMON, ATTACK, TELEPORT };
	GameStates State = GameStates::IDLE;

	enum GameEvents { ON_ENTER, ON_UPDATE };
	GameEvents Event = GameEvents::ON_ENTER;

	void FSMUpdate();
	void SetFSMState(GameStates newState);

	void Idle_Enter();
	void Idle_Update();
	void Idle_Exit();

	void Summon_Enter();
	void Summon_Update();
	void Summon_Exit();

	void Attack_Enter();
	void Attack_Update();
	void Attack_Exit();

	void Teleport_Enter();
	void Teleport_Update();
	void Teleport_Exit();

};
