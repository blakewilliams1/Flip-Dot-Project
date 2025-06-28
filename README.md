This is a miniature flip dot wall made with scavenged panels from a much larger installation that fell out of maintenance.

It's powered by a Raspberry Pi 3b+ and an Intel Realsense camera. Here is a wiring schematic of how data gets from the Pi to the panels via the output RS485 twisted wires. The design is: Raspberry Pi -> 3v/5v logic level converter -> rs485 converter -> display panels.
![Imgur Image](http://i.imgur.com/JXdJaeP.jpg)

For compiling the rs-depth executable, It's really just a few minor modifications to the example program that comes with the RealSense SDK.
All the examples are compiled in bulk with 'make' command from root example directory then output executable is copied over into the main program folder.
