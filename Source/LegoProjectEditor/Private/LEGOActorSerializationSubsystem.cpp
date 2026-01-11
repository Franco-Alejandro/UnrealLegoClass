#include "LEGOActorSerializationSubsystem.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogLEGOActorSerialization);

bool ULEGOActorSerializationSubsystem::SerializeCurrentLevel(const FString& InPath)
{
    UWorld* world = GEditor->GetEditorWorldContext().World();
    if (!world)
        return false;

    UE_LOG(LogLEGOActorSerialization, Log,
        TEXT("ULEGOActorSerializationSubsystem::SerializeCurrentLevel Serializing LEGO actors."));

    FLEGOLevelSerializedData levelData;
    for (TActorIterator<ALEGOActor> it(world); it; ++it)
    {
        ALEGOActor* legoActor = *it;
        if (!IsValid(legoActor))
            continue;

        FLEGOActorSerializedData legoActorData;
        legoActorData.ActorName = legoActor->GetActorLabel();
        legoActorData.Transform = legoActor->GetActorTransform();
        legoActorData.Color = legoActor->Color;
        legoActorData.Shape = legoActor->Shape;
        legoActorData.Size = legoActor->Size;
        legoActorData.ActorClassPath = legoActor->GetClass()->GetPathName();

        for (ALEGOActor* Connected : legoActor->ConnectedActors)
        {
            if (IsValid(Connected))
                legoActorData.ConnectedActorNames.Add(Connected->GetActorLabel());
        }

        levelData.Actors.Add(MoveTemp(legoActorData));

        UE_LOG(LogLEGOActorSerialization, Log,
            TEXT("ULEGOActorSerializationSubsystem::SerializeCurrentLevel Staging '%s' for saving."),
            *legoActor->GetName());
    }

    FString jsonString;
    if (!FJsonObjectConverter::UStructToJsonObjectString(levelData, jsonString))
    {
        ensureMsgf(false, TEXT("ULEGOActorSerializationSubsystem::SerializeCurrentLevel Json format could not be saved!"));
        return false;
    }

    return FFileHelper::SaveStringToFile(jsonString, *InPath);
}

bool ULEGOActorSerializationSubsystem::DeserializeIntoCurrentLevel(const FString& InPath)
{
    FString jsonString;
    if (!FFileHelper::LoadFileToString(jsonString, *InPath))
        return false;

    FLEGOLevelSerializedData levelData;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &levelData))
    {
        UE_LOG(LogLEGOActorSerialization, Error,
            TEXT("ULEGOActorSerializationSubsystem::DeserializeIntoCurrentLevel Failed to serialize JSON in [%s]."), *InPath);
        return false;
    }

    UWorld* world = GEditor->GetEditorWorldContext().World();
    if (!world)
        return false;

    UE_LOG(LogLEGOActorSerialization, Log,
        TEXT("ULEGOActorSerializationSubsystem::DeserializeIntoCurrentLevel Deserializing LEGO actors."));

    TMap<FString, ALEGOActor*> spawnedActors;

    for (const FLEGOActorSerializedData& dataStored : levelData.Actors)
    {
        UClass* actorClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, *dataStored.ActorClassPath));
        if (!actorClass)
        {
            // might not be able to find the class, could fallback to ALEGOActor::StaticClass()
            UE_LOG(LogLEGOActorSerialization, Error,
                TEXT("ULEGOActorSerializationSubsystem::DeserializeIntoCurrentLevel Class [%s] was not found, we are skipping this actor and it will not be spawned."),
                *dataStored.ActorClassPath);
            continue;
        }

        ALEGOActor* legoActor = world->SpawnActor<ALEGOActor>(actorClass, dataStored.Transform);
        if (!legoActor)
            continue;

        legoActor->SetActorLabel(dataStored.ActorName);
        legoActor->Color = dataStored.Color;
        legoActor->Shape = dataStored.Shape;
        legoActor->Size = dataStored.Size;

        spawnedActors.Add(dataStored.ActorName, legoActor);
    }

    // we need to iterate again, because there was no guarantee actors were spawned to make a connection on previous loop
    for (const FLEGOActorSerializedData& data : levelData.Actors)
    {
        ALEGOActor* legoActor = spawnedActors.FindRef(data.ActorName);
        if (!legoActor)
            continue;

        for (const FString& connectedName : data.ConnectedActorNames)
        {
            ALEGOActor* connectedLEGOActor = spawnedActors.FindRef(connectedName);
            if (connectedLEGOActor)
                legoActor->AddConnection(*connectedLEGOActor);
        }
    }

    UE_LOG(LogLEGOActorSerialization, Log,
        TEXT("ULEGOActorSerializationSubsystem::DeserializeIntoCurrentLevel Spawned [%d] actors."), spawnedActors.Num());

    return true;
}