// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"



#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

#include "GameFramework/Character.h"
#include "Alien_OutbreakCharacter.generated.h"


UCLASS(config = Game)

class AAlien_OutbreakCharacter : public ACharacter
{
	GENERATED_BODY()

		/** Side view camera */
//		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
//		class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
//		class USpringArmComponent* CameraBoom;

protected:

	/** Called for side to side input */
	void MoveRight(float Val);

	/** Handle touch inputs. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handle touch stop event. */
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	void AirDash();

	//void AdjustCamera();

	void PAttack();

	void Grab();

	void Pause();

	UFUNCTION()
		void AirDashCD();

	UFUNCTION()
		void AirDashStop();


	UFUNCTION()
		void AvoidStop();

	UFUNCTION()
		void InvincibleStop();
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	FTimerHandle AirDashCDTimerHandle;
	FTimerHandle AirDashingTimerHandle;
	FTimerHandle AvoidTimerHandle;

	FTimerHandle PAttackTimerHandle;
	FTimerHandle InvincibleTimerHandle;

	bool facingRight = true;

	FVector prevLoc;
public:
	AAlien_OutbreakCharacter();
	void PlayerHP_Setter(float new_HP);
	void beingHit(float minsHP, float rockY);

	/** Returns SideViewCameraComponent subobject **/
	//FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	//FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UPROPERTY(BlueprintReadOnly)
		bool isAttacking;

	UPROPERTY(BlueprintReadOnly)
		bool isHolding;

	UPROPERTY(BlueprintReadOnly)
		bool isThrowing;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


public:
	UPROPERTY(BlueprintReadOnly)
		float HP = 1.0f;
	UPROPERTY()
		TSubclassOf<class UPlayerHPWidget> WidgetClass;
	bool Avoiding;
	bool Invincible;


	void damagePlayer(float damageTaken, float knockback);

	void processHit();


private:
	USoundWave* HurtSound1;
	USoundWave* HurtSound2;
	USoundWave* HurtSound3;
	USoundWave* HurtSound4;
	USoundWave* HurtSound5;
	void playHurtSound(int num);

	bool knockToLeft;
	bool knockingBack;
	float knockBackTime;
	float knockBackSpeed;
	int fps;
	int knockBackCount;

	float AttackCD;
	
	float DashCD;
	bool DashCDing;
	float AvoidTime;

	bool Dashing;
	float dashSpeed;

	float invincibleTime;

	enum GameStates { IDLE, MOVE, JUMP, DASH, ATTACK, HURT, DEATH, GRAB, THROW, PAUSE };
	GameStates State = GameStates::IDLE;

	enum GameEvents { ON_ENTER, ON_UPDATE };
	GameEvents Event = GameEvents::ON_ENTER;

	void FSMUpdate(GameStates nState);
	void SetFSMState(GameStates newState);

	void Idle_Enter();
	void Idle_Update();
	void Idle_Exit();

	void Move_Enter();
	void Move_Update();
	void Move_Exit();

	void Jump_Enter();
	void Jump_Update();
	void Jump_Exit();

	void Dash_Enter();
	void Dash_Update();
	void Dash_Exit();

	void Attack_Enter();
	void Attack_Update();
	void Attack_Exit();

	void Hurt_Enter();
	void Hurt_Update();
	void Hurt_Exit();

	void Death_Enter();
	void Death_Update();
	void Death_Exit();

	void Grab_Enter();
	void Grab_Update();
	void Grab_Exit();

	void Throw_Enter();
	void Throw_Update();
	void Throw_Exit();

	void Pause_Enter();
	void Pause_Update();
	void Pause_Exit();


};

