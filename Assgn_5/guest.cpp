#include "guest.hpp"

using namespace std;

extern int X, Y, N;                     // X: # cleaning staff, Y: # Guest, N: # Room
extern pthread_t *guestThreads;         // guest threads
extern pthread_t *cleaningStaffThreads; // cleaning staff threads

extern bool CLEANING_UNDER_PROCESS;
extern int count_of_cleaning; // count of cleaning

extern int *guestNo;
extern int *cleaningStaffNo;
extern Room *rooms;
extern Guest *guests;
extern sem_t sem_room_ptr, sem_terminal, sem_cleaning, sem_room_to_clean;
extern sem_t *sem_guests, *sem_cleaningStaff, *sem_room;
extern vector<int> rooms_to_clean;
extern vector<int> available_0, available_1;
extern set<pair<int, pair<int, int>>> occupied;

int request(int guestId, int time)
{
    // find the guest
    Guest *guest = &guests[guestId];
    // find the empty room
    Room *room = NULL;
    sem_wait(&sem_room_ptr); // wait for rooms pointer

    if (available_0.size() > 0)
    { 
        /* 
            first try to get a fresh room (the room which is not occupied by any guest after cleaning)
        */
        room = &rooms[available_0[0]]; 
        swap(available_0[0], available_0[available_0.size() - 1]);
        available_0.pop_back();
        int priority = guest->priority;
        occupied.insert({priority, {guestId, room->roomID}});
        sem_wait(&sem_room[room->roomID]);
        room->GuestId = guestId;
        room->GuestCount++;
        room->isOccupied = true;
        room->SpentTime+=time;
        sem_post(&sem_room[room->roomID]);
        sem_post(&sem_room_ptr); // release rooms pointer
        return room->roomID;
    }
    if (available_1.size() > 0)
    {
        /* 
            if there is no fresh room, then try to get a room which is occupied by one guest
        */
        room = &rooms[available_1[0]];
        swap(available_1[0], available_1[available_1.size() - 1]);
        available_1.pop_back();
        sem_wait(&sem_room[room->roomID]);
        room->GuestId = guestId;
        room->GuestCount++;
        room->isOccupied = true;
        room->SpentTime+=time;
        room->room_status = DIRTY;
        sem_wait(&sem_room_to_clean);
        rooms_to_clean.push_back(room->roomID);
        sem_post(&sem_room_to_clean);
        sem_post(&sem_room[room->roomID]);
        sem_post(&sem_room_ptr); // release rooms pointer
        return room->roomID;
    }

    if (occupied.size() > 0)
    {
        /* 
            if there is no fresh room and no room which is occupied by one guest, then try to get a room which is occupied by two guests by kicking out one of them
        */
        int priority = occupied.begin()->first;
        int roomNo = occupied.begin()->second.second;
        int removeGuestId = occupied.begin()->second.first;
        if (priority < guest->priority)
        {
            sem_post(&sem_guests[removeGuestId]);
            occupied.erase(occupied.begin());
            sem_wait(&sem_room[roomNo]);
            rooms[roomNo].GuestId = guestId;
            rooms[roomNo].GuestCount++;
            rooms[roomNo].SpentTime+=time;
            rooms[roomNo].isOccupied = true;
            int num = rooms[roomNo].roomID;
            rooms[num].room_status = DIRTY;
            sem_wait(&sem_room_to_clean);
            rooms_to_clean.push_back(num);
            sem_post(&sem_room_to_clean);
            sem_post(&sem_room[roomNo]);
            sem_wait(&sem_terminal);
            cout << "Guest " << guestId << " kicked out Guest " << removeGuestId << " from room " << roomNo << endl;
            sem_post(&sem_terminal);
            sem_post(&sem_room_ptr); // release rooms pointer
            return num;
        }
    }
    sem_post(&sem_room_ptr); // release rooms pointer
    return -1; // no room available
}

void *guestThread(void *arg)
{
    // signal(SIGALRM, signal_handler);
    int guestId = *(int *)arg;
    while (true)
    {
        // Sleeps for a random time between 10-20 seconds
        sleep((rand() % 11) + 10);
        sem_wait(&sem_cleaning);
        if (CLEANING_UNDER_PROCESS)
        {
            // if cleaning is under process, then wait for cleaning to complete
            sem_post(&sem_cleaning);
            continue;
        }
        sem_post(&sem_cleaning);
        int temp;
        while (true)
        {
            // reset the semaphore
            sem_getvalue(&sem_guests[guestId], &temp);
            if (temp == 0)
                break;
            sem_wait(&sem_guests[guestId]);
        }

        // requests for a room in the hotel for a time to stay between 10-30 seconds.
        int stayTime = (rand() % 21) + 10;
        int RoomNo = request(guestId, stayTime); // request for a room
        if (RoomNo == -1)
            continue;
        sem_wait(&sem_terminal);
        cout << "Guest " << guestId << " entered room " << RoomNo << " for " << stayTime << " seconds" << endl;
        sem_post(&sem_terminal);
        sem_wait(&sem_room_to_clean);
        if ((int)rooms_to_clean.size() == N)
        {
            sem_post(&sem_room_to_clean);
            sem_wait(&sem_terminal);
            cout << "--------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
            cout << "Cleaning is required" << endl;
            cout << "--------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
            sem_post(&sem_terminal);
            // send signal to all guests to leave
            sem_wait(&sem_cleaning);
            CLEANING_UNDER_PROCESS = true;
            sem_post(&sem_cleaning);

            for (int i = 0; i < N; i++)
            {
                if (rooms[i].GuestId != -1)
                {
                    sem_post(&sem_guests[rooms[i].GuestId]);
                }
            }

            for (int i = 0; i < X; i++)
            {
                sem_post(&sem_cleaningStaff[i]);
                sem_wait(&sem_terminal);
                cout << "Cleaning staff " << i << " woke up" << endl;
                sem_post(&sem_terminal);
            }
        }
        else
        {
            sem_post(&sem_room_to_clean);
        }
        // wait using sem_timedwait for the time to stay
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += stayTime;
        int ret = sem_timedwait(&sem_guests[guestId], &timeout); // wait for the time to stay
        if (ret == 0)
        {
            sem_wait(&sem_terminal);
            cout << "Guest " << guestId << " left room " << RoomNo << " due to interruption" << endl;
            sem_post(&sem_terminal);
        }
        // check if sem_timedwait returned due to timeout
        else if (ret == -1 && errno == ETIMEDOUT)
        {
            sem_wait(&sem_terminal);
            cout << "Guest " << guestId << " left room " << RoomNo << " due to timeout" << endl;
            sem_post(&sem_terminal);
            sem_wait(&sem_room_ptr); // wait for rooms pointer
            sem_wait(&sem_room[RoomNo]);
            rooms[RoomNo].isOccupied = false;
            rooms[RoomNo].GuestId = -1;
            if (rooms[RoomNo].GuestCount == 1)
            {
                /* 
                    if the room was occupied by one guest, then put it in available_1 (rooms which were used by one guest)
                */
                available_1.push_back(RoomNo);
                int priority = guests[guestId].priority;
                occupied.erase(make_pair(priority, make_pair(guestId, RoomNo)));
            }
            sem_post(&sem_room[RoomNo]);
            sem_post(&sem_room_ptr); // release rooms pointer
        }
        else
        {
            // someone kicked out the guest
            sem_wait(&sem_terminal);
            cout << "Error in guest " << guestId << endl;
            sem_post(&sem_terminal);
            exit(0);
        }
    }
}