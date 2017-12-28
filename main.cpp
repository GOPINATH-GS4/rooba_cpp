//
//  main.cpp
//  Roomba
//
//  Created by janakiraman gopinath on 2/27/15.
//  Copyright (c) 2015 org.koneksahealth.com. All rights reserved.
//


#include "Roomba.h"

Roomba *r1;

void generalEvent(int event, int value) {
    std::cout << (char) event << ":" << value << std::endl;
    switch(event) {
        case CALLIBRATION_HEADER:
            return;
        case ANGLE_HEADER:
            return;
        case SPEED_HEADER:
            r1->spin(Roomba::CLOCKWISE, value);
            return;
        case DONE_HEADER:
            r1->stop();
            return;

    }


}
void eventf(char *event, int value) {
    

    if(strcmp(event, "BUMP") == 0 ) {
        //printf("Event : %s\n", event);
        if (BUMP_RIGHT(value)) printf("Bump Right\n");
        if (BUMP_LEFT(value)) printf("Bump Left\n");
        if (WHEEL_DROP_LEFT(value)) printf("Wheel Drop Left\n");
        if (WHEEL_DROP_RIGHT(value)) printf("Wheel Drop Right\n");
        if (WHEEL_DROP_CASTER(value)) printf("Wheel Drop Caster\n");
    }
    if (strcmp(event, "SONG_PLAYING") == 0) {
        //printf("Event : %s\n", event);
        if (SONG_PLAYING(value)) printf("Song playing\n");
    }
    if (strcmp(event, "VIRTUAL_WALL") == 0) {
        //printf("Event : %s\n", event);
        if (VIRTUAL_WALL_DETECTED(value)) printf("Virtual wall detected\n");
    }
    if (strncmp(event, "CLIFF", 5) == 0) {
    	  //printf("Event : %s\n", event);    
        if(CLIFF_LEFT_DETECTED(value)) printf("Cliff left detected\n");
        if(CLIFF_FRONT_LEFT_DETECTED(value)) printf("Cliff front left detected\n");
        if(CLIFF_RIGHT_DETECTED(value)) printf("Cliff right detected\n");
        if(CLIFF_FRONT_RIGHT_DETECTED(value)) printf("Cliff front right detected\n");
    }

}



int main(int argc, const char * argv[]) {


    //Roomba *r = new Roomba((char *) "/dev/tty.usbserial-DA017QCF", B115200);
    Roomba *r2 = new Roomba((char *) "/dev/cu.usbmodem1411", B9600, true);
    //r1 = new Roomba((char *) "/dev/ttyUSB0", B115200);


    //r2->setDebug(true);
    //r1->printCommands();
    //r2->setGeneralEvent(generalEvent);
    
    if (r1->getStatus())
        std::cout << "Robot initialized successfully" << std::endl;
    else
        std::cout << "Failed to initialize robot" << std::endl;
    
    //int events = BUMP_EVENT | SONG_PLAYING_EVENT | VIRTUAL_WALL_EVENT;
    
    //r1->setEvents(events, eventf);
    //r->bumpEvent(eventf);

    //r->songPlayingEvent(eventf);
 
    //r->virtualWallEvent(eventf);
    
    // GGGAGG GAABGA
    //array<int, 32> songSequence = { 91,32,91,32,91,32,93,32,91,32,91,42,91,32,93,32,93,32,95,32,91,32,93,32 };
   
    // 12 Midi sequences from the array  
    
    //r->createSong(0, 12, songSequence);
    //r->playSong(0);

    r1->spin(Roomba::CLOCKWISE, -100);
    /*
    for (int speed = 0; speed < 256; speed += 5) {
        r->spin(Roomba::CLOCKWISE, speed);
        sleep(3);
    }
     */
    //r->drive(500, 0); // Velocity and angle
    //r->driveDirect(100, -100);
    
    //r->bumpEvent(eventf);
    sleep(5);


    while(1) {
        usleep(500);
    }
    r1->stop(); // Finally stop the robot
}
