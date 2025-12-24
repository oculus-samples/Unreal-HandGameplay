# Hand Gameplay Showcase

[<img width="100%" src="./Media/hands-hero.png" />](https://www.meta.com/experiences/oculus-hand-gameplay-showcase-for-unreal/4232440213539049/)

This project offers reusable components built on the robust hand tracking mechanics from [First Steps with Hand Tracking](https://www.meta.com/experiences/first-steps-with-hand-tracking/3974885535895823/) and [Tiny Castles](https://www.meta.com/experiences/tiny-castles/3647163948685453/).

Try the showcase yourself on the [Horizon Store](https://www.meta.com/experiences/4232440213539049/).

## Build Instructions

### Download the Project

First, install Git LFS by running:
```sh
git lfs install
```

Next, clone this repository using the "Code" button above or run:
```sh
git clone https://github.com/oculus-samples/Unreal-HandGameplay.git
```

Finally, open the project in Unreal Editor using one of the following methods.

#### Epic Games Launcher

The easiest way to start is with the prebuilt Unreal Engine from the Epic Games Launcher. Note that the [Hand Movement Filtering](./Plugins/OculusHandTools/README_HandTrackingFilter.md) will not work without the Oculus fork described below.

1. Install the [Epic Games Launcher](https://store.epicgames.com/en-US/download).
2. Install Unreal Engine 5.3 or later via the launcher.
3. Launch Unreal Editor.
4. Click "More" <br /><img width="400" src="https://user-images.githubusercontent.com/791774/148618198-afbe2e70-18a4-41ec-9bad-bf90fac05edc.png" />
5. Click "Browse" and select `HandGameplay.uproject`.

#### Oculus Unreal Fork

The Oculus Unreal fork provides the latest Oculus feature integration but requires building the editor from source.

1. [Get access to the Unreal source code](https://developers.meta.com/horizon/documentation/unreal/unreal-building-ue4-from-source/#prerequisites).
2. [Clone the `oculus-5.3` branch of the Oculus fork](https://github.com/Oculus-VR/UnrealEngine/tree/oculus-5.3).
3. [Install Visual Studio](https://developers.meta.com/horizon/documentation/unreal/unreal-building-ue4-from-source/#to-download-and-build-unreal-engine).
4. Open a command prompt in the Unreal root directory and run:
```sh
.\GenerateProjectFiles.bat -Game HandGameplay -Engine <full path to Unreal-HandGameplay directory>\HandGameplay.uproject
```
5. Open the generated `HandGameplay.sln` file in the `Unreal-HandGameplay` directory.
6. Set `HandGameplay` as the start-up project and `Development Editor` as the configuration.
7. Press `F5` to build and debug the project and engine.

### Use as Plugin

To add these features to your project, install the OculusHandTools plugin. Download the latest release of `OculusHandTools.zip` and extract it into your project's `Plugins` folder.

For a detailed explanation of the mechanics, see [here](./Plugins/OculusHandTools/README.md#mechanics-implementations). The OculusHandTools plugin also includes several useful C++ modules:

- [HandInput](./Plugins/OculusHandTools/README_HandInput.md)
- [HandPoseRecognition](./Plugins/OculusHandTools/README_HandPoseRecognition.md)
- [OculusHandTrackingFilter](./Plugins/OculusHandTools/README_HandTrackingFilter.md)
- [OculusInteractable](./Plugins/OculusHandTools/README_Interactable.md)
- [OculusThrowAssist](./Plugins/OculusHandTools/README_ThrowAssist.md)
- [OculusUtils](./Plugins/OculusHandTools/README_OculusUtils.md)

## Features

| Feature | Image | Description |
|---------|-------|-------------|
| **[Teleportation](./Plugins/OculusHandTools/README.md#teleportation)** | <img width="256" src="./Media/teleportation.png" /> | Simple movement using pose recognition from the Hand Pose Showcase. |
| **[Grabbing](./Plugins/OculusHandTools/README_HandInput.md)** | <img width="256" src="./Media/grabbing.png" /> | Recognizes natural grab gestures, attaches objects to your hand, and overrides hand pose for visual feedback. |
| **[Throwing](./Plugins/OculusHandTools/README.md#throwing)** | &nbsp; | Uses hand history data to calculate the velocity of thrown objects. |
| **[Button Pushing](./Plugins/OculusHandTools/README.md#pushing-buttons)** | <img width="256" src="./Media/button.png" /> | Reliable digital interaction (on/off). (Bonus: your pointer finger is a digit!) |
| **[Punching](./Plugins/OculusHandTools/README.md#punching)** | <img width="256" src="./Media/punching.png" /> | A satisfying hand interaction, despite fast movement sometimes causing tracking loss. |
| **[Hand Movement Filtering](./Plugins/OculusHandTools/README_HandTrackingFilter.md)** | &nbsp; | Stabilizes hand and finger movement during low-quality or lost tracking, improving feel especially during punching. More details [here](https://developers.meta.com/horizon/blog/adding-hand-tracking-to-first-steps/). |
| **[Two-handed Aiming](./Plugins/OculusHandTools/README.md#two-handed-aiming)** | <img width="256" src="./Media/aiming.png" /> | Reliable and fulfilling hand interaction using both hands. |
| **[Example Hands for Tutorials](./Plugins/OculusHandTools/README.md#example-hands-for-tutorials)** | &nbsp; | Illustrates the poses your app expects from users. |

## License

This codebase serves as a reference and template for multiplayer VR games. All code and assets follow the license found [here](./LICENSE), unless otherwise noted.

## Contribution

See the [CONTRIBUTING](./CONTRIBUTING.md) file for contribution guidelines.

# Updates

## 20 December 2023 Update

We updated the project to UE5.3.

## March 2025 Update

We updated the project to use OpenXR from Epic with meta vendor extensions.
Please note, you can still use the OVRPlugin, but you'll need to update the Grab Poses on:

* Content/HandGameplay/Probs/Blocks/InteractableBrick
* Content/HandGameplay/Probs/RingWeapon/InteractableArtifactHandle

Example for InteractableBrick:

* Change relative Hand Transform left to: ``LOC X1.623 Y12.178 Z8.067 ROT W0.160 X0.189 Y-0.832 Z-0.497``
* Change relative Hand Transform right to: ``LOC X5.690 Y-10.448 Z-8.640 ROT W0.840 X0.534 Y0.064 Z0.068``

Example for InteractableArtifactHandle:

* Change relative Hand Transform left to: ``LOC X1.094 Y6.294 Z11.635 ROT W0.101 X-0.548 Y-0.830 Z-0.009``
* Change relative Hand Transform right to: ``LOC X-0.519 Y-6.451 Z-12.047 ROT W0.859 X0.053 Y0.058 Z-0.506``

Best way to get new HandTransforms:

* Open HansCharacterHandsState from OculusHandTools/Content/Hands/
* Reconnect the Blueprint-flow-nodes
* This will output the location of your hand when grabbing an object in the correct format for copy and paste.
