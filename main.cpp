//
//  main.cpp
//  Roomba
//
//  Created by janakiraman gopinath on 2/27/15.
//  Copyright (c) 2015 org.koneksahealth.com. All rights reserved.
//

#include <iostream>
#include "Roomba.h"

void eventf(char *buf) {
    printf("%s\n", buf);
}
int main(int argc, const char * argv[]) {
    
    Roomba *r = new Roomba((char *) "/dev/tty.usbserial-DA017QCF", B115200);
    
    r->printCommands();
    
    if (r->getStatus())
        cout << "Robot initialized successfully" << endl;
    else
        cout << "Failed to initialize robot" << endl;
    
    r->bumpSignal(eventf);
    array<int, 32> songSequence;
    
    // GGGAGG GAABGA
    //G
    songSequence[0] = 91;
    songSequence[1] = 32;
    //G
    songSequence[2] = 91;
    songSequence[3] = 32;
    //G
    songSequence[4] = 91;
    songSequence[5] = 32;
    //A
    songSequence[6] = 93;
    songSequence[7] = 32;
    //G
    songSequence[8] = 91;
    songSequence[9] = 32;
    //G
    songSequence[10] = 91;
    songSequence[11] = 48;
    // G
    songSequence[12] = 91;
    songSequence[13] = 32;
    //A
    songSequence[14] = 93;
    songSequence[15] = 32;
    //A
    songSequence[16] = 93;
    songSequence[17] = 32;
    //B
    songSequence[18] = 95;
    songSequence[19] = 32;
    //G
    songSequence[20] = 91;
    songSequence[21] = 32;
    //A
    songSequence[22] = 93;
    songSequence[23] = 32;
   
    // 12 Midi sequences from the array  
    r->createSong(0, 12, songSequence);
    r->playSong(0);
    r->spin(1); // -1 for clockwise spin
    sleep(10);
    //r->drive(500, 0); // Velocity and angle
    r->stop(); // Finally stop the robot
    
    return 0;
}
