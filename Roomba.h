//
//  Roomba.h
//  Roomba
//
//  Created by janakiraman gopinath on 2/27/15.
//  Copyright (c) 2015 org.koneksahealth.com. All rights reserved.
//

#ifndef __Roomba__Roomba__
#define __Roomba__Roomba__

#include <stdio.h>
#include <unordered_map>
#include <array>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <iostream>
#include <stdlib.h>

#define MAX_EVENTS 100
#endif /* defined(__Roomba__Roomba__) */
using namespace std;

#define STREAM_HEADER 19 
#define BUFFER_LIMIT 1024

#define BUMP_RIGHT(value)               ((value & 0x01) == 0x01)
#define BUMP_LEFT(value)                ((value & 0x02) == 0x02)
#define WHEEL_DROP_RIGHT(value)         ((value & 0x04) == 0x04)
#define WHEEL_DROP_LEFT(value)          ((value & 0x08) == 0x08)
#define WHEEL_DROP_CASTER(value)        ((value & 0x10) == 0x10)

#define SONG_PLAYING(value)             ((value & 0x01) == 0x01) 

typedef  struct EVENTS_INFO {
    char *event;
    int packetId;
    int packetLength;
    int eventMask;
} EVENT_INFO;

typedef struct EVENTS {
    EVENT_INFO event;
    void (*f)(char *event, int value);
} EVENTS;

typedef struct PACKET {
    int packetId;
    int value;
} PACKET;

class Roomba {
    
   
private:
    unordered_map <string, int> cmds;
    int fd;
    bool isOpen;
    void (*event)(char *);
    int eventPid;
    
    unordered_map<char *, EVENTS > events;
    
    EVENT_INFO eventInfo[MAX_EVENTS];
    
    void initializeCommands();
    void sendCommand(string cmd);
    void sendCommand(int cmd);
    void sendCommand(string cmd, int value);
    static bool setBaudRate(int fd, int speed);
    static void sleepMilliSecond(int ms);
    void streamPacket(char *buffer, int index);
    void print(char *buffer, int index);
    void setEventListener();
    bool robotReady();
    void setEvent(char *eventName, void (*f)(char *, int));
    
public:

    Roomba(char *device, int baudRate);
    void createSong(int songNumber, int midiLengh, array<int,32> midiSequence);
    void playSong(int songNumber);
    bool getStatus();
    void printCommands();
    void spin(int direction);
    void stop();
    void drive(int velocity, int angle);
    void bumpEvent(void (*f)(char *, int));
    void songPlayingEvent(void (*f)(char *, int));
};
