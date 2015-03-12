#### roomba_cpp, Simple C++ library for Roomba iCreate 2 

Roomba library C++

Files : Roomba.cpp, Roomba.h 

This library is complete and self contained and will run only on Mac, Linux, Raspberry-PI and Beaglebone (should run without any problem, but I have not tested it yet) 

````NOTE: This will not work in Windows```` 

To communicate with iCreate2 you need the FTDI cable like the one shown below 

![Cable](./images/iCreateCable.png)


If you dont have one you can easily hack one, please follow the instructions in these links to make one yourself, ````Note: The serial port in Roomba iCreate2 is a miniDin connector````

[link](http://www.irobot.com/filelibrary/pdfs/hrd/create/Create%20Open%20Interface_v2.pdf)



The main.cpp contains a sample of how to use the library, Right now the events supported are only bump events, I will be making changes to add more events and features

Compiling and running the main.cpp (sample program)

Make sure you change the **"/dev/tty.usbserial-DA017QCF"** to the device attached to your system when you connect your Roomba 


````

	** Compile **
	Roomba master % g++ -c Roomba.cpp 
	Roomba master % g++ -o ./test main.cpp Roomba.o 
	
	** Run **
	./test 
	
	Now you shoule hear first few notes of Vuelie from frozen movie and the robot will spin for 10 seconds 
	
````


Sample code 

````
	
//
//  main.cpp
//  Roomba
//
//  Created by janakiraman gopinath on 2/27/15.
//  Copyright (c) 2015 org.koneksahealth.com. All rights reserved.
//


#include "Roomba.h"

void eventf(char *event, int value) {
    
  
    if(strcmp(event, "BUMP") == 0 ) {
        printf("Event : %s\n", event);
        if (BUMP_RIGHT(value)) printf("Bump Right\n");
        if (BUMP_LEFT(value)) printf("Bump Left\n");
        if (WHEEL_DROP_LEFT(value)) printf("Wheel Drop Left\n");
        if (WHEEL_DROP_RIGHT(value)) printf("Wheel Drop Right\n");
        if (WHEEL_DROP_CASTER(value)) printf("Wheel Drop Caster\n");
    }
    if (strcmp(event, "SONG_PLAYING") == 0) {
        printf("Event : %s\n", event);
        if (SONG_PLAYING(value)) printf("Song playing\n");
    }
    if (strcmp(event, "VIRTUAL_WALL") == 0) {
        if (VIRTUAL_WALL_DETECTED(value)) printf("Virtual wall detected\n");
    }

}



int main(int argc, const char * argv[]) {
    
    Roomba *r = new Roomba((char *) "/dev/tty.usbserial-DA017QCF", B115200);
    
    r->printCommands();
    
    if (r->getStatus())
        cout << "Robot initialized successfully" << endl;
    else
        cout << "Failed to initialize robot" << endl;
    
    int events = BUMP_EVENT | SONG_PLAYING_EVENT | VIRTUAL_WALL_EVENT;
    
    r->setEvents(events, eventf);
    //r->bumpEvent(eventf);

    //r->songPlayingEvent(eventf);
 
    //r->virtualWallEvent(eventf);
    
    // GGGAGG GAABGA
    array<int, 32> songSequence = { 91,32,91,32,91,32,93,32,91,32,91,42,91,32,93,32,93,32,95,32,91,32,93,32 };
   
    // 12 Midi sequences from the array  
    
    r->createSong(0, 12, songSequence);
    r->playSong(0);
    
    r->spin(1); // -1 for clockwise spin
    //r->drive(500, 0); // Velocity and angle
    //r->driveDirect(100, -100);
    
    //r->bumpEvent(eventf);
    sleep(10);

    r->stop(); // Finally stop the robot
    
    return 0;
}

````