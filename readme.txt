Fred the Dalek head control for Arduino

http://redhunter.com/blog/category/dalek/

This repo tracks the development of Fred's head control code.

See the website for hardware details.

This works for my Dalek, hopefully other will find it useful as a basis for
controlling their project.

Legal disclaimer: Daleks are copyright BBC and Terry Nation. Fred is a
non-commercial fan-built Dalek not affiliated with either. 

Requires the following libraries

ACE128     https://github.com/arielnh56/ACE128
Keypad     http://playground.arduino.cc/code/Keypad
Keypad_MCP https://github.com/joeyoung/arduino_keypads/tree/master/Keypad_MCP 
WiiChuck   http://playground.arduino.cc/Main/WiiChuckClass

WiiChuck is modified with the addition of the following method:

// returns pitch in tenths of degrees
int readPitch10() {        
    return (int) (acos(readAccelY()/RADIUS)/ M_PI * 1800.0);  // optionally swap 'RADIUS' for 'R()'
}

This is the same as the original but gives 10X finer definition.


