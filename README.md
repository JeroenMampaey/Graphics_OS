# About
This is an OS made for fun. I mainly used information from the OSdev wiki to make this aswell as from https://github.com/cfenollosa/os-tutorial. I use 640x480 VGA mode with 4-bit colors, I provide multicore support for 3D rendering, provide a disk driver for an IDE controller using ATA PIO mode and I make use of an Intel 82540EM network card. Though the source code is in my opinion very messy, I am happy with the result.
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
Make sure you save all the changes in the settings and also remember the name of the drive for the IDE controller, in my case it was "Test_1.vdi" because we now will have to put our disk image into this file. First we will need to compile the entire OS, to do this you will need a linux terminal to execute the Makefile that is included here. I have only ever tested the Makefile on the Ubuntu 18.04 LTS for windows which you can get via the Microsoft Store these days it appears. Executing the make command (navigating to the directory with the Makefile from this github and then typing "make" in the terminal) should give something like:  
![image](https://user-images.githubusercontent.com/44338633/126884304-6005496c-276b-4443-9212-618b9f5b1c40.png)  
Use a file explorer to find the disk image called "os_image.bin" and open it with a binary file editor, for windows I always use HxD (https://mh-nexus.de/en/hxd/).  
![image](https://user-images.githubusercontent.com/44338633/126884393-70d34a5c-f9c2-474e-8b83-9ae23c866b12.png)  
Now while keeping the "os_image.bin" file open, go back to Virtualbox and select the machine that you just made, then at the top click "Machine" and next click "Show in Explorer". Now open the vdi file from the IDE Primary Master with a binary editor.  
![image](https://user-images.githubusercontent.com/44338633/126884505-bc810d74-c957-4818-a8bc-239f8ddce219.png)  
The bytes at 0x158, 0x159, 0x15A and 0x15B indicate where the disk actually begins, it represents an hexadecimal address, in my case it says "00002000" and since it is written in little endian this refers to address 0x00200000. Copy all the binary data from "os_image.bin" and paste it in the vdi file starting at the address you just found (there are probably some more efficiënt methods to craft a vdi file but this is the way that I have always done it and it doesn't take to long). When you have done this (and saved it), you can start up the machine in virtualbox and a screen should show looking like this:  
![image](https://user-images.githubusercontent.com/44338633/126898339-f9becd71-f78e-4395-aec8-8b8c512c6637.png)  
In that case, you have done everything correctly.  
# What can you do with it?
In the terminal, you can use some basic commands:
- "CLEAR": clears the terminal
- "VENDOR ID": shows the vendor id of your processor
- "TEST MULTICORE": test every core that is used by the OS
- "GET IP": uses DHCP to get a local IP-address for the machine
- "NETWORK INFO": shows the MAC- and IP-address of the machine, also shows the IP-address of the router
- "RUN DEMO GRAPHICS": alows the user to walk around in a demo 3D space

As far as networking is concerned, at the moment, your machine should answer to ARP requests and ICMP echo requests (ping requests) with correct replies. 
  
The OS is centered around allowing users to make their own 3D environment that you can walk around in similar to the "RUN DEMO GRAPHICS" command. To achieve this, the OS has an editor which you can start by using the "WRITE" command. In this editor, you can tell the OS to place a triangle somewhere by typing "T(x0, y0, z0, x1, y1, z1, x2, y2, z2);" where (xn, yn, zn) represent the n'th point of the triangle. The editor is not supposed to contain more than 2080 lines and the OS is made so that it is possible to render up to 2080 traingles. Leaving the editor is possible by pressing the ESC-key and you can view your 3D environment by typing the "RUN GRAPHICS" command (and you can leave this one also by pressing the ESC-key). 



