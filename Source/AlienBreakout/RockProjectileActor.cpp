// Fill out your copyright notice in the Description page of Project Settings.


#include "RockProjectileActor.h"
#include "Kismet/GameplayStatics.h"
#include "Alien_BreakOutBossOne.h"
#include "Alien_OutbreakCharacter.h"
#include "PAttackHitbox.h"


// Sets default values
ARockProjectileActor::ARockProjectileActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");

	int rand = FMath::RandRange(1, 4);
	FString assetLocation;
	switch (rand) {
	case 1: assetLocation = TEXT("StaticMesh'/Game/Objects/Rock'"); break;
	case 2: assetLocation = TEXT("StaticMesh'/Game/Objects/Rock2'"); break;
	case 3: assetLocation = TEXT("StaticMesh'/Game/Objects/Rock3'"); break;
	case 4: assetLocation = TEXT("StaticMesh'/Game/Objects/Rock4'"); break;
		//case 5: assetLocation = TEXT("StaticMesh'/Game/Objects/UnwrappedRock'"); break;
	default: assetLocation = TEXT("StaticMesh'/Game/Objects/Rock'"); break;
	}

	ConstructorHelpers::FObjectFinder<UStaticMesh>rockAsset(*assetLocation);
	Mesh->SetStaticMesh(rockAsset.Object);
	Mesh->SetCollisionProfileName(TEXT("OverlapAll"));

	RootComponent = Mesh;

	fireSpeed = 25.f;
	dimention = FVector(300, 0, 0);
	axis = FVector(0, 0, 1);
	rotateSpeed = 90.f;

	timeTick = 0;
	waitTime = 0.5;
	readyToFire = false;
	firing = false;
	//direction;
}



void ARockProjectileActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor->IsA(AAlien_OutbreakCharacter::StaticClass())) {

		float rockY = this->GetActorLocation().Y;
		AAlien_OutbreakCharacter* player = (AAlien_OutbreakCharacter*)GetWorld()->GetFirstPlayerController()->GetPawn();
		if (!player->Avoiding && !player->Invincible) {
			((AAlien_OutbreakCharacter*)GetWorld()->GetFirstPlayerController()->GetPawn())->beingHit(0.04f, rockY);
			this->Destroy();
		}
	}
	else if (OtherActor->IsA(AAlien_BreakOutBossOne::StaticClass()) || OtherActor->IsA(ARockProjectileActor::StaticClass())) {
	}
	else if (OtherActor->IsA(APAttackHitbox::StaticClass())) {
		GetWorld()->DestroyActor(OtherActor);
		this->Destroy();
	}
}

// Called when the game starts or when spawned
void ARockProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &ARockProjectileActor::OnOverlapBegin);
	//Mesh->OnComponentHit.AddDynamic(this, &ARockProjectileActor::OnActorHit);
}

// Called every frame
void ARockProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (readyToFire) {
		timeTick++;
		//FVector distPerTick = readyDirection;
		//distPerTick.X /= 
		if (!firing)this->SetActorLocation(GetActorLocation() + readyDirection / (waitTime * 60));
		if (timeTick >= waitTime * 60 && !firing) {
			FVector targetLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
			FVector curLocation = GetActorLocation();
			direction = (targetLocation - curLocation);
			direction.Normalize();
			firing = true;
		}
		if (firing) {
			fireAtPlayer();
		}
	}
	else {
		rotating(DeltaTime);
	}
}

void ARockProjectileActor::rotating(float DeltaTime) {
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAlien_BreakOutBossOne::StaticClass(), FoundActors);

	FVector NewLocation = this->GetActorLocation();

	for (AActor* Actor : FoundActors)
	{
		NewLocation = Actor->GetActorLocation();
	}
	angleAxis += DeltaTime * rotateSpeed;

	if (angleAxis >= 360) {
		angleAxis = 0;
	}

	FVector rotateValue = dimention.RotateAngleAxis(angleAxis, axis);

	NewLocation.X += rotateValue.X;
	NewLocation.Y += rotateValue.Y;
	NewLocation.Z += rotateValue.Z;

	FRotator NewRotation = FRotator(0, angleAxis, 0);
	FQuat QuatRotation = FQuat(NewRotation);
	SetActorLocationAndRotation(NewLocation, QuatRotation, false, 0, ETeleportType::None);
}

void ARockProjectileActor::readyFire() {
	// Set to align with player
	readyLoc = GetActorLocation();
	readyLoc.X = 0;
	readyDirection = readyLoc - GetActorLocation();
	readyToFire = true;
}

void ARockProjectileActor::fireAtPlayer() {
	//UE_LOG(LogTemp, Error, TEXT("Firing Direction: %s"), *direction.ToString());
	SetActorLocation(GetActorLocation() + direction * fireSpeed);
	return;
}