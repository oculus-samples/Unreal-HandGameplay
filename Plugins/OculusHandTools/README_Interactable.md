# The OculusInteractable Module

The *HandPoseShowcase* started with the idea that we would bring to UE4 one proven hand gameplay mechanic to life.  One of our internal Oculus teams was working on a very cool demo for the Unity engine called Tiny Castles and we selected their *force grab/throw* to showcase in this project.  To achieve this, we have implemented our own version of a far field selector, [AInteractableSelector](../Source/OculusInteractable/Public/InteractableSelector.h), that is meant to be attached to the VR character's motion controller.  Other uses are possible, such as attaching it to the HMD.

The interactable selector is meant to select game actors that are subclassed from [AInteractable](../Source/OculusInteractable/Public/Interactable.h).

## Interactable Actors

AInteractable actors have two main events that are called by the selector: *BeginSelection* and *EndSelection*.  The selector can only select one object at a time, and this unique object is notified of the beginning and end of that selection through those events.

A typical reaction to selection is to highlight the selected actor.

The interactable interface also has three user-defined events that can be implemented in a game-specific way.  They are not used by the plugin code and their meaning is left undefined.  In the *HandPoseShowcase*, the events *Interaction1*, *Interaction2* and *Interaction3* are, respectively, used for *BeginGrab* (the user started grabbing the object), *EndGrab* (the object has reached the user's hand), and *Drop* (the user is letting go of the object).

To be selectable, the AInteractable actor needs to have at least one mesh that generates overlap events with the first game trace channel.  In our implementation (see [AInteractableSelector](../Source/OculusInteractable/Private/InteractableSelector.cpp)) you can find the following definition at the top of the file

    ECollisionChannel InteractableTraceChannel = ECollisionChannel::ECC_GameTraceChannel1;

This corresponds to a trace channel that is added to the project settings in the engine collision section as shown below.

<img width="400" src="./Media/trace_channel.png" alt="Interactable trace channel." />

If your project already has a use for that channel, you will simply have to associate the InteractableTraceChannel to a different collision channel.

## Interactable Selector

The far field selector is meant to be subclassed and customized, but is operational as-is with many configuration parameters.

<img width="400" src="./Media/selector.png" alt="Default far field selector." />

Typically the selector will start deactivated.  In the case of the *HandPoseShowcase*, the selector is activated when the user makes an open palm hand pose.  The selection ray starts at the specified *Raycast Offset* relative to the selector actor, and traces in the environment for the specified *Raycast Distance*.

Two angles determine the behavior of the selector: the *Raycast Angle* and the *Raycast Stickiness Angle*.  The first one defines the shape of the cone of selection: only objects in that cone are subject to selection.  The second angle is meant to provide stickiness to the current selection by allowing it to stay selected beyond the stricter cone of selection.

The visuals of the selector are defined by an *Aiming Actor* and by a particle beam effect.

If an existing *Aiming Actor* is not specified, the *Aiming Actor Class* is used to spawn one.  A few settings control the aiming actor: you can have it aligned with the hit normal of the surface, you can control how fast it changes it's orientation with *Aiming Actor Rotation Rate*.

The *Dampening Factor* affects the aiming location.  With no dampening, the aiming is as jittery as the user's hand.
