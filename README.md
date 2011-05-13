Scattering The World
====================

A simple interactive visual effect using Microsoft's Kinect and Cinder.

Currently the system is limited to 5000 particles.  When I get the chance, I
intend to implement a particle system that uses location binning to limit
particle interactions to nearest neighbor interactions. This will probably
raise the particle capacity to 14k particles @ 30fps. GPU based techniques
would be needed to implement a larger number of particles.

Requirements
------------
To date, the only environment I have tested this on is the following:

  * Mac OS X 10.6.7
  * Xcode 3.2.6
  * [Homebrew 0.8](http://mxcl.github.com/homebrew/)
  * libusb with OpenKinect patch. [Homebrew formula](https://github.com/OpenKinect/libfreenect/raw/unstable/platform/osx/homebrew/libusb-freenect.rb)
  * [OpenNI-Binaries](https://github.com/OpenNI/OpenNI/tree/unstable) [v1.1.0.41](http://openni.org/downloadfiles/openni-binaries/latest-unstable/107-openni-unstable-build-for-macosx-10-6-universal-x86x64-3264-bit-v1-1-0/download)
  * [SensorKinect](https://github.com/avin2/SensorKinect) [v5.0.1.32](https://github.com/avin2/SensorKinect/raw/unstable/Bin/SensorKinect-Bin-MacOSX-v5.0.1.32.tar.bz2)
  * [NITE-MiddleWare Binaries](openni.org) [v1.1.0.41](http://openni.org/downloadfiles/openni-binaries/latest-unstable/107-openni-unstable-build-for-macosx-10-6-universal-x86x64-3264-bit-v1-1-0/download)
  * [Cinder](libcinder.org) [HEAD](https://github.com/cinder/Cinder)
  * And of course: A [Kinect](www.xbox.com/kinect) (no seriously, it will not run without a kinect plugged into the computer.)


Installing Prerequisites
------------------------

  1. If you haven't already downloaded and installed Xcode 3.2.6, Grab it [here](https://developer.apple.com/devcenter/download.action?path=/Developer_Tools/xcode_3.2.6_and_ios_sdk_4.3__final/xcode_3.2.6_and_ios_sdk_4.3.dmg).  I have not tested this with Xcode 4. You will need to log in.

  2. Fire up Terminal.app and install Homebrew and git

        ruby -e "$(curl -fsSL https://gist.github.com/raw/323731/install_homebrew.rb)"
        brew install git
     
  3. Then install libusb

        brew install https://github.com/OpenKinect/libfreenect/raw/unstable/platform/osx/homebrew/libusb-freenect.rb

  4. Create a link between /usr/local and /opt/local to fool NITE into thinking we like macports.

        ln -s /usr/local /opt/local

  5. Install OpenNI using the following steps:

    1. Download the package. 

    2. extract the package into its own directory.

    3. `cd` to that directory in the terminal and run

            sudo ./install.sh 

  4. Repeat Step 5. for NITE and SensorKinect


Seting Up the Build Environment
-------------------------------

Now that you have a working install of OpenNI and NITE we need to download and build Cinder.

  1. Make a directory on your desktop and call it `CinderProjects`

        cd ~/Desktop
        mkdir CinderProjects

  2. Clone cinder's git repository 

        cd ~/Desktop/CinderProjects
        git clone git://github.com/cinder/Cinder.git Cinder

  3. Download the compatible version of
  boost([v1.44.0](http://sourceforge.net/projects/boost/files/boost/1.44.0/)),
  Extract it, and rename the directory to `boost`.  Copy this directory to
  `~/Desktop/CinderProjects/Cinder`.

  4. Build Cinder:

        cd ~/Desktop/CinderProjects/Cinder/xcode
        ./fullbuild.sh

With this step complete you should have a working build environment for this project. All that remains is to clone this repo into `~/Desktop/CinderProjects`. 

        cd ~/Desktop/CinderProjects
        git clone git://github.com/jimmyharris/ScatterTheWorld

Build Instructions
------------------

  1. Open the project `ScatterTheWorld/xcode/ScatterTheWorld.xcodeproj` in Xcode.
  2. Make sure the target is set to `Release`
  3. Plug in your kinect.
  4. Hit Build & Run


Controls
--------

Gestures:

 - Wave to initialte hand tracking. If any existing particles are in the system they are cleared.

While Tracking:

  - Push to toggle particle creation on/off.

  - Move the hand to spread particles out over the screen.

  - Wave to leave the seesion.

Keyboard Controlls:

  - `1` Toggle display Visible Light map (grayscale camera).

  - `2` Toggle Rendering of Particles.

  - `g` Toggle Gravitational pull towards the center of the window.

  - `p` Toggle Perlin Noise effect on particles.

  - `s` Begin recording frames to a  movie to the desktop. The file will be named `TestMovie.mov`. Pressing `s` again will stop it.


