#include "cleaning.hpp"

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
// extern int *room_status;
extern vector<int> rooms_to_clean;
extern vector<int> available_0, available_1;
extern set<pair<int, pair<int, int>>> occupied;

void *cleaningStaffThread(void *arg)
{
    // signal(SIGUSR2, signal_handler);
    int cleaningStaffId = *(int *)arg;
    while (true)
    {
        sem_wait(&sem_cleaningStaff[cleaningStaffId]);
        // check if cleaning is going on
        while (1)
        {
            sem_wait(&sem_cleaning);
            if (!CLEANING_UNDER_PROCESS)
            { // if cleaning is not going on
                sem_post(&sem_cleaning);
                break;
            }
            sem_post(&sem_cleaning);
            // choose a random room to clean
            sem_wait(&sem_room_to_clean);
            if(rooms_to_clean.size() == 0){
                sem_post(&sem_room_to_clean);
                continue;
            }
            int idx = rand() % rooms_to_clean.size();
            int roomNo = rooms_to_clean[idx];
            swap(rooms_to_clean[idx], rooms_to_clean[rooms_to_clean.size() - 1]);
            rooms_to_clean.pop_back();
            sem_post(&sem_room_to_clean);
            sem_wait(&sem_room[roomNo]);
            if (rooms[roomNo].room_status == CLEANING || rooms[roomNo].room_status == CLEANING_DONE)
            {
                sem_post(&sem_room[roomNo]);
                continue;
            }
            if (rooms[roomNo].GuestCount > 1)
            {
                rooms[roomNo].room_status = CLEANING; // set room status to cleaning
                
                // sleep for time spentTime / DELTA
                int cleaningTime = rooms[roomNo].SpentTime / DELTA;
                sleep(cleaningTime);
                // reset room status
                rooms[roomNo].GuestId = -1;
                rooms[roomNo].GuestCount = 0;
                rooms[roomNo].SpentTime = 0;
                rooms[roomNo].isOccupied = false;
                rooms[roomNo].room_status = CLEANING_DONE;
                sem_wait(&sem_terminal);
                cout << "Cleaning staff " << cleaningStaffId << " cleaned room " << roomNo << endl;
                sem_post(&sem_terminal);
                sem_wait(&sem_cleaning);
                count_of_cleaning++;
                sem_post(&sem_cleaning);
                sem_post(&sem_room[roomNo]);
            }
            // if all rooms are cleaned then reset the cleaning flag
            sem_wait(&sem_cleaning);
            if (count_of_cleaning == N)
            {
                CLEANING_UNDER_PROCESS = false;
                count_of_cleaning = 0;
                sem_wait(&sem_room_ptr);
                available_0.clear();
                available_1.clear();
                for (int i = 0; i < N; i++)
                {
                    available_0.push_back(i);
                }
                occupied.clear();
                rooms_to_clean.clear();
                sem_post(&sem_room_ptr);
                sem_post(&sem_cleaning);
            }else{
                sem_post(&sem_cleaning);
            }
        }
    }
}
