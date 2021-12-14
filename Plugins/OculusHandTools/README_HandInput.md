# Hand Input

## Grabbing (with Pose Override)

The core implementation of grabbing is in the *CameraHandInput* component. This component implements methods *IsGripping* and *IsPinching* to detect a variety of grab poses.

The rest of the implementation can be seen in *HandsCharacterHandState* in the "Grabbing", "Final attachment" and "Pose" sections of its event graph.

While an object is grabbed, the hand pose must be overridden in order for the hand to stay wrapped around the object in a sensible way. To accomplish this, you must provide the component with a "Bone Map" so that it can apply poses to the hand mesh.

<img width="512" src="./Media/bonemap.png" />

For an actor to be grabbable, it must be a child of the *Interactable* actor class. Within that class, you can set its grab pose properties with hand pose strings. For an example of this, look at the *InteractableBrick* class in the Hand Gameplay Showcase.

## Finger Stabilization

In addition, the hand skeleton is also stabilized and smoothed by the *CameraHandInput* component. You can tweak its settings (or disable the filtering completely) through its properties:

<img width="512" src="./Media/camerahandinput_filtering.png" />
