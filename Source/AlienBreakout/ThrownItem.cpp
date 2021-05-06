// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrownItem.h"
#include "Alien_BreakOutBossOne.h"
#include "Alien_OutbreakCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AThrownItem::AThrownItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AThrownItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AThrownItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AThrownItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(AAlien_BreakOutBossOne::StaticClass())) {
		UE_LOG(LogTemp, Warning, TEXT("Player hit boss!"));

		((AAlien_BreakOutBossOne*)OtherActor)->hitByPlayer(0.15f);
		this->Destroy();
	}

}

