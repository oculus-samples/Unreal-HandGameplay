# The OculusThrowAssist Module

This module contains the implementation of the throwing system. Specifically, it contains *ThrowingComponent*, an ActorComponent that tracks a hand and estimates throwing velocities. 

To integrate *ThrowingComponent* into your Character or Hand implementation, add an instance of it for each hand to your Actor. Call the *Initialize* method with the component that controls the transform of the hand (usually a *MotionControllerComponent*). To track that transform, call *Update* on every Tick. Finally, call *GetThrowVector* to estimate the velocity of an object thrown from the tracked hand.
