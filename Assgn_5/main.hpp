#ifndef MAIN_HPP
#define MAIN_HPP
#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>


#define CLEANING 1
#define CLEANING_DONE 2
#define DIRTY 0


struct Room
{
    int roomID;      // room number
    bool isOccupied; // is occupied or not
    int GuestId;     // guest id if occupied
    int GuestCount;  // number of guest in room till now
    int SpentTime;   // time spent in room
    int room_status; // 0: dirty, 1: cleaning, 2: clean
    Room(int roomID)
    {
        this->roomID = roomID;
        GuestCount = 0;
        isOccupied = false;
        SpentTime = 0;
        GuestId = -1;
        room_status = CLEANING_DONE;
    }
};

struct Guest
{
    int guestID;
    int priority;
    Guest(int guestID, int priority)
    {
        this->guestID = guestID;
        this->priority = priority;
    }
};



#endif