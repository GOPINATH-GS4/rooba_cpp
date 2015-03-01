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
    
}

void Roomba::bumpSignal(void (*f)(char *)) {
    if (!robotReady()) return;
    
    if (this->eventPid != -1)
        kill(this->eventPid, SIGTERM);
    
    sendCommand(this->cmds["STREAM"]);
    sendCommand(1);
    sendCommand(7);
    setEventListener(f);

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
    //printf("Drive1 %d, Drive 2 %d, angle1 %d, angle2 %d\n", speed1, speed2, angle1, angle2);
    sendCommand(speed1);
    sendCommand(speed2);
    sendCommand(angle1);
    sendCommand(angle2);
    
    
}

void Roomba::setEventListener(void (*f)(char *)) {

    if (!robotReady()) return;
    this->event = f;
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
    
        if (FD_ISSET(this->fd, &fd_set)) {
            while((n = read(this->fd, &c, 1)) > 0) {
             
                if ((c & 0x01) == 0x01) {
                    f((char *)"Bump Right");
                }
                else if ((c & 0x02) == 0x02) {
                    f((char *)"Bump Left");
                }
                else {
                    f((char *) "No bump");
                }
            }

        }
    }
    this->eventPid = childPid;
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
