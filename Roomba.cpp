//
//  Roomba.cpp
//  Roomba
//
//  Created by janakiraman gopinath on 2/27/15.
//  Copyright (c) 2015 org.koneksahealth.com. All rights reserved.
//

#include "Roomba.h"

bool Roomba::setBaudRate(int fd, int speed) {
    struct termios tty;
    
    memset (&tty, 0, sizeof tty);
    
    if (tcgetattr (fd, &tty) != 0) {
        printf("tcgetattr %s\n", sys_errlist[errno]);
        return false;
    }

    
    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);
    
    /* Needed for ROOMBA Serial communication from create open interface documentation
     Data bits: 8
     Parity: None
     Stop bits: 1
     Flow control: None
    */
    tty.c_cflag |= (CLOCAL | CREAD | CS8);
    tty.c_cflag &= ~(PARENB | CSTOPB);
    tty.c_cflag &= ~CRTSCTS; 
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);           // disable XON XOFF (for transmit and receive)
    tty.c_cflag &= ~CSIZE;
    /* tty.c_cflag &= ~CSIZE;         Mask the character size bits     
     tty.c_cflag &= ~CRTSCTS;     disable hardware flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);           // disable XON XOFF (for transmit and receive)
    //tty.c_cflag |= CRTSCTS;       enable hardware flow control */
    
    tty.c_cc[VMIN] = 1;     //min carachters to be read
    tty.c_cc[VTIME] = 0;    //Time to wait for data (tenths of seconds)
    
    //Set the new options for the port...
    
    if(tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("tcgetattr %s\n", sys_errlist[errno]);
        return false;
 
    }

    return true;
    
}
void Roomba::printCommands() {
  
    for (auto keyValue : this->cmds) {
        printf("command[%s]=%d\n", keyValue.first.c_str(), keyValue.second);
    }
}

void Roomba::initializeCommands() {
    
    this->cmds["START"] = 128;
    this->cmds["SAFE"] = 131;
    this->cmds["FULL"] = 132;
    this->cmds["SONG"] = 140;
    this->cmds["PLAY"] = 141;
    this->cmds["DRIVE"] = 137;
    this->cmds["DRIVE_DIRECT"] = 137;
    this->cmds["PASSIVE"] = 128;
    this->cmds["STREAM"] = 148;
    
    this->eventInfo[0].event = (char *)"BUMP";
    this->eventInfo[0].packetId = 7;
    this->eventInfo[0].packetLength = 1;
    this->eventInfo[0].eventMask = 0x1f;
    
    this->eventInfo[1].event = (char *)"SONG_SELECTED";
    this->eventInfo[1].packetId = 36;
    this->eventInfo[1].packetLength = 1;
    this->eventInfo[1].eventMask = 0xff;

    this->eventInfo[2].event = (char *)"SONG_PLAYING";
    this->eventInfo[2].packetId = 37;
    this->eventInfo[2].packetLength = 1;
    this->eventInfo[2].eventMask = 0xff;
    
    this->eventInfo[3].event = (char *)"VIRTUAL_WALL";
    this->eventInfo[3].packetId = 13;
    this->eventInfo[3].packetLength = 1;
    this->eventInfo[3].eventMask = 0xff;

    
}
void Roomba::destroyThread() {
   
    // Destroy Thread
    if (this->threadRunning) {
        this->finishThread = true;
        while (this->finishThread) sleepMilliSecond(200);
        this->threadRunning = false;
    }

}
void Roomba::setEvent(char *events[], int total_events, void (*f)(char *, int)) {
    
    if (!robotReady()) return;
    
    if (this->threadRunning) {
        printf("Cannot intialize events in the middle of a stream");
        return;
    }
    
    int packetIds[MAX_PACKETS];
    int packet = 0;
    for (int i = 0; i < total_events; i++) {
        
        int index = 0;
        bool found = false;
        while (strcmp(this->eventInfo[index].event,"")  != 0 && index < MAX_EVENTS ) {
            if (!strcmp(this->eventInfo[index].event, events[i])) {
                found = true;
                break;
            };
            index++;
        }
        
        if (found) {
            this->events[events[i]].event = this->eventInfo[index];
            this->events[events[i]].f = f;
        }
        else continue;
        printf("Found event . %s\n", events[i]);
        //sendCommand((int) this->events.size());
        
        packetIds[packet] = this->eventInfo[index].packetId;
        packet++;
        for (auto v : this->events) {
           // packetIds[packet] = v.second.event.packetId;
           
           // sendCommand(v.second.event.packetId);
        }

    }

    sendCommand("START");
    sendCommand("FULL");
    sendCommand(this->cmds["STREAM"]);
    sendCommand(packet);
    for (int i = 0; i < packet; i++) {
        sendCommand(packetIds[i]);
    }
    
    this->threadRunning = true;
    this->th = thread(setEventListener, this);
    this->th.detach();
}

void Roomba::setEvents(int events, void (*f)(char *, int)) {
    
    char *ev[MAX_EVENTS];
    int i = 0;
    
    if ((events & BUMP_EVENT) == BUMP_EVENT) {
        ev[i] = (char *) "BUMP";
        i++;
    }
    if ((events & SONG_PLAYING_EVENT) == SONG_PLAYING_EVENT) {
        ev[i] = (char *) "SONG_PLAYING";
        i++;
    }
    if ((events & VIRTUAL_WALL_EVENT) == VIRTUAL_WALL_EVENT) {
        ev[i] = (char *) "VIRTUAL_WALL";
        i++;
    }
    setEvent(ev, i, f);
    
}
void Roomba::songPlayingEvent(void (*f)(char *, int)) {

    char *events[MAX_EVENTS];
    events[0] = (char *) "SONG_PLAYING";
    this->setEvent(events, 1,  f);

}
void Roomba::bumpEvent(void (*f)(char *, int)) {

    char *events[MAX_EVENTS];
    events[0] = (char *) "BUMP";
    this->setEvent(events, 1,  f);

}
void Roomba::virtualWallEvent(void (*f)(char *, int)) {
   
    char *events[MAX_EVENTS];
    events[0] = (char *) "VIRTUAL_WALL";
    this->setEvent(events, 1,  f);
}
void Roomba::stop() {
    if (!robotReady()) return;
    sendCommand(this->cmds["PASSIVE"]);
    
    destroyThread();
   
}
void Roomba::spin(int direction) {
    sendCommand("DRIVE");
    sendCommand(255);
    sendCommand(56);
    if (direction > 0) {
        sendCommand(0xff);
        sendCommand(0xff);
    } else {
        sendCommand(0x00);
        sendCommand(0x01);
    }
    
}
void Roomba::drive(int velocity, int angle) {
    
    if (!robotReady()) return;
    
    int speed1 = velocity & 0xff00;
    int speed2 = velocity & 0x00ff;
    int angle1 = angle & 0xff00;
    int angle2 = angle & 0x00ff;
    sendCommand("DRIVE");
    sendCommand(speed1);
    sendCommand(speed2);
    sendCommand(angle1);
    sendCommand(angle2);
    
    
}
void Roomba::driveDirect(int rightWheelSpeed, int leftWheelSpeed) {
    
    if (!robotReady()) return;
    
    int rV1 = rightWheelSpeed & 0xff00;
    int rV2 = rightWheelSpeed & 0x00ff;
    int lV1 = leftWheelSpeed & 0xff00;
    int lV2 = leftWheelSpeed & 0x00ff;
    sendCommand(this->cmds["DRIVE_DIRECT"]);
    sendCommand(rV1);
    sendCommand(rV2);
    sendCommand(lV1);
    sendCommand(lV2);
    
}


bool Roomba::getStatus() {
    return this->isOpen;
}

void Roomba::sendCommand(string cmd) {
    sendCommand(this->cmds[cmd]);
}
void Roomba::sendCommand(string cmd, int value) {
    sendCommand(this->cmds[cmd]);
    sendCommand(value);
}
void Roomba::sendCommand(int value) {
    size_t n  = write(this->fd, &value , 1);
    if(n < 1) {
        perror("write");
    }
    sleepMilliSecond(100);
    printf("-[%d]-\n", value);
   
}
void Roomba::sleepMilliSecond(int ms) {
    usleep(ms * 1000);
}
bool Roomba::robotReady() {
    if (!this->getStatus()) {
        printf("Robot not ready ... \n");
        return false;
    }
    return true;
}
void Roomba::createSong(int songNumber,  int midiLength, array<int, 32> midiSequence) {
    
    if (!robotReady()) return;
    
    sendCommand("SONG");
    sendCommand(songNumber);
    sendCommand(midiLength);
    
    for (int i = 0; i < midiLength * 2; i++) {
        sendCommand(midiSequence.at(i));
    }
    sleepMilliSecond(100);
}
void Roomba::playSong(int songNumber) {
    
    if (!robotReady()) return;
    
    sendCommand("PLAY");
    sendCommand(songNumber);
}

void Roomba::setEventListener(Roomba *r) {

    if (!r->robotReady()) return;
    
    r->threadRunning = true;
    fd_set fdset, read_set;
    FD_ZERO (&fdset);
    FD_SET (r->fd, &fdset);
    int ret;
    
    struct timeval tm;
    tm.tv_sec = 1;
    tm.tv_usec = 0;
    
    while (true) {
        if(r->debug) printf("waitOnEvent() ...... \n");
        if ((ret = select (r->fd + 1, &fdset, 0,  0,  &tm)) < 0) {
            perror ("select");
            return;
        }
        if(ret == 0) { 
          if(r->debug) printf("Select timeout ...\n");
          continue;
        }
        
        if(r->debug) printf("event() ......\n");
        read_set = fdset;
        size_t n;
        int c;
        int buffer[BUFFER_LIMIT];
        int index  = 0;
        
        if (FD_ISSET(r->fd, &fdset)) {
            while((n = read(r->fd, &c, 1)) > 0) {
                if(r->finishThread) {
                    printf("Asked to exit now ...\n");
                    r->finishThread = false;
                    r->th.~thread();
                    return;
                }
                if(n == -1) {
                    r->streamPacket(buffer, index);
                    break;
                }
                buffer[index] = c;
                if (buffer[index] == STREAM_HEADER && index > 0 && n > 0) {
                    r->streamPacket(buffer, index);
                    index = 0;
                }
                else
                    index++;
                if(index >= BUFFER_LIMIT) index = 0;
                
            }
            
        }

    }
    
    return;
}

void Roomba::print(int buffer[], int index, int checksum) {
    printf("[ ");
    for (int i = 0; i <= index; i++) {
        printf("%d,", buffer[i]);
    }
    
    printf("checksum:%d]\n", checksum);

}
void Roomba::streamPacket(int buffer[], int index) {
    int i = 0;

    int checksum = 0;
    
    for (int i = 0; i < index; i++) checksum += buffer[i];
   
    
    //print(buffer, index-1, checksum);
    
    if (checksum != 256) return;
    int no_of_packets = buffer[1];
    
    i = 2;
    
    while (no_of_packets > 0  && true) {
        for (auto v : this->events) {
            if (v.second.event.packetId == buffer[i]) {
                for (int k = 0; k < v.second.event.packetLength; k++) {
                    if (v.second.event.eventMask & buffer[i + k + 1]) {
                        printf("event(1)......\n");
                        v.second.f(v.first, buffer[i+k+1]);                    }
                }

                i += v.second.event.packetLength + 1;
            }
        }
        no_of_packets -= i;
    }
    
    
    return;
}

void Roomba::setDebug(bool debug) {
  this->debug = debug;
}

Roomba::Roomba(char *d, int baudrate) {
    this->isOpen = false;
    this->debug = false; 
    this->threadRunning = false;
    this->finishThread = false;
    
    initializeCommands();
    printCommands();
    this->fd = open(d, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (this->fd < 0) {
        printf("Error opening file ... %s\n", sys_errlist[errno]);
        return;
    }
    this->isOpen =  setBaudRate(fd, B115200);
    printf("Robot initialized ... \n");
    sendCommand("START");
    sendCommand("FULL");
}
