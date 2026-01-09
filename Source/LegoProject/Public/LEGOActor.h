

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

UCLASS()
class LEGOPROJECT_API ALEGOActor : public AActor
{
	GENERATED_BODY()
	
	ALEGOActor();

#if WITH_EDITOR
    UFUNCTION(CallInEditor, BlueprintCallable)
    static void ConnectLEGOActors(const TArray<AActor*>& InActors);

    UFUNCTION(CallInEditor, BlueprintCallable)
    static void DisconnectLEGOActors(const TArray<AActor*>& InActors);

    void AddConnection(ALEGOActor* InOtherActor);
    void RemoveConnection(ALEGOActor* InOtherActor);
    bool IsConnectedTo(ALEGOActor* InOtherActor) const;
#endif

 protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALEGOActor")
    FColor Color = FColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALEGOActor")
    EShapeType Shape = EShapeType::Box;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALEGOActor", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float Size = 50.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALEGOActor")
    TArray<TObjectPtr<ALEGOActor>> ConnectedActors;
	
};