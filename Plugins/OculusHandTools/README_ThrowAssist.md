# The OculusThrowAssist Module

This module implements the throwing system. It includes *ThrowingComponent*, an ActorComponent that tracks a hand and estimates throwing velocities.

To use *ThrowingComponent* in your Character or Hand, add one instance per hand to your Actor. Call *Initialize* with the component controlling the hand’s transform (usually a *MotionControllerComponent*). Call *Update* every Tick to track the transform. Finally, call *GetThrowVector* to estimate the velocity of an object thrown from the tracked hand.
