# The OculusUtils Module

## Continuous Overlap

The *RightFingerTip* in *HandsCharacterBase* is a *ContinuousOverlapSphereComponent*, unlike the fist spheres, which use a standard *SphereComponent*.

<img width="256" src="./Media/continuousoverlapsphere.png" />

This component prevents the fingertip from passing through buttons without triggering them. The standard *SphereComponent* only supports continuous **collision** detection (enabled with "Use CCD"), not continuous **overlap** detection.

## Tick Until

<img width="256" src="./Media/tickuntil.png" />

The *Tick Until* blueprint node runs once per tick from the *Completed* pin until it hits *Break*.
