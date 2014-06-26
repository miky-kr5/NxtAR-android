NxtAR: A generic software architecture for Augmented Reality based mobile robot control.
========================================================================================

Android backend module
----------------------

### Abstract ###

NxtAR is a generic software architecture for the development of Augmented Reality games
and applications centered around mobile robot control. This is a reference implementation
with support for [LEGO Mindstorms NXT][1] mobile robots.

### Module description ###

The Android backend module is a concrete [LibGDX][2] application that implements the operating
system dependent parts of the NxtAR reference implementation. It is based around the [OpenCV][3]
Computer Vision and Machine Learning library. Currently this module supports Android (>= 3.1)
devices though it has been tested only on Android (>= 4.0). The module includes direct support for
the [OUYA][4] gaming console and other devices using OUYA gamepads.

### Module installation and usage. ###

Install the NxtAR-core_XXXXXX.apk file on your device. To use you need additionally an Android (>= 3.0)
phone and a LEGO Mindstorms NXT robot with the [LejOS][5] firmware installed. The [NxtAR-cam][6] module must be
installed on the device and the [NxtAR-bot][7] module must be installed on the robot. Then, to start the compiled
scenario follow these steps:

* Start the NxtAR-core application.
* Start the NxtAR-bot program on the robot.
* Calibrate the robot's light sensor following the on-screen instructions.
* When the robot displays *"Waiting for connection"* start the NxtAR-cam application and connect it with the robot.
* Press the *"Start video streaming"* button on the NxtAR-cam application.
* Press the *"Calibrate camera"* button on the NxtAR-core application and point the camera of the device running NxtAR-cam to an OpenCV checkerboard camera calibration pattern.

The camera calibration step can be repeated if needed.

 [1]: http://www.lego.com/en-us/mindstorms/?domainredir=mindstorms.lego.com
 [2]: http://libgdx.badlogicgames.com/
 [3]: http://opencv.org/
 [4]: https://www.ouya.tv/
 [5]: http://www.lejos.org/nxj.php
 [6]: https://github.com/sagge-miky/NxtAR-cam
 [7]: https://github.com/sagge-miky/NxtAR-bot
