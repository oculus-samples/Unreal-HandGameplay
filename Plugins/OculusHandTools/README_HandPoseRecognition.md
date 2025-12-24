# The OculusHandPoseRecognition Module

This module includes two [USceneComponent](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/USceneComponent?application_version=5.6) subclasses. They recognize specific hand poses—defined by relative hand bone angles—and hand gestures—defined as sequences of hand poses. It also provides a library of useful blueprint utility nodes.

## 1. Hand Pose Recognizer Component

The [UHandPoseRecognizer](./Source/OculusHandPoseRecognition/Public/HandPoseRecognizer.h) class is the core of the hand recognition system. It regularly polls the [Oculus Hands API](../../../../../Engine/Plugins/Runtime/Oculus/OculusVR/Source/OculusInput/Public/OculusXRInputFunctionLibrary.h) to record hand bone data when reliable. Wrist bone data comes from the motion controller component attached to the player's VR character.

<img width="256" src="./Media/recognizer_attachment.png" alt="Recognizer attached to controller." />

You can use multiple recognizers per hand, each with its own set of hand poses. Group poses into one recognizer when they are mutually exclusive. For example, our showcase uses one recognizer for American Sign Language numbers and another for letters. Although ASL distinguishes the letter 'O' from the number '0', our example uses static hand poses recognized by separate recognizers to detect both simultaneously.

### Pose Strings

Each recognizer defines what it can recognize as a set of bone angles. Here is the thumbs-up pose for the left hand:

    L T0-52-18+51 T1+13-8+30 T2+7-9-10 T3-10+21+8
      I1+6-72+1 I2-3-108+1 I3+1-55-3
      M1+1-77-8 M2-1-99+1 M3-6-51-8
      R1-4-85-10 R2-5-100-1 R3-4-50-1
      P0+15-6-25 P1+8-88+4 P2-8-94-7 P3-4-54+2
      W+81+0+0

The description starts with 'L' or 'R' for left or right hand. It lists hand bones in order: 'T' (Thumb), 'I' (Index), 'M' (Middle), 'R' (Ring), 'P' (Pinky), and 'W' (Wrist). All bones except the wrist include a bone index starting at 0 for thumb and pinky, and 1 for the other fingers.

Angles are three signed numbers for pitch, yaw, and roll, in degrees.

You can generate new pose strings using the logger described in [Using a Hand Pose Recognizer](#using-a-hand-pose-recognizer).

### Computing Distance (Error) to Hand Pose

Matching a hand to a reference pose starts by summing the squares of all angular differences. A lower sum means a better match. You decide what error value counts as a good match; this is covered in the next section.

Sometimes, you may want to ignore some bones. For example, in a "gun" pose, some people use only the index finger, others use both index and middle fingers. To match both with one pose, remove the middle finger bones from the reference pose.

You can also ignore specific angles (pitch, yaw, or roll) of a bone. For example, in the thumbs-up pose, you only need to know if the thumb points up or down, not its facing direction. Use +0 to ignore an angle. Use +1 or -1 to recognize angles near zero.

We have realized that for some bones are more (or less) important than others for some poses.  For that purpose, you can add between the bone identifier and the pitch/yaw/roll values a weight value.  Here's a full example that combines weights and ignores specific fingers:

    L T0*3-61+1+31 T1*3+12-8+30 T2*3+6-13-10 T3*3-10+21+8
      I1+16-22-1 I2+1-2-3 I3+3+2-2
      R1-2-77-12 R2-5-100-1 R3-4-57-1
      P0+15-6-25 P1+8-90-3 P2-5-76-8 P3-4+1+3

This left hand "gun" pose differs from the "gun shot" pose because the thumb is lifted away from the index. The thumb's weight is increased by 3. The middle finger is ignored to match both single and double-barrel poses. The wrist is ignored, so hand orientation of which way the hand is pointed does not matter.

### Error vs Confidence Level

Weights and ignoring angles or fingers affect the error range. You may also want to be more lenient with some poses.

To normalize this, each pose defines a maximum error value for 100% confidence (error at max confidence, EAMC).

For example, if you set EAMC to 2000, any error below 2000 means full confidence (1.0). An error of 4000 corresponds to 0.5 confidence, and so on.

| Error | Confidence (EAMC = 2000) |
| ----- | ------------------------ |
| 500   | 1.00                     |
| 2000  | 1.00                     |
| 3000  | 0.66                     |
| 4000  | 0.50                     |
| 6000  | 0.33                     |
| 8000  | 0.25                     |

Confidence levels simplify handling recognition results, but raw errors remain useful during development to set EAMC.

### Configuring a Hand Pose Recognizer

In the hand pose recognizer's details panel, find the *Hand Pose Recognition* section.

<img width="256" src="./Media/recognizer_config.png" alt="Recognizer configuration." />

Set the *side* value to recognize the left or right hand. Set it to none to disable the recognizer.

The recognition interval throttles recognition frequency. The default 0s runs recognition every tick.

The confidence floor sets the minimum confidence required to recognize a pose. You can set a default at the recognizer level and customize it per pose.

The damping factor controls how slowly bone updates integrate per recognition interval. By default, the latest values fully replace the current state every tick. A value of 0.2 blends 80% of the latest value with the current state.

You can configure an array of [poses](#pose-strings). There is no limit to the number of poses per recognizer.

Each pose has a name, which need not be unique. You also set the encoded pose, custom confidence floor, and error at max confidence.

### Using a Hand Pose Recognizer

The *Log Encoded Hand Pose* blueprint node outputs the current hand pose as an encoded string to the output log. The example below shows recognizers for both hands wired to input events.

<img width="256" src="./Media/log_encoded_pose.png" alt="Log hand pose." />

In this example, CTRL-L and CTRL-R keyboard events trigger logging. A typical workflow is to generate hand poses, log them, then transfer or modify the strings for use in a hand pose recognizer.

The *Get Recognized Hand Pose* node retrieves the current recognition state.

<img width="512" src="./Media/get_recognized_hand_pose.png" alt="Get recognized hand pose." />

This image from the Hand Pose Showcase shows the VRCharacter blueprint forwarding the recognizer's state to display on a projector slide.

When a pose is recognized (confidence above the floor), you get its index, name, duration held (seconds), raw error, and confidence level.

## 2. Hand Gesture Recognizer Component

The [UHandGestureRecognizer](./Source/OculusHandPoseRecognition/Public/HandGestureRecognizer.h) class handles gesture recognition.

A gesture is a sequence of hand poses. Our implementation recognizes gestures only from one hand pose recognizer, which is the common case. Attach the gesture recognizer to the pose recognizer it depends on.

<img width="256" src="./Media/gesture_recognizer.png" alt="Gesture recognizer." />

In this screenshot, *LeftFlickSwipeGestureRecognizer* attaches to *LeftFlickSwipePoseRecognizer*. This gesture moves the projector to the previous slide. Similar recognizers on the right hand move to the next slide.

### Defining a Gesture as a Sequence of Poses

In the Hand Pose Showcase, a sci-fi movie gesture changes channels by pointing the right index and middle fingers at the screen, then flicking left.

We tested other gestures but found this one easy to learn and perform. First, the static hand pose recognizer:

<img width="256" src="./Media/flick_pose.png" alt="Flick pose recognition." />

It recognizes two poses: pointing and flicking. Next, the gesture recognizer:

<img width="256" src="./Media/flick_gesture.png" alt="Flick gesture recognition." />

The recognition interval throttles gesture recognition frequency. It defaults to every game tick.

The skipped frames value adds delay control but is experimental and may be removed.

This example recognizes one gesture, "Flick," which transitions from "Point" to "Flick." Gesture names need not be unique.

*Max Transition Time* sets the maximum allowed time (seconds) between poses, including when no pose is recognized. Here, it tolerates up to 0.2 seconds between "Point" and "Flick."

You can require holding a pose before continuing. For example:

    Point/200, Flick

The user must hold "Point" for 200 milliseconds before "Flick" triggers the gesture.

The *Is Looping* flag enables recognition of looping gestures, like waving your hand.

### Using a Hand Gesture Recognizer

The gesture recognizer supports *force grab* and *force throw* gestures. Here is part of the grabbing code in VRCharacter:

<img width="512" src="./Media/force_grab_gesture.png" alt="Grab gesture recognition." />

This node returns whether a gesture was recognized. If so, you get the gesture index, name (not necessarily unique), and direction.

The gesture direction is a vector from an average location near the end of the first pose to an average location near the start of the last pose.

We also experimented with gesture duration measures. The outer duration spans from the start of the first pose to the end of the last. The inner duration spans from near the end of the first pose to near the start of the last.

The *Behavior* argument controls how the recognizer resets after recognizing a gesture. By default, only the recognized gesture resets. You can reset all gestures if needed.

<img width="512" src="./Media/gesture_state.png" alt="Gesture progress." />

The Hand Pose Showcase shows how to detect a gesture in progress. Here, the player's hand highlights when ready to flick to the previous or next slide. The "Flick" gesture is queried to check if it is in progress.

## 3. Hand Recognition Blueprint Library

<img width="512" src="./Media/wait_nodes.png" alt="Pose and gesture wait nodes." />

### Waiting for Hand Pose

UE4 supports latent blueprint nodes that pause execution until a condition is met.

The *Wait for Hand Pose* node waits for a specified UHandPoseRecognizer instance to recognize a pose. Specify *Min Pose Duration* to require the pose be held for a minimum time. If you set a non-negative *Time to Wait*, the node exits via the *Time Out* pin if no pose is recognized within that time.

When a pose is recognized, execution resumes at the *Pose Seen* pin, providing the recognized pose index and name.

### Waiting for Gesture

The *Wait for Hand Gesture* node waits for a gesture recognition. Specify how long to wait with *Time to Wait*. The *Behavior* argument matches that in the *GetRecognizedHandGesture* node.

When a gesture is recognized, execution resumes at the *Gesture Seen* pin, providing gesture index, name, direction, and durations.

If *Time to Wait* expires, the node exits via the *Time Out* pin.

You can loop back into the wait node to wait again, as shown in the screenshot.

### Recording a Pose Range

<img width="512" src="./Media/pose_range_recording.png" alt="Pose range recording." />

The *Record Hand Pose* blueprint records the range of hand bone angles. In the screenshot from *HandPoseShowcase*, you start and stop recording with a keyboard key. When stopped, the node outputs a report like this (recording the [royal wave](https://www.youtube.com/watch?v=n5pkDB7zEeo)):

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
