# 589project

This project should be able to:
      refer to proposal

Env: MacOS 10.12.6 // /fedora

Instructions:
      To run: make run // / make -f fedoraMakefile run
      Camera Controls:
            mouse mid Pressed with mouse movement to change camera angle
            mouse scroll to adjust camera distance
            mouse mid Pressed and shift pressed with mouse movement trans camera center
      Points Controls:
            key 1 to shift between 1 time point getter and constant time interval pointer getter
            key space press and release to start/stop point getter save data to temp
            key s to save temp to stack
            key tab to toggle different obj
            key n to pop stack to make obj

Based on:
      cpsc453 & cpsc587 & cpsc589 assignment
      makefile, VertexArray.cpp and Program.cpp are adapted from Boilerplate code.
Demo:


Log:
March 10
ios side finished, point detected.
Server obj finished with multithread and mutex. API: start() getXY(index)

March 11
https://developer.apple.com/library/content/documentation/DeviceInformation/Reference/iOSDeviceCompatibility/Cameras/Cameras.html
