


#include "Misc/AutomationTest.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/Paths.h"

#include "LEGOActor.h"
#include "LegoActorSerializationSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FLEGOActorSerializationTest,
    "LegoProject.Serialization.ALEGOActor",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

static void DestroyAllLEGOActors(UWorld& World)
{
    TArray<ALEGOActor*> actorToDestroy;

    // TODO FA: fix naive way to obtain all lego actors
    for (TActorIterator<ALEGOActor> iterator(&World); iterator; ++iterator)
    {
        actorToDestroy.Add(*iterator);
    }

    for (ALEGOActor* legoActor : actorToDestroy)
    {
        if (IsValid(legoActor))
        {
            legoActor->Modify();
            legoActor->Destroy();
        }
    }
}

bool FLEGOActorSerializationTest::RunTest(const FString& Parameters)
{
    UWorld* world = GEditor->GetEditorWorldContext().World();
    TestNotNull(TEXT("Editor world exists"), world);
    if (!world)
    {
        return false;
    }

    ULEGOActorSerializationSubsystem* serializationSubsystem =
        GEditor->GetEditorSubsystem<ULEGOActorSerializationSubsystem>();

    TestNotNull(TEXT("Serialization subsystem exists"), serializationSubsystem);
    if (!serializationSubsystem)
    {
        return false;
    }

    TArray<ALEGOActor*> legoActors;
    TArray<FLEGOActorSerializedData> dataSerialized;
    for (TActorIterator<ALEGOActor> it(world); it; ++it)
    {
        ALEGOActor* legoActor = *it;
        if (!IsValid(legoActor))
            continue;

        legoActors.Add(*it);

        FLEGOActorSerializedData legoActorData;
        legoActorData.ActorName = legoActor->GetActorLabel();
        legoActorData.Transform = legoActor->GetActorTransform();
        legoActorData.Color = legoActor->Color;
        legoActorData.Shape = legoActor->Shape;
        legoActorData.Size = legoActor->Size;
        legoActorData.ActorClassPath = legoActor->GetClass()->GetPathName();

        dataSerialized.Add(legoActorData);
    }

    const bool bUsingExistingActors = legoActors.Num() >= 2;

    if (!bUsingExistingActors)
    {
        ALEGOActor* A = world->SpawnActor<ALEGOActor>();
        ALEGOActor* B = world->SpawnActor<ALEGOActor>();

        A->Color = FColor::Red;
        B->Color = FColor::Blue;

        A->AddConnection(*B);

        legoActors = { A, B };

        TestNotNull(TEXT("Actor A spawned"), A);
        TestNotNull(TEXT("Actor B spawned"), B);

        A->Color = FColor::Red;
        B->Color = FColor::Blue;
        A->Size = 42.f;
        B->Size = 84.f;

        FLEGOActorSerializedData legoActorDataA;
        legoActorDataA.ActorName = A->GetActorLabel();
        legoActorDataA.Transform = A->GetActorTransform();
        legoActorDataA.Color = A->Color;
        legoActorDataA.Shape = A->Shape;
        legoActorDataA.Size = A->Size;
        legoActorDataA.ActorClassPath = A->GetClass()->GetPathName();

        dataSerialized.Add(legoActorDataA);

        FLEGOActorSerializedData legoActorDataB;
        legoActorDataB.ActorName = B->GetActorLabel();
        legoActorDataB.Transform = B->GetActorTransform();
        legoActorDataB.Color = B->Color;
        legoActorDataB.Shape = B->Shape;
        legoActorDataB.Size = B->Size;
        legoActorDataB.ActorClassPath = B->GetClass()->GetPathName();

        dataSerialized.Add(legoActorDataB);
    }

    const FString serializationFilePath =
        FPaths::ProjectSavedDir() / TEXT("Automation_LEGOActors.json");

    TestTrue(
        TEXT("SerializeCurrentLevel succeeds"),
        serializationSubsystem->SerializeCurrentLevel(serializationFilePath)
    );

    DestroyAllLEGOActors(*world);

    TestTrue(
        TEXT("DeserializeIntoCurrentLevel succeeds"),
        serializationSubsystem->DeserializeIntoCurrentLevel(serializationFilePath)
    );

    legoActors.Reset(dataSerialized.Num());

    for (TActorIterator<ALEGOActor> it(world); it; ++it)
    {
        ALEGOActor* legoActor = *it;
        if (!IsValid(legoActor))
            continue;

        legoActors.Add(*it);
    }

    TestTrue(
        TEXT("Restored actors"),
        dataSerialized.Num() == legoActors.Num()
    );

    return true;
}
