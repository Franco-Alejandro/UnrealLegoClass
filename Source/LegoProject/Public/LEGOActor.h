

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
    // TODO: LOS can be affected by external actors. We should add a button to refresh data or bake this data when cooking. 
    // There is no way right now in which we can monitor external actors getting in the middle of two LEGO actors, unless we have an overarching
    // system that takes care of tackling this.
    bool bHasLineOfSight = false;
    FVector ClosestPointOnSphere = FVector::ZeroVector;
    float ForwardAngleDegrees = 0.f;
};

UCLASS()
class LEGOPROJECT_API ALEGOActor : public AActor
{
	GENERATED_BODY()
	
public:
    ALEGOActor();
#if WITH_EDITOR
    virtual void PostEditMove(bool InFinished) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& InEvent) override;
    virtual void PostDuplicate(bool InDuplicateForPIE) override;

    void RebuildDerivedDataForAllConnections();
    void RebuildDerivedDataForConnection(ALEGOActor& InOther);
    void RemoveDerivedDataForConnection(const ALEGOActor& InOther);

    static bool ValidateLEGOActors(const TArray<AActor*>& InActors, TArray<ALEGOActor*>& OutValidActors, ULevel* OutCommonLevel, const TCHAR* InContextName);

    UFUNCTION(CallInEditor, BlueprintCallable)
    static void ConnectLEGOActors(const TArray<AActor*>& InActors);

    UFUNCTION(CallInEditor, BlueprintCallable)
    static void DisconnectLEGOActors(const TArray<AActor*>& InActors);

    bool AddConnection(ALEGOActor& InOtherActor);
    bool RemoveConnection(ALEGOActor& InOtherActor);
    bool IsConnectedTo(const ALEGOActor& InOtherActor) const;

    bool CheckLineOfSight(const ALEGOActor& InOther) const;
    FVector CalculateClosestPointOnSphere(const ALEGOActor& InOther) const;
    float CalculateForwardAngleDegrees(const ALEGOActor& InOther) const;

    bool bIsRebuildingDerivedData = false;
#endif

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