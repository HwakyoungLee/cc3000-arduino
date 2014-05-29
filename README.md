# Berg CC3000 Library For Arduino Mega

An Arduino Library for running connecting an Arduino Mega to [Berg](http://bergcloud.com/devcenter/). **Please note this library is in an alpha state** so please check the GitHub repository regularly and create an [issue on GitHub](https://github.com/bergcloud/BergCC3000/issues) if you encounter any problems.

## Hardware Requirements
* [Adafruit CC3000 Shield](http://www.adafruit.com/products/1491) or [Breakout](https://www.adafruit.com/products/1469).
* [Arduino Mega 2560](http://arduino.cc/en/Main/ArduinoBoardMega2560)

## Software Dependencies
The BergCC3000 library has dependencies on the following:

* [Arduino 1.0.5](http://arduino.cc/en/Main/Software#toc2)
* Modified [Adafruit CC3000 Library](https://github.com/bergcloud/Adafruit_CC3000_Library)
* Modified [aJson Library](https://github.com/bergcloud/aJson)
* Modified [Websockets Library](https://github.com/bergcloud/Arduino-Websocket). Note that this library folder should be called `Websockets`

## Installation
Download or clone the GitHub repos listed above and install into your ~/Documents/Arduino/libraries folder. You may need to remove any existing folders containing the Adafruit CC3000, aJson and Websockets libraries and replace them with the Berg forks.

When installing the BergCC3000 code you will need to copy the directory named `BergCC3000` directly inside the repository root, and not the repository itself.

## Berg Platform
This library is designed to work with the [v2 project API](http://bergcloud.com/devcenter/api/v2/cloud-v2) and implements a Device API similar to that of the [Berg Device API version 2](http://bergcloud.com/devcenter/api/v2/device-v2) except where Wifi communication requires it to behave differently. Complete documentation will be available on bergcloud.com when the library is officially released.

## Example code
We've included an example called `BERGCloud_BareBones.ino` which gives you a basic sketch structure for you to implement your projects. It will prompt you with the claim code when running for the first time, and you can bring it online by using the **List and claim devices** section of the [**Manage Projects**](http://bergcloud.com/devcenter/projects) page.

In order to join your sketch up with Berg you must first [create a project](http://bergcloud.com/devcenter/projects/new) in the Dev Center. This will give you a unique **Project Key** that you need to place inside your Arduino sketch. At the bottom of the project info screen you'll find a pre-generated line of code, beginning with `#define PROJECT_KEY` that you can copy and paste into the sketch at the top, replacing the existing `#define PROJECT_KEY` line.
