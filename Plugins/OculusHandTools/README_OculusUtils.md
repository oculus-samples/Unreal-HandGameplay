# The OculusUtils Module

## Continuous Overlap

Looking at the *HandsCharacterBase* and hovering over *RightFingerTip*, you may notice that it is not a standard *SphereComponent* as the fist spheres are, but rather a *ContinuousOverlapSphereComponent*.

<img width="256" src="./Media/continuousoverlapsphere.png" />

This is a specialized component that prevents the fingertip from moving through the button without triggering it. The current implementation of *SphereComponent* does not support continuous **overlap** detection (only continuous **collision** detection, which can be enabled with "Use CCD").

## Tick Until

<img width="256" src="./Media/tickuntil.png" />

This is a useful blueprint node that will run from the *Completed* pin once per tick until *Break* is hit.
