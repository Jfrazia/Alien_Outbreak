// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"

#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

#include "RockProjectileActor.generated.h"

UCLASS()
class ALIENBREAKOUT_API ARockProjectileActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARockProjectileActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	USoundWave* StoneSound1;
	USoundWave* StoneSound2;
	void playStoneSound(int num);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UParticleSystem* ParticleRock;

	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* Mesh;

	float angleAxis;
	FVector dimention;
	FVector axis;
	float rotateSpeed;
	void rotating(float DeltaTime);

	int timeTick;
	bool firing;
	FVector direction;

	float fireSpeed;
	void readyFire();
	void rotateSelf(float DeltaTime);

	FVector readyLoc;
	FVector readyDirection;

	float aliveTime;
	int hp;

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
