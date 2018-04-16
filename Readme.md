# 589project

Env: MacOS 10.12.6 / fedora

Instructions:
      To run:
            make run / make -f fedoraMakefile run
            ios side app launch

      Camera Controls:
            mouse mid Pressed with mouse movement to change camera angle
            mouse scroll to adjust camera distance
            mouse mid Pressed and shift pressed with mouse movement trans camera center
            key_c camera reset
      Objects Controls:
            key_1 to shift between 1 time point getter and constant time interval pointer getter
            key_space press/release to start/stop point getter and save data to temp
            key_s to save temp to stack
            key tab to toggle different obj
            key n to pop stack to make obj

Based on:
      cpsc453 & cpsc587 & cpsc589 assignment
      makefile, VertexArray.cpp and Program.cpp are adapted from Boilerplate code.
Demo:

Log:
      point detection on ios side finished
      communication: ios stream and c++ server: finished with multithread and mutex.
      frame: process design(tempPoints->stack->objs), pointGetter design(single, continues), realtime frames.
      tool kit(renderableObj):
            add: curve(line seg, Bspline),  surface(bilinear blending, rotational blending, cross sectional blending, sweep surface), other(point, spray)
            selecting and pasting
      Testing and demo scene
            readFile for testing
            shading
