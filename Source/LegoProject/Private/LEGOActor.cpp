


#include "LEGOActor.h"

DEFINE_LOG_CATEGORY(LogLEGOActorConnections);

ALEGOActor::ALEGOActor()
{
}

#if WITH_EDITOR

void ALEGOActor::ConnectLEGOActors(const TArray<AActor*>& InActors)
{
    // TODO: Move validation to common place
    TArray<ALEGOActor*> validLEGOActors;
    ULevel* commonLevel = nullptr;

    if (InActors.Num() < 2)
    {
        UE_LOG(LogLEGOActorConnections, Error,
            TEXT("ALEGOActor::ConnectLEGOActors ConnectActors requires at least two valid actor instances."));
        return;
    }

    for (AActor* actor : InActors)
    {
        if (!IsValid(actor))
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("ALEGOActor::ConnectLEGOActors Null actor passed to ConnectActors."));
            continue;
        }
       
        ALEGOActor* legoActor = Cast<ALEGOActor>(actor);
        if (!IsValid(legoActor))
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("ALEGOActor::ConnectLEGOActors Actor '%s' is not an ALEGOActor and will be ignored."),
                *actor->GetName());
            continue;
        }               

        if (!commonLevel)
        {
            commonLevel = legoActor->GetLevel();
        }
        else if (legoActor->GetLevel() != commonLevel)
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("ALEGOActor::ConnectLEGOActors Actor '%s' is in a different level and will be ignored."),
                *legoActor->GetName());
            continue;
        }

        validLEGOActors.AddUnique(legoActor);
    }

    if (validLEGOActors.Num() < 2)
    {
        UE_LOG(LogLEGOActorConnections, Error,
            TEXT("ALEGOActor::ConnectLEGOActors ConnectActors requires at least two valid AMyActor instances."));
        return;
    }

    int32 connectionCount = 0;

    for (int32 i = 0; i < validLEGOActors.Num(); ++i)
    {
        ALEGOActor* previousLEGOActor = validLEGOActors[i];

        for (int32 j = i + 1; j < validLEGOActors.Num(); ++j)
        {
            ALEGOActor* subsequentLEGOActor = validLEGOActors[j];

            if (!previousLEGOActor->ConnectedActors.Contains(subsequentLEGOActor))
            {
                previousLEGOActor->Modify();
                subsequentLEGOActor->Modify();

                previousLEGOActor->ConnectedActors.Add(subsequentLEGOActor);
                subsequentLEGOActor->ConnectedActors.Add(previousLEGOActor);

                ++connectionCount;

                UE_LOG(LogLEGOActorConnections, Log,
                    TEXT("ALEGOActor::ConnectLEGOActors Connected '%s' <-> '%s'"),
                    *previousLEGOActor->GetName(), *subsequentLEGOActor->GetName());
            }
        }
    }

    UE_LOG(LogLEGOActorConnections, Log,
        TEXT("ALEGOActor::ConnectLEGOActors ConnectActors completed: %d connections created."),
        connectionCount);

}
void ALEGOActor::DisconnectLEGOActors(const TArray<AActor*>& InActors)
{
    // TODO: Move validation to common place
    TArray<ALEGOActor*> validLEGOActors;
    ULevel* commonLevel = nullptr;

    if (InActors.Num() < 2)
    {
        UE_LOG(LogLEGOActorConnections, Error,
            TEXT("ALEGOActor::DisconnectLEGOActors ConnectActors requires at least two valid actor instances."));
        return;
    }

    for (AActor* actor : InActors)
    {
        if (!IsValid(actor))
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("ALEGOActor::DisconnectLEGOActors Null actor passed to ConnectActors."));
            continue;
        }

        ALEGOActor* legoActor = Cast<ALEGOActor>(actor);
        if (!IsValid(legoActor))
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("ALEGOActor::DisconnectLEGOActors Actor '%s' is not an ALEGOActor and will be ignored."),
                *actor->GetName());
            continue;
        }

        if (!commonLevel)
        {
            commonLevel = legoActor->GetLevel();
        }
        else if (legoActor->GetLevel() != commonLevel)
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("ALEGOActor::DisconnectLEGOActors Actor '%s' is in a different level and will be ignored."),
                *legoActor->GetName());
            continue;
        }

        validLEGOActors.AddUnique(legoActor);
    }

    if (validLEGOActors.Num() < 2)
    {
        UE_LOG(LogLEGOActorConnections, Error,
            TEXT("ALEGOActor::DisconnectLEGOActors ConnectActors requires at least two valid AMyActor instances."));
        return;
    }

    int32 connectionCount = 0;

    for (int32 i = 0; i < validLEGOActors.Num(); ++i)
    {
        ALEGOActor* previousLEGOActor = validLEGOActors[i];

        for (int32 j = i + 1; j < validLEGOActors.Num(); ++j)
        {
            ALEGOActor* subsequentLEGOActor = validLEGOActors[j];

            if (previousLEGOActor->ConnectedActors.Contains(subsequentLEGOActor))
            {
                previousLEGOActor->Modify();
                subsequentLEGOActor->Modify();

                previousLEGOActor->ConnectedActors.Remove(subsequentLEGOActor);
                subsequentLEGOActor->ConnectedActors.Remove(previousLEGOActor);

                ++connectionCount;

                UE_LOG(LogLEGOActorConnections, Log,
                    TEXT("ALEGOActor::DisconnectLEGOActors Removed Connection '%s' <- / -> '%s'"),
                    *previousLEGOActor->GetName(), *subsequentLEGOActor->GetName());
            }
        }
    }

    UE_LOG(LogLEGOActorConnections, Log,
        TEXT("ALEGOActor::DisconnectLEGOActors DisconnectActors completed: %d connections removed."),
        connectionCount);
}

void ALEGOActor::AddConnection(ALEGOActor* InOtherActor)
{
    if (!InOtherActor || InOtherActor == this)
        return;

    if (GetLevel() != InOtherActor->GetLevel())
        return;

    if (!ConnectedActors.Contains(InOtherActor))
    {
        ConnectedActors.Add(InOtherActor);
    }

    if (!InOtherActor->ConnectedActors.Contains(this))
    {
        InOtherActor->ConnectedActors.Add(this);
    }
}

void ALEGOActor::RemoveConnection(ALEGOActor* InOtherActor)
{
    if (!InOtherActor)
        return;

    ConnectedActors.Remove(InOtherActor);
    InOtherActor->ConnectedActors.Remove(this);
}

bool ALEGOActor::IsConnectedTo(ALEGOActor* InOtherActor) const
{
    return ConnectedActors.Contains(InOtherActor);
}
#endif


