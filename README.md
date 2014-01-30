objdetect-opencv
================

detect obj and react accordingly

Here is my code for opencv, it should be able to run on mac. I tested and compiled on my mac os 64 bit. i use xcode+ opencv to write this. but i think it also works on ubuntu if u can compile it from my source code.

here is how it works. I crop the top part of webcam as region of interest, the rest would be ignored. if it detects the object, depends on its coordinate, it would print out the robot move direction accordingly. at this moment i use single point of following line, which i think should be sufficient. Although the accuracy for single point is not good enough, but i think i should be sufficient for your competition.

The worst thing about single point is, if it reached the end of line, and it would probably turn a big round, and move back to starting. That is not what we wanted.

Anyway, this is how to use my app. drag the window and display it as how it looks like my attached image. find something black, such as your hair on your head or a black belt to simulate the line. Once detected, it would print out on terminal for robot movement as well its coordinate.
