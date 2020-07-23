#!/bin/bash

#Array of dependencies any new dependencies can be added here
deps=(
"alsa-utils"
"cmake"
"libboost-all-dev"
"libusb-1.0.0-dev"
"libssl-dev"
"libprotobuf-dev"
"protobuf-c-compiler"
"protobuf-compiler"
"libqt5multimedia5"
"libqt5multimedia5-plugins"
"libqt5multimediawidgets5"
"qtmultimedia5-dev"
"libqt5bluetooth5"
"libqt5bluetooth5-bin"
"qtconnectivity5-dev"
"pulseaudio"
"librtaudio-dev"
"librtaudio6"
"libkf5bluezqt-dev"
"libtag1-dev"
"qml-module-qtquick2"
"doxygen"
"qml-module-qtquick*"
"libglib2.0-dev"
"libgstreamer1.0-dev"
"gstreamer1.0-plugins-base-apps"
"gstreamer1.0-plugins-bad"
"gstreamer1.0-libav"
"gstreamer1.0-alsa"
"libgstreamer-plugins-base1.0-dev"
"qtdeclarative5-dev"
"qt5-default"
"libgstreamer-plugins-bad1.0-dev"
"libunwind-dev"
"qml-module-qtmultimedia"
)
echo "installing Gstreamer dependencies"
for app in ${deps[@]}; do
	echo "installing: " $app
        sudo apt install $app -y 

        if [[ $? > 0 ]]
        then
            echo $app " Failed to install, quitting"
            exit
        else
            echo $app " Installed ok"
            echo

        fi
done

echo "All dependencies installed"
echo

#make ilclient
echo "making ilclient"
make /opt/vc/src/hello_pi/libs/ilclient
if [[ $? > 0 ]]
  then
    echo "unable to make ilclient"
  exit
else
  echo "made ok"
  echo
fi

echo "Cloning Gstreamer"
git clone git://anongit.freedesktop.org/gstreamer/qt-gstreamer
if [[ $? > 0 ]]
  then
    echo "unable to clone Gstreamer"
  exit
else
  echo "cloned OK"
  echo
fi

cd qt-gstreamer

echo "creating Gstreamer build directory"
mkdir build
if [[ $? > 0 ]]
  then
    echo "unable to create Gstreamer build directory"
  exit
else
  echo "build directory made"
  echo
fi

cd build

echo "beginning cmake"

cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib/$(dpkg-architecture -qDEB_HOST_MULTIARCH) -DCMAKE_INSTALL_INCLUDEDIR=include -DQT_VERSION=5 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-std=c++11

if [[ $? > 0 ]]
  then
    echo "cmake failed"
  exit
else
  echo "make ok"
  echo
fi

echo "making J4"
make -j4

if [[ $? > 0 ]]
  then
    echo "unable to make j4"
  exit
else
  echo "make ok"
  echo
fi

echo "beginning make install"
sudo make install

if [[ $? > 0 ]]
  then
    echo "unable to make Gstreamer"
  exit
else
  echo "make ok"
  echo
fi

sudo ldconfig

cd ../..

#move to build directory
echo "Moving to build directory"
cd build
if [[ $? > 0 ]]
  then
    #if directory doesn't exist, then create
    echo "Directory doesnt exist...creating"
    mkdir build
    if [[ $? > 0 ]]
     then
       echo $app " unable to create directory"
       exit
     else
        cd build
        if [[ $? > 0 ]]
           then
              echo $app "Cant change into directory"
              exit
            else
              echo "moved directory"
              echo
        fi
    fi
   
else
  echo "moved directory"
  echo
fi

#begin cmake
echo "beginning cmake for raspberry pi"
cmake -DRPI_BUILD=TRUE -DCMAKE_BUILD_TYPE=Release ../ -DGST_BUILD=True
if [[ $? > 0 ]]
  then
    echo "Cmake error"
  exit
else
  echo "Cmake ok"
  echo #####
fi

#start make
echo "beginning make, this will take a while"
make
if [[ $? > 0 ]]
  then
    echo "make error check output above"
    exit
  else
    echo "make ok, executable can be found ../bin/ia"
    echo

    #check if usb rules exist
    echo "checking if permissions exist"

    #udev rule to be created below, change as needed
    FILE=/etc/udev/rules.d/51-iadash.rules
    if [[ -f "$FILE" ]]
      then
        echo "rules exists"
        echo
      else
        sudo touch $FILE

        # OPEN USB RULE, CREATE MORE SECURE RULE IF REQUIRED
        echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"*\", ATTR{idProduct}==\"*\", MODE=\"0660\", GROUP=\"plugdev\"" | sudo tee $FILE
      if [[ $? > 0 ]]
        then
          echo "unable to create permissions"
        else
          echo "permissions created"
      fi
    fi
fi

#Start app
echo "starting app"
cd ../bin
./ia
