#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "LEGOActor.h"
#include "LEGOActorSerializationSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLEGOActorSerialization, Log, All);

// Data structs for serialization
USTRUCT()
struct FLEGOActorSerializedData
{
    GENERATED_BODY()

    UPROPERTY()
    FString ActorName;

    UPROPERTY()
    FTransform Transform;

    UPROPERTY()
    FColor Color;

    UPROPERTY()
    EShapeType Shape;

    UPROPERTY()
    float Size = 50.f;

    UPROPERTY()
    TArray<FString> ConnectedActorNames;

    // Could improve by soft object ptr
    FString ActorClassPath;
};

USTRUCT()
struct FLEGOLevelSerializedData
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FLEGOActorSerializedData> Actors;
};

/**
 * Used to serialize and deserialize all ALEGOActors in the current level.
 * Notes FA: could be a place to hold states for future operations when serializing or deserializing, depending on the complexity of lego actors
 */
UCLASS()
class ULEGOActorSerializationSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    /** Serialize all ALEGOActors in the current editor level to a JSON file */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "ALEGOActor|Serialization")
    bool SerializeCurrentLevel(const FString& InPath);

    /** Deserialize ALEGOActors from a JSON file and spawn them in the current editor level */
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "ALEGOActor|Serialization")
    bool DeserializeIntoCurrentLevel(const FString& InPath);
};
