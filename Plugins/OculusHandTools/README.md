# Oculus Hand Tools For UE4

## Oculus Quest Hand Tracking

The Oculus Quest is the first computing platform that offers native hand tracking using its inside-out cameras.  The hand tracking API offers raw hand bone rotations, hand location and orientation, confidence levels and the recognition of a few higher-level hand poses in the form of pinch strength and system poses.

To help the Oculus developer get started with raw hand bone rotations, the Oculus DevTech team has developed the Oculus Hand Tools plugin for UE4.  It enables many common hand tracking mechanics and utilities.

Many of the mechanics are implemented in blueprints in the [Content](./Content/) folder. You can find more information about them below. There are also mechanics and utilities implemented in the included C++ modules, which you can find here:

- [HandInput module](./README_HandInput.md)
- [HandPoseRecognition module](./README_HandPoseRecognition.md)
- [OculusHandTrackingFilter module](./README_HandTrackingFilter.md)
- [OculusInteractable module](./README_Interactable.md)
- [OculusThrowAssist module](./README_ThrowAssist.md)
- [OculusUtils module](./README_OculusUtils.md)

## Mechanics Implementations

The Content folder contains utilities for implementing hand tracking gameplay mechanics. You can see examples of their use in the *Oculus Hand Gameplay Showcase*.

All of these mechanics are integrated into the *HandsCharacterHandState* component. This component is instantiated in the construction script for *HandsCharacterBase*, setting up references to all of the relevant components on the actor.

The simplest way to integrate these mechanics into your own project is to create a new blueprint class with *HandsCharacterBase* as its parent. You can see an example of this in the showcase's *HandsCharacter* class.

### Throwing

In the *HandsCharacterHandState* class (in the "Throwing" section of its event graph), you will find the implementation of throwing. In its *Throw with Assist* function, it uses the *Get Throw Vector* method of the *ThrowingComponent*. That component tracks the position of the hand over a short period of time in order to infer an accurate and predictable velocity for the thrown object.

### Punching

Punching is a surprisingly simple and fun mechanic using hand tracking. In the "Fist / punching" section of the *HandsCharacterHandState* event graph, you can see that a collision sphere on the hand is enabled only while the hand is gripping (in a fist pose) and no object is currently grabbed.

An example of an "punchable" object is in the Hand Gameplay Showcase in the *TetherBallBP* class.

### Teleportation

Utilizing the *InteractableSelector* actors from the *OculusInteractable* module (see above), a simple teleportation system can be implemented. This is done fairly simply: In the "Grabbing" section of the *HandsCharacterHandState* event graph, when a grab fails, it falls back to checking for a selected teleporter and teleporting the player to it. The teleporter selection itself is in the "Selection" section of the same graph.

In order to limit selection to teleporters, the *TeleportSelector* actor is used rather than the standard *InteractableSelector*.

Note that this requires the following in your project's *DefaultEngine.ini*:

```ini
[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Interactable")
```

### Pushing Buttons

The *PushButtonBaseBP* class implements logic for a button that can be pushed with the player's pointer finger. The button moves with the finger and makes clicking sounds when pushed in all the way. You can set up the response to the button press using the *OnButtonPress* event.

You may notice that in the Box component, the collision channel is set to *FingerTip*. Looking at the *HandsCharacterBase*, you'll see the *RightFingerTip* component uses this same collision channel.

Note that buttons require the following in your project's *DefaultEngine.ini*:

```ini
[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel2,DefaultResponse=ECR_Overlap,bTraceType=False,bStaticObject=False,Name="FingerTip")
```

### Two-Handed Aiming

In making *First Steps with Hands*, we discovered that the two handed rifle was very stable and fulfilling to aim with. Check out *InteractableTwoHandedArtefact* as an example implementation of this sort of aiming device that can be implemented using the grabbing system.

### Example Hands for Tutorials

<img width="256" src="./Media/tutorialhand.png" />

The *TutorialHand* actor can be used to display hand poses to the player. You can set the pose by setting the [*Pose String*](./README_HandPoseRecognition.md#pose-strings) property.

<img width="256" src="./Media/tutorialhand_details.png" />

## Future Improvements

We hope that this showcase will inspire your own work with hand tracking. The team is eager to see what you will end up doing with it and how you will improve it.  We are still working on ways for you to reach out back to us, so that you can share your work with the rest of the Oculus community.
