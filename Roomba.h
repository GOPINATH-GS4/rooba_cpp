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
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

#include <thread>

#define MAX_EVENTS 100
#define MAX_PACKETS 100
#endif /* defined(__Roomba__Roomba__) */
using namespace std;

#define STREAM_HEADER 19 
#define BUFFER_LIMIT 1024

#define BUMP_EVENT                   0x01
#define VIRTUAL_WALL_EVENT           0x02
#define SONG_PLAYING_EVENT           0x04
#define CLIFF_LEFT_EVENT             0x08
#define CLIFF_RIGHT_EVENT            0x10
#define CLIFF_FRONT_LEFT_EVENT       0x20
#define CLIFF_FRONT_RIGHT_EVENT      0x40

#define BUMP_RIGHT(value)                 ((value & 0x01) == 0x01)
#define BUMP_LEFT(value)                  ((value & 0x02) == 0x02)
#define WHEEL_DROP_RIGHT(value)           ((value & 0x04) == 0x04)
#define WHEEL_DROP_LEFT(value)            ((value & 0x08) == 0x08)
#define WHEEL_DROP_CASTER(value)          ((value & 0x10) == 0x10)

#define SONG_PLAYING(value)               ((value & 0x01) == 0x01)

#define VIRTUAL_WALL_DETECTED(value)      ((value & 0x01) == 0x01)

#define CLIFF_LEFT_DETECTED(value)        ((value & 0x01) == 0x01)
#define CLIFF_RIGHT_DETECTED(value)       ((value & 0x01) == 0x01)
#define CLIFF_FRONT_LEFT_DETECTED(value)  ((value & 0x01) == 0x01) 
#define CLIFF_FRONT_RIGHT_DETECTED(value) ((value & 0x01) == 0x01)

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
    thread th;
    
    bool threadRunning;
    bool finishThread;
    bool debug;
    
    unordered_map<char *, EVENTS > events;
    
    EVENT_INFO eventInfo[MAX_EVENTS];
    
    void initializeCommands();
    void sendCommand(string cmd);
    void sendCommand(int cmd);
    void sendCommand(string cmd, int value);
    static bool setBaudRate(int fd, int speed);
    static void sleepMilliSecond(int ms);
    void streamPacket(int buffer[], int index);
    void print(int buffer[], int index, int checksum);
    static void setEventListener(Roomba *r);
    bool robotReady();
    void destroyThread();
    void setEvent(char *events[], int total_events, void (*f)(char *, int));
    void print(int buffer[], int index, int cmd[], int cmd_index);
    void resetStreamHead(int buffer[], int *index, int command[], int *cmd_index);
   
public:

    Roomba(char *device, int baudRate);
    void createSong(int songNumber, int midiLengh, array<int,32> midiSequence);
    void playSong(int songNumber);
    bool getStatus();
    void printCommands();
    void spin(int direction);
    void stop();
    void drive(int velocity, int angle);
    void driveDirect(int rightWheelSpeed, int leftWheelSpeed);
    void bumpEvent(void (*f)(char *, int));
    void songPlayingEvent(void (*f)(char *, int));
    void virtualWallEvent(void (*f)(char *, int));
    void cliffEvent(void (*f)(char *, int));
    void setEvents(int events, void (*f)(char *, int));
    void setDebug(bool debug);
};
