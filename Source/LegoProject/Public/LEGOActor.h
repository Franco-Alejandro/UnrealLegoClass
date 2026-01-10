

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LEGOActor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLEGOActorConnections, Log, All);

UENUM(BlueprintType)
enum class EShapeType : uint8
{
    Box     UMETA(DisplayName = "Box"),
    Sphere  UMETA(DisplayName = "Sphere"),
    Capsule UMETA(DisplayName = "Capsule"),
    Convex  UMETA(DisplayName = "Convex")
};

struct FDerivedConnectionData
{
    bool bHasLineOfSight = false;
    FVector ClosestPointOnSphere = FVector::ZeroVector;
    float ForwardAngleDegrees = 0.f;
};

UCLASS()
class LEGOPROJECT_API ALEGOActor : public AActor
{
	GENERATED_BODY()
	
	ALEGOActor();

#if WITH_EDITOR

    virtual void PostEditMove(bool InFinished) override;

    virtual void PostEditChangeProperty(FPropertyChangedEvent& InEvent) override;


    static bool ValidateLEGOActors(const TArray<AActor*>& InActors, TArray<ALEGOActor*>& OutValidActors, ULevel* OutCommonLevel, const TCHAR* ContextName);

    UFUNCTION(CallInEditor, BlueprintCallable)
    static void ConnectLEGOActors(const TArray<AActor*>& InActors);

    UFUNCTION(CallInEditor, BlueprintCallable)
    static void DisconnectLEGOActors(const TArray<AActor*>& InActors);

    bool AddConnection(ALEGOActor& InOtherActor);
    bool RemoveConnection(ALEGOActor& InOtherActor);
    bool IsConnectedTo(ALEGOActor& InOtherActor) const;
    bool CheckLineOfSight(const ALEGOActor& InOther) const;
    FVector CalculateClosestPointOnSphere(const ALEGOActor& InOther) const;
    float CalculateForwardAngleDegrees(const ALEGOActor& InOther) const;
    void RebuildDerivedData();
#endif

 protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALEGOActor")
    FColor Color = FColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALEGOActor")
    EShapeType Shape = EShapeType::Box;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALEGOActor", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float Size = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALEGOActor")
    TArray<ALEGOActor*> ConnectedActors;

    TMap<TWeakObjectPtr<ALEGOActor>, FDerivedConnectionData> DerivedData;

};