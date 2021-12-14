# Hand Gameplay Showcase

[<img width="100%" src="./Media/hands-hero.png" />](https://www.oculus.com/experiences/quest/4232440213539049/)

This project contains reusable components based on the most robust hand tracking mechanics from [First Steps with Hand Tracking](https://www.oculus.com/experiences/quest/3974885535895823/?locale=en_US) and [Tiny Castles](https://www.oculus.com/experiences/quest/3647163948685453/?locale=en_US).

You can try the showcase for yourself on Quest [here](https://www.oculus.com/experiences/quest/4232440213539049/).

This codebase is available both as a reference and as a template for multiplayer VR games. All code and assets are under the license found [here](LICENSE) unless otherwise specified.

See the [CONTRIBUTING](CONTRIBUTING.md) file for how to help out.

## How to Use

To try the showcase, simply load the project in the Unreal Editor, build, and launch!

To integrate these features into your own project, install the OculusHandTools plugin into your project. The easiest way to do so is to download the latest [release](../../releases/latest) of `OculusHandTools.zip` and extract it into your project's `Plugins` folder.

You can find a detailed breakdown of how the mechanics are implemented [here](./Plugins/OculusHandTools/#mechanics-implementations). In addition to the featured mechanics, the OculusHandTools plugin has several useful C++ modules:

- [HandInput](./Plugins/OculusHandTools/README_HandInput.md)
- [HandPoseRecognition](./Plugins/OculusHandTools/README_HandPoseRecognition.md)
- [OculusHandTrackingFilter](./Plugins/OculusHandTools/README_HandTrackingFilter.md)
- [OculusInteractable](./Plugins/OculusHandTools/README_Interactable.md)
- [OculusThrowAssist](./Plugins/OculusHandTools/README_ThrowAssist.md)
- [OculusUtils](./Plugins/OculusHandTools/README_OculusUtils.md)

## Features

||||
|-|-|-|
|**[Teleportation](./Plugins/OculusHandTools/#teleportation)**|<img width="256" src="./Media/teleportation.png" />|A simple movement mechanic using pose recognition from the Hand Pose Showcase.|
|**[Grabbing](./Plugins/OculusHandTools/README_HandInput.md)**|<img width="256" src="./Media/grabbing.png" />|A system for recognizing natural grab gestures, attaching the object to your hand, and overriding the pose of your rendered hand for visual feedback.|
|**[Throwing](./Plugins/OculusHandTools/#throwing)**|&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|This component uses the hand’s historical data to determine the velocity of the thrown object.|
|**[Button Pushing](./Plugins/OculusHandTools/#pushing-buttons)**|<img width="256" src="./Media/button.png" />|A reliable digital interaction. (Get it, digital? It’s on or off, but also your pointer finger is a digit. I’ll let myself out…)|
|**[Punching](./Plugins/OculusHandTools/#punching)**|<img width="256" src="./Media/punching.png" />|Punching is a fulfilling interaction with your hands, despite the fast movement being a cause of tracking loss.|
|**[Hand Movement Filtering](./Plugins/OculusHandTools/README_HandTrackingFilter.md)**||Stabilizes the hand and finger movement while tracking is low quality or lost. This significantly improves the feel of using your hands, particularly in poor tracking situations like while punching. You can read more about the implementation of this component [here](https://developer.oculus.com/blog/adding-hand-tracking-to-first-steps/).|
|**[Two-handed Aiming](./Plugins/OculusHandTools/#two-handed-aiming)**|<img width="256" src="./Media/aiming.png" />|Aiming is a reliable, useful, and fulfilling hand interaction when done with both hands.|
|**[Example Hands for Tutorials](./Plugins/OculusHandTools/#example-hands-for-tutorials)**||Effectively illustrate to users the poses your app expects.|
