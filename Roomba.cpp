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
    
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~PARENB; /* Parity disabled */
    //tty.c_cflag &= ~CSTOPB;  // No stop bit, but we need one
    tty.c_cflag &= ~CSIZE;        /* Mask the character size bits */
    tty.c_cflag |= CS8;           /* CS8 - Selects 8 data bits */
    tty.c_cflag &= ~CRTSCTS;      /* disable hardware flow control */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);           // disable XON XOFF (for transmit and receive)
    //tty.c_cflag |= CRTSCTS;       /* enable hardware flow control */
    
    tty.c_cc[VMIN] = 1;     //min carachters to be read
    tty.c_cc[VTIME] = 0;    //Time to wait for data (tenths of seconds)
    
    //Set the new options for the port...
    tcflush(fd, TCIFLUSH);
    
    if(tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("tcgetattr %s\n", sys_errlist[errno]);
        return false;
 
    }

    return true;
    
}
void Roomba::stop() {
     if (!robotReady()) return;
    sendCommand(this->cmds["PASSIVE"]);
    if(this->eventPid != -1)
        kill(eventPid, SIGTERM);
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

void Roomba::setEvent(char *eventName, void (*f)(char *, int)) {
    
    if (!robotReady()) return;
    
    if (this->eventPid != -1)
        kill(this->eventPid, SIGTERM);
    
    
    sendCommand(this->cmds["STREAM"]);
    int index = 0;
    bool found = false;
    printf("event_name:%s\n", eventName);
    while (strcmp(this->eventInfo[index].event,"")  != 0 && index < MAX_EVENTS ) {
        if (!strcmp(this->eventInfo[index].event, eventName)) {
            found = true;
            break;
        };
        index++;
    }

    if (found) {
        this->events[(char *) eventName].event = this->eventInfo[index];
        this->events[(char *) eventName].f = f;
    }
    else {
        return;
    }
    
    sendCommand((int) this->events.size());
    
    for (auto v : this->events) {
        sendCommand(v.second.event.packetId);
    }
    
    setEventListener();


}
void Roomba::songPlayingEvent(void (*f)(char *, int)) {
    this->setEvent((char *)"SONG_PLAYING", f);
}
void Roomba::bumpEvent(void (*f)(char *, int)) {
    this->setEvent((char *)"BUMP", f);
}
void Roomba::virtualWallEvent(void (*f)(char *, int)) {
    this->setEvent((char *)"VIRTUAL_WALL", f);
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
void Roomba::sleepMilliSecond(int ms) {
    usleep(ms * 1000);
}
void Roomba::sendCommand(int value) {
    size_t n  = write(this->fd, &value , 1);
    if(n < 1) {
        perror("write");
    }
    sleepMilliSecond(100);
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
    
    sendCommand("START");
    sendCommand("FULL");
    sleepMilliSecond(100);
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
void Roomba::drive(int velocity, int angle) {
    
    if (!robotReady()) return;
    
    int speed1 = velocity & 0xff00;
    int speed2 = velocity & 0x00ff;
    int angle1 = angle & 0xff00;
    int angle2 = angle & 0x00ff;
    sendCommand(this->cmds["DRIVE"]);
    sendCommand(speed1);
    sendCommand(speed2);
    sendCommand(angle1);
    sendCommand(angle2);
    
    
}

void Roomba::setEventListener() {

    if (!robotReady()) return;
    
    int childPid = 0;
    
    if ((childPid  = fork()) == 0) {
        fd_set fd_set, read_set;
        FD_ZERO (&fd_set);
        FD_SET (this->fd, &fd_set);
        int ret;
        if ((ret = select (this->fd + 1, &fd_set, 0,  0,  0)) < 0) {
            perror ("select");
            return;
        }
        read_set = fd_set;
        size_t n;
        int c;
        char buffer[BUFFER_LIMIT];
        int index  = 0;
        
        if (FD_ISSET(this->fd, &fd_set)) {
            while((n = read(this->fd, &c, 1)) > 0) {
                buffer[index] = c;
                if(n == -1) {
                    continue;
                }
                if (buffer[index] == STREAM_HEADER && index > 0 && n > 0) {
                    streamPacket(buffer, index);
                    index = 0;
                }
                else
                    index++;
                if(index >= BUFFER_LIMIT) index = 0;
                
            }

        }
    }
    this->eventPid = childPid;
    return;
}

void Roomba::print(char *buffer, int index) {
    printf("[ ");
    for (int i = 0; i <= index; i++) {
        printf("%d,", buffer[i]);
    }
    
    printf(" ]\n");

}
void Roomba::streamPacket(char *buffer, int index) {
    int i = 0;

    int checksum = 0;
    
    for (int i = 0; i < index; i++) checksum += buffer[i];
   
    
    if (checksum != -1 * STREAM_HEADER) return;

    //print(buffer, index-1);
    int no_of_packets = buffer[0];
    i = 1;
    
    while (no_of_packets > 0  && true) {
        for (auto v : this->events) {
            if (v.second.event.packetId == buffer[i]) {
                for (int k = 0; k < v.second.event.packetLength; k++) {
                    if (v.second.event.eventMask & buffer[i + k + 1]) {
                        v.second.f(v.first, buffer[i+k+1]);                    }
                }

                i += v.second.event.packetLength + 1;
            }
        }
        no_of_packets -= i;
    }
    
    
    return;
}

Roomba::Roomba(char *d, int baudrate) {
    this->isOpen = false;
    this->eventPid = -1;
    initializeCommands();
    printCommands();
    this->fd = open(d, O_RDWR | O_NONBLOCK );
    if (this->fd < 0) {
        printf("Error opening file ... %s\n", sys_errlist[errno]);
        return;
    }
    this->isOpen =  setBaudRate(fd, B115200);
    printf("Robot initialized ... \n");
}
