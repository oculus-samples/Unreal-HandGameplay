# The OculusHandPoseRecognition Module

This module contains two [USceneComponent](https://docs.unrealengine.com/en-US/API/Runtime/Engine/Components/USceneComponent/index.html) subclasses that can be configured to recognize specific hand poses &ndash; described as a set of relative hand bone angles &ndash; and hand gestures &ndash; described as sequences of hand poses.  It also provides a library of useful blueprint utility nodes.

## 1. Hand Pose Recognizer Component

The [UHandPoseRecognizer](../Source/OculusHandPoseRecognition/Public/HandPoseRecognizer.h) class is the core of our hand recognition system.  It regularly polls the [Oculus Hands API](../../../../../Engine/Plugins/Runtime/Oculus/OculusVR/Source/OculusInput/Public/OculusInputFunctionLibrary.h) to record hand bone information when it can be reliably assessed.  Wrist bone information is acquired from the motion controller component that the pose recognizer is attached to on the player's VR character.

<img width="256" src="./Media/recognizer_attachment.png" alt="Recognizer attached to controller." />

There can be multiple recognizers per hand, each with its own set of hand poses to recognize.  The main reason to group hand poses into one recognizer is when they are mutually exclusive from one another.  In our showcase, we provide a recognizer for American Sign Language numbers and another for letters.  In actual ASL, there is a way to distinguish the letter 'O' and the number '0', but in our example we have limited ourselves to static hand poses and having them recognized in separate recognizers allows us to recognize both in parallel.

### Pose Strings

What each recognizer can recognize is described as a set of bone angles.  Here's the thumbs-up pose description for the left hand.

    L T0-52-18+51 T1+13-8+30 T2+7-9-10 T3-10+21+8
      I1+6-72+1 I2-3-108+1 I3+1-55-3
      M1+1-77-8 M2-1-99+1 M3-6-51-8
      R1-4-85-10 R2-5-100-1 R3-4-50-1
      P0+15-6-25 P1+8-88+4 P2-8-94-7 P3-4-54+2
      W+81+0+0

The description starts with 'L' or 'R' for the left and right hands.  It is followed by an ordered list of hand bones starting with letters 'T' (Thumb), 'I' (Index), 'M' (Middle), 'R' (Ring), 'P' (Pinky) and finally 'W' (Wrist). All bones, except the wrist, are followed by a bone index starting at 0 for the thumb and pinky, and 1 for the other three fingers.

Angles are described by three signed numbers for pitch, yaw and roll in that order.  All angles are expressed in degrees with whole numbers.

You can easily generate new pose strings by using the logger described in [Using a Hand Pose Recognizer](#using-a-hand-pose-recognizer).

### Computing Distance (a.k.a. Error) To Hand Pose

The way a hand is matched to a reference pose starts with the computation of the sum of the squares of all angular differences.  The closer you get to zero the better your match is.  What constitutes a good match is left to your own assessment and we will cover that in the next section on configuration.

As we have experimented with the hand pose recognition, we quickly realized that there are situations where it was useful to ignore some bones.  For instance, when we ask people to perform a "gun" hand pose, everybody uses the index to represent the gun barrel, but some people use both the index and the middle fingers.  To have only one pose match both gun poses, we can simply ignore the middle finger.  This is as simple as removing the bones that you want to ignore from the reference hand pose.

Sometimes, especially for the wrist, what you want is to ignore only the pitch, yaw and/or roll of a bone.  An example of that is the thumbs-up pose shown above.  You do not need to distinguish between a thumbs-up that is facing forward or sideways, you only need to know if the thumb is facing up or down.  You can ignore the pitch, yaw or roll by using +0.  To recognize an angle that is near zero, use +1 or -1.

In addition to that, we have realized that for some bones are more (or less) important than others for some poses.  For that purpose, you can add between the bone identifier and the pitch/yaw/roll values a weight value.  Here's a full example that combines weights and ignores specific fingers.

    L T0*3-61+1+31 T1*3+12-8+30 T2*3+6-13-10 T3*3-10+21+8
      I1+16-22-1 I2+1-2-3 I3+3+2-2
      R1-2-77-12 R2-5-100-1 R3-4-57-1
      P0+15-6-25 P1+8-90-3 P2-5-76-8 P3-4+1+3

This is the left hand "gun" pose that differs from the "gun shot" pose only because the thumb is lifted away from the index.  For that reason, the position of the thumb is very important and here we have increased its weight by a factor 3.  As discussed before, the middle finger is ignored so that we can recognize both the single and double-barrel poses with only one pattern.  The wrist is also ignored, which means that it doesn't matter which way you pointing your hand.

### Error vs Confidence Level

The use of weights and the possibility of ignoring angles or whole fingers has an important effect on the error range.  In addition to that, you may be more lenient with the recognition of some poses than others.

To help normalize this, we have added the concept of confidence level. Basically, for each pose you will define the maximum error value at which you still consider that it is matching with 100% certainty.

As you are testing the error returned for a specific pose, you may decide that you have full confidence for any error value below 2000.  Using that error for your maximum confidence (a.k.a. error at max confidence or EAMC), any value below 2000 will correspond to a confidence level of 1.0. An error of 4000 will return a confidence level of 0.5, and so on.

| Error | Confidence (EAMC of 2000) |
| ----- | ------------------------- |
| 500   | 1.00                      |
| 2000  | 1.00                      |
| 3000  | 0.66                      |
| 4000  | 0.50                      |
| 6000  | 0.33                      |
| 8000  | 0.25                      |

While confidence levels are much easier to handle than raw errors, the latter is still needed during development to assess the EAMC.

### Configuring a Hand Pose Recognizer

In the details section of a hand pose recognizer, you will find the *Hand Pose Recognition* section.

<img width="256" src="./Media/recognizer_config.png" alt="Recognizer configuration." />

The *side* value must be set to recognize either the left or right hand. You can disable a recognizer by setting the side to none.

The recognition interval is meant throttle the execution of the recognition code.  The default value of 0s makes the recognizer perform its duties every tick.

The confidence floor is the minimum confidence level required for a pose to be recognized.  A default confidence floor can be defined at the recognizer level, and it can be customized for each individual pose.

The damping factor indicates how slow we integrate hand bone updates per recognition interval.  By default we use the latest values fully every tick.  A value of 0.2 indicates that we merge 80% of the latest value with the current state.

Finally, and maybe most importantly, you can configure an array of [poses](#pose-strings). There are no preset limits to the number of poses one recognizer can handle.

Each pose has a name.  That name is not required to be unique even within the same recognizer (two poses can still be distinguised by the recognized pose index, if needed).  Then you have the custom encoded pose, the custom confidence floor and the error at max confidence that were discussed in earlier sections.

### Using a Hand Pose Recognizer

The first blueprint node that you need to know about is the one that outputs the current hand pose as an encoded string to the output log.  In the following image you can see an example where recognizers for both hands have been wired to input events to a *Log Encoded Hand Pose* node.

<img width="256" src="./Media/log_encoded_pose.png" alt="Log hand pose." />

In our case, the input events are the CTRL-L and CTRL-R keyboard events. A typical workflow is to generate a few hand poses, output them to the UE4 log window, and transfer those strings to a hand pose recognizer either as is or modified as described previously by removing bones, deactivating specific angles or adding weights.

The hand pose recognizer offers a blueprint node to retrieve it's current state simply called *Get Recognized Hand Pose*.

<img width="512" src="./Media/get_recognized_hand_pose.png" alt="Get recognized hand pose." />

The image above is taken from the Hand Pose Showcase.  The part of the VRCharacter blueprint shown here forwards the recognition state of the recognizer of your choice to be displayed on the one of the projector slides.

When a hand pose is recognized, that is when there's at least one pose that is below the confidence floor for that pose, you will be able to receive its index, its name, how long that pose has been held (in seconds) and both the raw error and the confidence level.

## 2. Hand Gesture Recognizer Component

The class that handles gesture recognition is [UHandGestureRecognizer](../Source/OculusHandPoseRecognition/Public/HandGestureRecognizer.h).

A gesture is defined as a sequence of hand poses.  Our implementation limits gesture recognition from the poses of exactly one hand pose recognizer, as this is likely to be the most common case.  For that reason you only need to attach the gesture recognizer to the pose recognizer that it relies on.

<img width="256" src="./Media/gesture_recognizer.png" alt="Gesture recognizer." />

In this screenshot, the *LeftFlickSwipeGestureRecognizer* is attached to a component called *LeftFlickSwipePoseRecognizer*.  This left flick gesture is meant to have the projector move to the previous slide in the Hand Pose Showcase.  Another couple of recognizers are attached to the right hand to perform the equivalent action for going to the next slide.

### Defining a Gesture as a Sequence of Poses

For our Hand Pose Showcase, we have found an interesting sci-fi movie gesture where the actor changes channel by pointing his right index and middle fingers towards the screen and then flick them left.

We tried a few other gestures for changing slides, and we found that it was one of the easiest to learn and execute.  Let's first look at the static hand pose recognizer.

<img width="256" src="./Media/flick_pose.png" alt="Flick pose recognition." />

We have here two poses: the one where we point at the screen and the one where we flick the hand.  Now let's look at the gesture recognizer.

<img width="256" src="./Media/flick_gesture.png" alt="Flick gesture recognition." />

The recognition interval value is there to have a way to throttle the gesture recognition system.  By default it is executed every game tick.  The skipped frames value is a complementary way to enforce delays during gesture recognition.  It was added to experiment with precise throwing but ended up not being required.  It is likely going to be removed in a future version.

Here we only recognize one gesture, named "Flick", which is the transition from "Point" to "Flick".  A gesture name doesn't need to be unique.

The *Max Transition Time* value is very important.  It indicates how much time (in seconds) that we can tolerate doing something outside of the hand pose sequence.  This includes time during which the pose recognizer is not recognizing a hand pose at all.  In our example, it is configured to tolerate at most 1/5th of a second between the end of "Point" and the beginning of "Flick".

There are cases where you need to have the user hold a pose a minimum amount of time before that step is considered complete.  Imagine that you want the user to hold the "Point" pose for a short moment before flicking, then you could customize the gesture in a way similar to the following.

    Point/200, Flick

The user would then need to hold the "Point" pose for 200 milliseconds before it is considered achieved, and only then would a "Flick" trigger the recognition of the full gesture.

The *Is Looping* flag is used to recognize gestures that loop.  Waving your hand from left to right is an example of this.

### Using a Hand Gesture Recognizer

The gesture recognizer was built first and foremost to support the *force grab* and *force throw* gestures.  Let's look at part of the grabbing code in VRCharacter.

<img width="512" src="./Media/force_grab_gesture.png" alt="Grab gesture recognition." />

This node returns whether or not a gesture was recognized.  When it is the case, you can access the gesture index, it's name (which may not be unique) and the gesture direction.

The gesture direction is computed as vector from some average location at the end of the first pose and another average location at the beginning of the last pose.

In addition to this, we have experimented with a few gesture duration measures.  The outer duration starts at the beginning of the first pose to the end of the last pose.  The inner duration starts towards the end of the first pose and the beginning of the last pose.

The *Behavior* argument allows you to control how to reset the state of the recognizer after a gesture has been recognized.  By default, only the recognized gesture is reset.  In some situations, you may want to reset the state of all gestures.

<img width="512" src="./Media/gesture_state.png" alt="Gesture progress." />

The Hand Pose Showcase also illustrates how one can detect that a gesture is in progress.  In this case, we highlight the hand of the player when it is ready to flick to the previous or next projector slide.  In the following image the "Flick" gesture is queried to see if it is currently in progress.

## 3. Hand Recognition Blueprint Library

<img width="512" src="./Media/wait_nodes.png" alt="Pose and gesture wait nodes." />

### Waiting for hand pose.

UE4 has support for latent blueprint nodes that can hold the execution flow until some condition is met.

The *Wait for Hand Pose* node will wait for the specified UHandPoseRecognizer instance to recognize a hand pose.  If you want the hand pose to have been held a minimum amount of time, specify that duration in *Min Pose Duration*.  If you specify a non-negative *Time to Wait*, the node will exit through the *Time Out* execution pin if the node has not recognized a hand pose for that number of seconds. 

When a pose is seen, the execution resumes on the *Pose Seen* node exit and the recognized pose index and name will be available on the corresponding pins.

### Waiting for gesture.

The *Wait for Hand Gesture* is another latent node that waits for a gesture to be recognized.  You can specify how long, in seconds, that your are willing to wait with the *Time to Wait* parameter.  The *Behavior* argument is the same that you specify in the GetRecognizedHandGesture node discussed earlier.

When a gesture is recognized, execution resumes on the *Gesture Seen* exec pin and the gesture index, name, direction and durations are available on the corresponding pins.

If you have set a *Time to Wait* the node is allowed to exit by the *Time Out* exec pin.

You can loop back into the wait node as shown in the screenshot to wait again. 

### Recording a pose range.

<img width="512" src="./Media/pose_range_recording.png" alt="Pose range recording." />

The *Record Hand Pose* blueprint is used to record the range of hand bone angles.  In the screenshot above, taken from the *HandPoseShowcase*,  you can start/stop recording using a keyboard key.  When stopped, the node produces a report to the output log like the following, where I recorded the [royal wave](https://www.youtube.com/watch?v=n5pkDB7zEeo).

    Hand Pose Range Recorded #0
      Thumb_0 pitch  11.58 [ -55.70 ..  -44.12]  yaw  13.78 [ -26.18 ..  -12.40]  roll  16.16 [ +44.29 ..  +60.45]
      Thumb_1 pitch  14.34 [ +14.35 ..  +28.70]  yaw   9.21 [  -6.92 ..   +2.29]  roll   1.96 [ +30.00 ..  +31.96]
      Thumb_2 pitch   1.39 [  -0.03 ..   +1.37]  yaw   7.99 [ -45.73 ..  -37.74]  roll   0.46 [ -10.10 ..   -9.64]
      Thumb_3 pitch   2.92 [  -7.76 ..   -4.84]  yaw  17.45 [ -12.73 ..   +4.72]  roll   0.45 [  +9.26 ..   +9.71]
      Index_1 pitch   5.86 [  +4.05 ..   +9.91]  yaw  10.05 [ -31.78 ..  -21.73]  roll   1.99 [  +0.69 ..   +2.69]
      Index_2 pitch   0.46 [  -0.03 ..   +0.43]  yaw   8.70 [ -16.26 ..   -7.55]  roll   0.02 [  -3.04 ..   -3.03]
      Index_3 pitch   0.17 [  +3.06 ..   +3.22]  yaw   5.54 [  +2.21 ..   +7.75]  roll   0.19 [  -1.82 ..   -1.63]
     Middle_1 pitch   8.14 [  -1.27 ..   +6.87]  yaw  13.01 [ -37.69 ..  -24.67]  roll   4.12 [  -4.32 ..   -0.20]
     Middle_2 pitch   0.20 [  +0.20 ..   +0.40]  yaw   8.38 [ -12.74 ..   -4.36]  roll   0.06 [  -1.39 ..   -1.33]
     Middle_3 pitch   0.70 [  +0.11 ..   +0.81]  yaw   8.97 [  +0.51 ..   +9.48]  roll   0.95 [  -4.95 ..   -4.00]
       Ring_1 pitch   5.89 [  -0.00 ..   +5.89]  yaw  18.23 [ -34.62 ..  -16.39]  roll   3.38 [  -9.76 ..   -6.39]
       Ring_2 pitch   0.83 [  -0.93 ..   -0.10]  yaw  11.61 [ -18.00 ..   -6.39]  roll   0.16 [  -4.15 ..   -3.99]
       Ring_3 pitch   0.06 [  -3.40 ..   -3.34]  yaw   6.10 [  -3.92 ..   +2.18]  roll   0.11 [  -0.60 ..   -0.50]
      Pinky_0 pitch   0.00 [ +15.32 ..  +15.32]  yaw   0.00 [  -5.57 ..   -5.57]  roll   0.00 [ -24.89 ..  -24.89]
      Pinky_1 pitch  10.01 [  -9.62 ..   +0.39]  yaw  19.92 [ -27.22 ..   -7.30]  roll   3.09 [ +10.10 ..  +13.20]
      Pinky_2 pitch   1.93 [  +2.19 ..   +4.11]  yaw  17.13 [ -25.88 ..   -8.75]  roll   1.73 [  -7.25 ..   -5.52]
      Pinky_3 pitch   0.05 [  -5.64 ..   -5.59]  yaw  11.15 [  -8.46 ..   +2.69]  roll   0.58 [  -0.06 ..   +0.53]
        Wrist pitch  32.72 [  -7.06 ..  +25.67]  yaw   0.72 [-179.60 .. +179.67]  roll  28.95 [ -90.53 ..  -61.58]
    Hand pose range total square error - full=1325.39 half=331.35 quarter=82.84
