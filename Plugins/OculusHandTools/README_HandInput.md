# Hand Input

## Grabbing (with Pose Override)

The *CameraHandInput* component handles grabbing. It uses the *IsGripping* and *IsPinching* methods to detect various grab poses.

Additional logic appears in *HandsCharacterHandState* under the "Grabbing," "Final attachment," and "Pose" sections of its event graph.

To keep the hand wrapped naturally around a grabbed object, you must override the hand pose. Provide the component with a "Bone Map" to apply poses to the hand mesh.

<img width="512" src="./Media/bonemap.png" />

An actor must inherit from the *Interactable* class to be grabbable. Set grab pose properties using hand pose strings within this class. For an example, see the *InteractableBrick* class in the Hand Gameplay Showcase.

## Finger Stabilization

The *CameraHandInput* component also stabilizes and smooths the hand skeleton. You can adjust or disable this filtering through its properties:

<img width="512" src="./Media/camerahandinput_filtering.png" />
