# About
This is an OS made for fun. I mainly used information from the OSdev wiki to make this aswell as from https://github.com/cfenollosa/os-tutorial. I use 640x480 VGA mode with 4-bit colors, I provide multicore support for 3D rendering, provide a disk driver for an IDE controller using ATA PIO mode and I make use of an Intel 82540EM network card. Though the source code quickly became very messy, I'm am happy with the result.
# How to use it
I have always used this on Oracle Virtualbox so that's how I will show it here.  
I do assume that your computer has an Intel Core chip and I am not sure whether some features that I use are supported on other chips, either way, startup of the OS will halt if you miss any features so you might as well try ;). You will also need atleast two cores (preferably more) on your processor.  
  
Add a new machine to your virtualbox with whatever name you like and give it your recommended amount of RAM.  
It doesn't matter how big the hard disk is since we will remove it anyway thus take for example "new disk"->"VDI"->"statically allocated"->"4 MB".  
Now to the important part, select the machine you just made and click "Settings". Go to "System" and then "Processor" and tell virtualbox to use more than one core, it's best to use the maximum amount of distinct !physical! processors on your chip.  
![image](https://user-images.githubusercontent.com/44338633/126883983-2a18ec37-f0b4-4453-9069-582f75ddfa2c.png)  
Next go to storage and delete the current controller.  
![image](https://user-images.githubusercontent.com/44338633/126883972-581d20d6-497b-4469-821d-ccd83a4cddc2.png)  
Add a new controller with "IDE" somewhere in the name (PIIX4 for example).  
![image](https://user-images.githubusercontent.com/44338633/126884054-0b17d54d-ba70-4f45-927a-ef80c2346d64.png)  
Then add a hard drive to this controller, create a new hard drive, VDI, statically allocated, 4 MB or bigger is fine and choose the one you just made to add to the controller to end up with somthing like this:  
![image](https://user-images.githubusercontent.com/44338633/126884120-13901139-073a-4030-bb1e-380e0ddd7005.png)  
Make sure that the drive you just made is an IDE Primary Master.  
Next go to network and instead of NAT, choose a network bridge adapter.  
![image](https://user-images.githubusercontent.com/44338633/126884161-3857c027-053b-44cd-81ed-353d693faf13.png)  
Go to the Advanced Settings and make sure that you are using the 82540EM.  
![image](https://user-images.githubusercontent.com/44338633/126884187-9e128c97-a799-4150-b951-0eada3b1e83c.png)  
Make sure you save all the changes in the settings and also remember the name of the drive for the IDE controller, in my case it was "Test_1.vdi" because we now will have to put our disk image into this file. First we will need to compile the entire OS, to do this you will need a linux terminal to execute the Makefile that is included here. I have only ever tested the Makefile on the Ubuntu 18.04 LTS for windows which you can get via the Microsoft Store these days it appears. Executing the make command should give somthing like:  
![image](https://user-images.githubusercontent.com/44338633/126884304-6005496c-276b-4443-9212-618b9f5b1c40.png)  
Use a file explorer to find the disk image called "os_image.bin" and open it with a binary file editor, for windows I always use HxD (https://mh-nexus.de/en/hxd/).  
![image](https://user-images.githubusercontent.com/44338633/126884393-70d34a5c-f9c2-474e-8b83-9ae23c866b12.png)  
Blablabla




