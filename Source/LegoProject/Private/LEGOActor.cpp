


#include "LEGOActor.h"

#if WITH_EDITOR
#include "ScopedTransaction.h"
#endif

DEFINE_LOG_CATEGORY(LogLEGOActorConnections);

ALEGOActor::ALEGOActor()
{
}


#if WITH_EDITOR
bool ALEGOActor::ValidateLEGOActors(const TArray<AActor*>& InActors, TArray<ALEGOActor*>& OutValidActors, ULevel* OutCommonLevel, const TCHAR* InContextName)
{
    OutValidActors.Reset();
    OutCommonLevel = nullptr;

    if (InActors.Num() < 2)
    {
        UE_LOG(LogLEGOActorConnections, Error,
            TEXT("%s requires at least two actor instances."), InContextName);
        return false;
    }

    for (AActor* actor : InActors)
    {
        if (!IsValid(actor))
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("%s: Null or invalid actor passed."), InContextName);
            continue;
        }

        ALEGOActor* LEGOActor = Cast<ALEGOActor>(actor);
        if (!IsValid(LEGOActor))
        {
            UE_LOG(LogLEGOActorConnections, Warning,
                TEXT("%s: Actor '%s' is not an ALEGOActor and will be ignored."),
                InContextName, *actor->GetName());
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
                InContextName, *LEGOActor->GetName());
            continue;
        }

        OutValidActors.AddUnique(LEGOActor);
    }

    if (OutValidActors.Num() < 2)
    {
        UE_LOG(LogLEGOActorConnections, Error,
            TEXT("%s requires at least two valid ALEGOActor instances."), InContextName);
        return false;
    }

    return true;
}

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

    if (!IsConnectedTo(InOtherActor))
    {
        ConnectedActors.Add(&InOtherActor);
    }

    if (!InOtherActor.IsConnectedTo(*this))
    {
        InOtherActor.AddConnection(*this);
    }

    RebuildDerivedDataForConnection(InOtherActor);
    InOtherActor.RebuildDerivedDataForConnection(*this);

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

    InOtherActor.RemoveDerivedDataForConnection(*this);
    RemoveDerivedDataForConnection(InOtherActor);

    return true;
}

bool ALEGOActor::IsConnectedTo(const ALEGOActor& InOtherActor) const
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

void ALEGOActor::PostEditMove(bool InFinished)
{
    Super::PostEditMove(InFinished);

    if (InFinished)
    {
        RebuildDerivedDataForAllConnections();
    }
}

void ALEGOActor::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent)
{
    Super::PostEditChangeProperty(InPropertyChangedEvent);
    
    if (InPropertyChangedEvent.Property == nullptr)
        return;

    if (InPropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
        return;

    RebuildDerivedDataForAllConnections();
}

void ALEGOActor::PostDuplicate(bool InDuplicateForPIE)
{
    Super::PostDuplicate(InDuplicateForPIE);
    
    //TODO FA: Could potentially just rebuild data and add connections
    DerivedData.Reset();
    ConnectedActors.Reset();

    UE_LOG(LogLEGOActorConnections, Log,
            TEXT("ALEGOActor::PostDuplicate ALEGOActor '%s' duplicated: connections cleaned."),
            *GetName());
}

void ALEGOActor::RemoveDerivedDataForConnection(const ALEGOActor& InOther)
{
    DerivedData.Remove(&InOther);
}

void ALEGOActor::RebuildDerivedDataForConnection(ALEGOActor& InOther)
{
    if (&InOther == this)
        return;

    if (!IsConnectedTo(InOther) || !InOther.IsConnectedTo(*this))
    {
        UE_LOG(LogLEGOActorConnections, Warning, TEXT("ALEGOActor::RebuildDerivedDataForConnection Tried to rebuild derived data for actors that are not connected to each other. You might see this during duplication."));
        return;
    }

    const bool bHasLOS = CheckLineOfSight(InOther);
    const FVector closestThisToOther = CalculateClosestPointOnSphere(InOther);
    const float forwardAngleThisToOther = CalculateForwardAngleDegrees(InOther);

    FDerivedConnectionData& connectionData = DerivedData.FindOrAdd(&InOther);
   
    bool hasDataChanged =
        connectionData.bHasLineOfSight != bHasLOS ||
        !connectionData.ClosestPointOnSphere.Equals(closestThisToOther, 0.01f) ||
        !FMath::IsNearlyEqual(connectionData.ForwardAngleDegrees, forwardAngleThisToOther, 0.01f);

    if (hasDataChanged)
    {
        Modify();

        connectionData.bHasLineOfSight = bHasLOS;
        connectionData.ClosestPointOnSphere = closestThisToOther;
        connectionData.ForwardAngleDegrees = forwardAngleThisToOther;

        UE_LOG(LogLEGOActorConnections, Log, TEXT("ALEGOActor::RebuildDerivedDataForConnection DerivedData: %s -> %s; LOS=%d Angle=%.2f Point=%s"),
            *GetName(), *InOther.GetName(),
            bHasLOS,
            forwardAngleThisToOther,
            *closestThisToOther.ToString());
    }
}

void ALEGOActor::RebuildDerivedDataForAllConnections()
{
    if (bIsRebuildingDerivedData)
        return;

    bIsRebuildingDerivedData = true;

    int32 updatedCount = 0;

    for (ALEGOActor* otherLEGOActor : ConnectedActors)
    {
        if (!IsValid(otherLEGOActor))
            continue;

        RebuildDerivedDataForConnection(*otherLEGOActor);
        otherLEGOActor->RebuildDerivedDataForConnection(*this);

        ++updatedCount;
    }

    bIsRebuildingDerivedData = false;

    UE_LOG(LogLEGOActorConnections, Log,
        TEXT("ALEGOActor::RebuildDerivedDataForAllConnections ALEGOActor '%s' rebuilt derived data for %d connections."),
        *GetName(),
        updatedCount);
}

#endif


