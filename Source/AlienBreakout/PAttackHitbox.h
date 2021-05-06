// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PAttackHitbox.generated.h"

UCLASS()
class ALIENBREAKOUT_API APAttackHitbox : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APAttackHitbox();
	class UStaticMeshComponent* SphereMesh;
	float Speed;
	FVector forward;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void DeleteBullet();

};
