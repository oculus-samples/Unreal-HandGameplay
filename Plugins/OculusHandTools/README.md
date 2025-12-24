# Meta Quest Hand Tools For UE4

## Meta Quest Hand Tracking

The Meta Quest (Oculus) is the first platform to offer native hand tracking using inside-out cameras. Its hand tracking API provides raw hand bone rotations, hand location and orientation, confidence levels, and recognition of some higher-level hand poses like pinch strength and system poses.

To help developers start with raw hand bone rotations, the Meta Quest DevTech team created the Meta Quest Hand Tools plugin for UE4. It supports many common hand tracking mechanics and utilities.

Most mechanics are implemented as blueprints in the [Content](./Content/) folder. More details follow below. Additional mechanics and utilities are available in these C++ modules:

- [HandInput module](./README_HandInput.md)
- [HandPoseRecognition module](./README_HandPoseRecognition.md)
- [OculusHandTrackingFilter module](./README_HandTrackingFilter.md)
- [OculusInteractable module](./README_Interactable.md)
- [OculusThrowAssist module](./README_ThrowAssist.md)
- [OculusUtils module](./README_OculusUtils.md)

## Mechanics Implementations

The Content folder contains utilities for hand tracking gameplay mechanics. Examples appear in the *Hand Gameplay Showcase*.

All mechanics integrate into the *HandsCharacterHandState* component. This component is created in the construction script of *HandsCharacterBase*, which sets up references to all relevant actor components.

To add these mechanics to your project, create a new blueprint class with *HandsCharacterBase* as its parent. The showcase’s *HandsCharacter* class demonstrates this.

### Throwing

The *HandsCharacterHandState* class implements throwing in the "Throwing" section of its event graph. Its *Throw with Assist* function calls the *Get Throw Vector* method of the *ThrowingComponent*. This component tracks hand position over a short time to calculate an accurate, predictable velocity for thrown objects.

### Punching

Punching is a simple, fun mechanic using hand tracking. In the "Fist / punching" section of the *HandsCharacterHandState* event graph, a collision sphere on the hand activates only when the hand forms a fist and no object is grabbed.

The *TetherBallBP* class in the Hand Gameplay Showcase provides an example of a "punchable" object.

### Teleportation

You can implement a simple teleportation system using *InteractableSelector* actors from the *OculusInteractable* module. In the "Grabbing" section of the *HandsCharacterHandState* event graph, if a grab fails, it checks for a selected teleporter and teleports the player there. Teleporter selection happens in the "Selection" section of the same graph.

To restrict selection to teleporters, use the *TeleportSelector* actor instead of the standard *InteractableSelector*.

Add this to your project's *DefaultEngine.ini*:

```ini
[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Interactable")
```

### Pushing Buttons

The *PushButtonBaseBP* class handles buttons pushed by the player’s pointer finger. The button moves with the finger and plays clicking sounds when fully pressed. Use the *OnButtonPress* event to define the button’s response.

The button’s Box component uses the *FingerTip* collision channel. The *RightFingerTip* component in *HandsCharacterBase* uses the same channel.

Add this to your project's *DefaultEngine.ini*:

```ini
[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel2,DefaultResponse=ECR_Overlap,bTraceType=False,bStaticObject=False,Name="FingerTip")
```

### Two-Handed Aiming

During *First Steps with Hands*, we found the two-handed rifle very stable and satisfying to aim. The *InteractableTwoHandedArtefact* class shows how to implement this aiming style using the grabbing system.

### Example Hands for Tutorials

<img width="256" src="./Media/tutorialhand.png" />

Use the *TutorialHand* actor to display hand poses to players. Set the pose via the [*Pose String*](./README_HandPoseRecognition.md#pose-strings) property.

<img width="256" src="./Media/tutorialhand_details.png" />

## Future Improvements

We hope this showcase inspires your hand tracking projects. The team looks forward to seeing your work and improvements. We are developing ways for you to share your projects with the Meta Quest community.
