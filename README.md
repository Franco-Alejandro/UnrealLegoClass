# LEGO Actor System for Unreal Engine

Small excersice to test knowledge on building tools for bidirectional connections on actors and data serialization.

## Project Structure

### Core Classes
- **Main Class**: `LEGOActor`
  - Where is it? `UnrealLegoClass/Source/LegoProject/`

### Editor Subsystem
- **Serialization Subsystem**: `ULEGOActorSerializationSubsystem`
  - Where is it? `UnrealLegoClass/Source/LegoProjectEditor/`

## Key Features

### 1. Actor Connection System
- **ConnectLEGOActors**: processes selected actors, validates them, and establishes bidirectional connections as specified in the exercise requirements
- **DisconnectLEGOActors**: removes connections between selected LEGO actors

####Edge cases
The `LEGOActor` class overrides these editor lifecycle methods to avoid issues during level editing:

```cpp
virtual void PostEditMove(bool InFinished) override;
virtual void PostEditChangeProperty(FPropertyChangedEvent& InEvent) override;
virtual void PostDuplicate(bool InDuplicateForPIE) override;
```

### 2. JSON serialization through subsystem
`ULEGOActorSerializationSubsystem` provides stateless serialization utilities, the idea is that it could be expanded, and that is why it is a subsystem:

#### Methods:
- **SerializeCurrentLevel(FString InPath)**: exports all LEGO actors in the current level to a JSON file
- **DeserializeIntoCurrentLevel(FString InPath)**: reads a JSON file and spawns corresponding LEGO actors (supports inherited classes)

## Usage Workflow

1. Place LEGO Actors in your level (either base `LEGOActor` or derived classes)
2. Select multiple actors in the editor viewport
3. Execute connection commands via editor utilities
4. Serialize/Deserialize using the subsystem methods for level persistence

## Requirements
- Unreal Engine 5.7 or more, only tested in development build

## Testing
 Just open a frontend session and run the lego actor test for serialization
 <img width="233" height="62" alt="image" src="https://github.com/user-attachments/assets/643cff3a-f95d-4441-90ce-b3577cca3705" />
 <img width="943" height="307" alt="image" src="https://github.com/user-attachments/assets/e0326d0f-b46f-453a-ae5b-b5a8a367472d" />
