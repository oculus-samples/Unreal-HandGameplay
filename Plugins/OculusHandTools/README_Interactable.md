# The OculusInteractable Module

The *HandPoseShowcase* brings a proven hand gameplay mechanic to UE4. One of our internal Meta Quest teams created a demo for Unity called Tiny Castles. We chose their *force grab/throw* mechanic to showcase in this project. To implement this, we created our own far field selector, [AInteractableSelector](./Source/OculusInteractable/Public/InteractableSelector.h), designed to attach to the VR character's motion controller. It can also attach to the HMD or other components.

The interactable selector selects game actors subclassed from [AInteractable](./Source/OculusInteractable/Public/Interactable.h).

## Interactable Actors

AInteractable actors have two main events triggered by the selector: *BeginSelection* and *EndSelection*. The selector selects only one object at a time. This object receives notifications when selection starts and ends.

Typically, selection highlights the chosen actor.

The interactable interface also defines three user events for game-specific use. The plugin does not use these events or define their meaning. In *HandPoseShowcase*, *Interaction1*, *Interaction2*, and *Interaction3* correspond to *BeginGrab* (user starts grabbing), *EndGrab* (object reaches the user's hand), and *Drop* (user releases the object), respectively.

To be selectable, an AInteractable actor must have at least one mesh that generates overlap events with the first game trace channel. In our implementation (see [AInteractableSelector](./Source/OculusInteractable/Private/InteractableSelector.cpp)), the following is defined at the top of the file:

```cpp
ECollisionChannel InteractableTraceChannel = ECollisionChannel::ECC_GameTraceChannel1;
```

This trace channel is added in the project settings under the engine collision section, as shown below.

<img width="400" src="./Media/trace_channel.png" alt="Interactable trace channel." />

If your project uses this channel, assign `InteractableTraceChannel` to a different collision channel.

## Interactable Selector

The far field selector works out of the box but is meant to be subclassed and customized. It has many configuration parameters.

<img width="400" src="./Media/selector.png" alt="Default far field selector." />

The selector usually starts deactivated. In *HandPoseShowcase*, it activates when the user makes an open palm hand pose. The selection ray begins at the specified *Raycast Offset* relative to the selector actor and traces forward for the specified *Raycast Distance*.

Two angles control selector behavior: *Raycast Angle* and *Raycast Stickiness Angle*. The *Raycast Angle* defines the selection cone—only objects inside this cone can be selected. The *Raycast Stickiness Angle* allows the current selection to remain active even if it moves outside the stricter cone.

The selector’s visuals include an *Aiming Actor* and a particle beam effect.

If no *Aiming Actor* exists, the *Aiming Actor Class* spawns one. You can align the aiming actor with the surface hit normal and control its rotation speed using *Aiming Actor Rotation Rate*.

The *Dampening Factor* affects aiming stability. Without dampening, aiming jitters with the user's hand movements.
