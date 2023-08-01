# Filtered Hand Tracking

To improve the user experience of a hand-tracking based game, the *HandTrackingFilterComponent* is used. This component stabilizes the hand movement while tracking is low quality or lost. You can read more about this technology in [Adding Hand Tracking To First Steps](https://developer.oculus.com/blog/adding-hand-tracking-to-first-steps/) in the "Hand tracking accuracy mitigation" section.

Simply attach this component to the *MotionControllerComponent* within your Character, and your hand tracking will be more stable.
