# About
This is an OS made for fun. I mainly used information from the OSdev wiki to make this aswell as from https://github.com/cfenollosa/os-tutorial. I use 640x480 VGA mode with 4-bit colors, I provide multicore support for 3D rendering, provide a disk driver for an IDE controller using ATA PIO mode and I make use of an Intel 82540EM network card. Though the source code quickly became very messy, I'm am happy with the result.
# How to use it
I have always used this on Oracle Virtualbox so that's how I will show it here.  
I do assume that your computer has an Intel Core chip and I am not sure whether some features that I use are supported on other chips, either way, startup of the OS will halt if you miss any features so you might as well try ;). You will also need atleast two cores (preferably more) on your processor.  
  
Add a new machine to your virtualbox with whatever name you like and give it your recommended amount of RAM.  
It doesn't matter how big the harddisk is since we will remove it anyway thus take for example "new disk"->"VDI"->"statically allocated"->"4 MB".  


