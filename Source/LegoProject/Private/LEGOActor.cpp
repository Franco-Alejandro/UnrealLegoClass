


#include "LEGOActor.h"

#if WITH_EDITOR
#include "ScopedTransaction.h"
#endif

DEFINE_LOG_CATEGORY(LogLEGOActorConnections);

ALEGOActor::ALEGOActor()
{
}

bool ALEGOActor::ValidateLEGOActors(const TArray<AActor*>& InActors, TArray<ALEGOActor*>& OutValidActors, ULevel* OutCommonLevel, const TCHAR* ContextName)
{
    OutValidActors.Reset();
    OutCommonLevel = nullptr;

    if (InActors.Num() < 2)
    {
        UE_LOG(LogLEGOActorConnections, Error,
            TEXT("%s requires at least two actor instances."), ContextName);
        return false;
    }

    for (AActor* actor : InActors)
    {
        if (!IsValid(actor))
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("%s: Null or invalid actor passed."), ContextName);
            continue;
        }

        ALEGOActor* LEGOActor = Cast<ALEGOActor>(actor);
        if (!IsValid(LEGOActor))
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("%s: Actor '%s' is not an ALEGOActor and will be ignored."),
                ContextName, *actor->GetName());
            continue;
        }

        if (!OutCommonLevel)
        {
            OutCommonLevel = LEGOActor->GetLevel();
        }
        else if (LEGOActor->GetLevel() != OutCommonLevel)
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("%s: Actor '%s' is in a different level and will be ignored."),
                ContextName, *LEGOActor->GetName());
            continue;
        }

        OutValidActors.AddUnique(LEGOActor);
    }

    if (OutValidActors.Num() < 2)
    {
        UE_LOG(LogLEGOActorConnections, Error,
            TEXT("%s requires at least two valid ALEGOActor instances."), ContextName);
        return false;
    }

    return true;
}

#if WITH_EDITOR

void ALEGOActor::ConnectLEGOActors(const TArray<AActor*>& InActors)
{
    TArray<ALEGOActor*> validLEGOActors;
    ULevel* commonLevel = nullptr;

    if (!ValidateLEGOActors(
        InActors,
        validLEGOActors,
        commonLevel,
        TEXT("ALEGOActor::ConnectLEGOActors")))
    {
        return;
    }

    int32 connectionCount = 0;
    FScopedTransaction transaction(FText::FromString(TEXT("Connect LEGO Actors")));

    for (int32 i = 0; i < validLEGOActors.Num(); ++i)
    {
        ALEGOActor* previousLEGOActor = validLEGOActors[i];

        if (!IsValid(previousLEGOActor))
            continue;

        for (int32 j = i + 1; j < validLEGOActors.Num(); ++j)
        {
            ALEGOActor* subsequentLEGOActor = validLEGOActors[j];
            if (!IsValid(subsequentLEGOActor))
                continue;

            if (!previousLEGOActor->IsConnectedTo(*subsequentLEGOActor))
            {
                // need this for undo
                previousLEGOActor->Modify();
                subsequentLEGOActor->Modify();

                if (previousLEGOActor->AddConnection(*subsequentLEGOActor))
                {
                    ++connectionCount;

                    UE_LOG(LogLEGOActorConnections, Log,
                        TEXT("ALEGOActor::ConnectLEGOActors Connected '%s' <-> '%s'"),
                        *previousLEGOActor->GetName(), *subsequentLEGOActor->GetName());
                }
            }
        }
    }

    UE_LOG(LogLEGOActorConnections, Log,
        TEXT("ALEGOActor::ConnectLEGOActors ConnectActors completed: %d connections created."),
        connectionCount);

}
void ALEGOActor::DisconnectLEGOActors(const TArray<AActor*>& InActors)
{
   
    TArray<ALEGOActor*> validLEGOActors;
    ULevel* commonLevel = nullptr;

    if (!ValidateLEGOActors(
        InActors,
        validLEGOActors,
        commonLevel,
        TEXT("ALEGOActor::DisconnectLEGOActors")))
    {
        return;
    }

    int32 disconnectCount = 0;
    FScopedTransaction transaction(FText::FromString(TEXT("Disconnect LEGO Actors")));

    for (int32 i = 0; i < validLEGOActors.Num(); ++i)
    {
        ALEGOActor* previousLEGOActor = validLEGOActors[i];
        if (!IsValid(previousLEGOActor))
            continue;

        for (int32 j = i + 1; j < validLEGOActors.Num(); ++j)
        {
            ALEGOActor* subsequentLEGOActor = validLEGOActors[j];
            if (!IsValid(subsequentLEGOActor))
                continue;

            if (previousLEGOActor->IsConnectedTo(*subsequentLEGOActor))
            {
                // need this for undo
                previousLEGOActor->Modify();
                subsequentLEGOActor->Modify();

                if (previousLEGOActor->RemoveConnection(*subsequentLEGOActor))
                {
                    ++disconnectCount;
                    UE_LOG(LogLEGOActorConnections, Log,
                        TEXT("ALEGOActor::DisconnectLEGOActors Removed Connection '%s' <- / -> '%s'"),
                        *previousLEGOActor->GetName(), *subsequentLEGOActor->GetName());
                }
            }
        }
    }

    UE_LOG(LogLEGOActorConnections, Log,
        TEXT("ALEGOActor::DisconnectLEGOActors DisconnectActors completed: %d connections removed."),
        disconnectCount);
}

bool ALEGOActor::AddConnection(ALEGOActor& InOtherActor)
{
    if (&InOtherActor == this)
        return false;

    if (GetLevel() != InOtherActor.GetLevel())
        return false;

    if (!ConnectedActors.Contains(&InOtherActor))
    {
        ConnectedActors.Add(&InOtherActor);
    }

    TArray<ALEGOActor*>& connectedActors = InOtherActor.ConnectedActors;
    if (!connectedActors.Contains(this))
    {
        connectedActors.Add(this);
    }

    return true;
}

bool ALEGOActor::RemoveConnection(ALEGOActor& InOtherActor)
{
    if (&InOtherActor == this)
        return false;

    if (GetLevel() != InOtherActor.GetLevel())
        return false;

    ConnectedActors.Remove(&InOtherActor);
    InOtherActor.ConnectedActors.Remove(this);
    return true;
}

bool ALEGOActor::IsConnectedTo(ALEGOActor& InOtherActor) const
{
    return ConnectedActors.Contains(&InOtherActor);
}

bool ALEGOActor::CheckLineOfSight(const ALEGOActor& InOther) const
{
    FHitResult hit;
    FVector start = GetActorLocation();
    FVector end = InOther.GetActorLocation();

    FCollisionQueryParams params;
    params.AddIgnoredActor(this);
    params.AddIgnoredActor(&InOther);

    return !GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Visibility, params);
}

FVector ALEGOActor::CalculateClosestPointOnSphere(const ALEGOActor& InOther) const
{
    FVector distance = InOther.GetActorLocation() - GetActorLocation();
    distance.Normalize();

    // is size ok here?
    return GetActorLocation() + distance * Size;
}

float ALEGOActor::CalculateForwardAngleDegrees(const ALEGOActor& InOther) const
{
    FVector forward = GetActorForwardVector();
    FVector toOther = (InOther.GetActorLocation() - GetActorLocation()).GetSafeNormal();

    float dotProduct = FVector::DotProduct(forward, toOther);
    dotProduct = FMath::Clamp(dotProduct, -1.f, 1.f);

    return FMath::RadiansToDegrees(FMath::Acos(dotProduct));
}

void ALEGOActor::RebuildDerivedData()
{
    int32 updatedConnections = 0;

    // naive approach, we also need to update counterpart
    for (ALEGOActor* otherLegoActor : ConnectedActors)
    {
        if (!IsValid(otherLegoActor))
            continue;

        FDerivedConnectionData data;
        data.bHasLineOfSight = CheckLineOfSight(*otherLegoActor);
        data.ClosestPointOnSphere = CalculateClosestPointOnSphere(*otherLegoActor);
        data.ForwardAngleDegrees = CalculateForwardAngleDegrees(*otherLegoActor);

        bool bChanged = true;

        const FDerivedConnectionData& oldData = DerivedData.FindOrAdd(otherLegoActor);
        bChanged = (oldData.bHasLineOfSight != data.bHasLineOfSight) ||
            !oldData.ClosestPointOnSphere.Equals(data.ClosestPointOnSphere, 0.01f) ||
            !FMath::IsNearlyEqual(oldData.ForwardAngleDegrees, data.ForwardAngleDegrees, 0.01f);

        if (bChanged)
        {
            DerivedData.Add(otherLegoActor, data);
            ++updatedConnections;

            UE_LOG(LogLEGOActorConnections, Log,
                TEXT("ALEGOActor::RebuildDerivedData. Updated data for connection %s -> %s: LOS=%s, ClosestPoint=%s, ForwardAngle=%.2f"),
                *GetName(),
                *otherLegoActor->GetName(),
                data.bHasLineOfSight ? TEXT("True") : TEXT("False"),
                *data.ClosestPointOnSphere.ToString(),
                data.ForwardAngleDegrees);
        }
    }

    UE_LOG(LogLEGOActorConnections, Log,
        TEXT("ALEGOActor::RebuildDerivedData. Actor '%s' rebuilt derived data. [%d] connections updated."),
        *GetName(),
        updatedConnections);
}

void ALEGOActor::PostEditMove(bool InFinished)
{
    // should take into account multiple actors due to compounded costs?
    Super::PostEditMove(InFinished);
    RebuildDerivedData();
}

void ALEGOActor::PostEditChangeProperty(FPropertyChangedEvent& InEvent)
{
    // should take into account multiple actors due to compounded costs?
    Super::PostEditChangeProperty(InEvent);
    RebuildDerivedData();
}

#endif


