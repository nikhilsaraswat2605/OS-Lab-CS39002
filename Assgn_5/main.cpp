#include "main.hpp"
#include "guest.hpp"
#include "cleaning.hpp"


#define CLEANING 1
#define CLEANING_DONE 2
#define DIRTY 0

using namespace std;

int X, Y, N;                     // X: # cleaning staff, Y: # Guest, N: # Room

pthread_t *guestThreads;         // guest threads
pthread_t *cleaningStaffThreads; // cleaning staff threads

bool CLEANING_UNDER_PROCESS = false;
int count_of_cleaning = 0; // count of cleaning



int *guestNo;
int *cleaningStaffNo;
Room *rooms;
Guest *guests;
sem_t sem_room_ptr, sem_terminal, sem_cleaning, sem_room_to_clean;
sem_t *sem_guests, *sem_cleaningStaff, *sem_room;
vector<int> rooms_to_clean;
vector<int> available_0, available_1;
set<pair<int, pair<int, int>>> occupied;


int main()
{
    // seed random number generator
    srand(time(NULL));
    // Y > N > X > 1
    cout << "input X (Number of cleaning staff): ";
    cin >> X; // # cleaning staff
    cout << "input Y (Number of guests): ";
    cin >> Y; // # Guest
    cout << "input N (Number of rooms): ";
    cin >> N; // # Room

    // allocate memory
    rooms = (Room *)malloc(N * sizeof(Room));
    guestNo = (int *)malloc(Y * sizeof(int));
    cleaningStaffNo = (int *)malloc(X * sizeof(int));
    guests = (Guest *)malloc(Y * sizeof(Guest));
    sem_guests = (sem_t *)malloc(Y * sizeof(sem_t));
    sem_cleaningStaff = (sem_t *)malloc(X * sizeof(sem_t));
    sem_room = (sem_t *)malloc(N * sizeof(sem_t));

    // initialize semaphore
    sem_init(&sem_room_ptr, 0, 1);
    sem_init(&sem_terminal, 0, 1);
    sem_init(&sem_cleaning, 0, 1);
    sem_init(&sem_room_to_clean, 0, 1);
    for (int i = 0; i < Y; i++)
    {
        sem_init(&sem_guests[i], 0, 0);
    }
    for (int i = 0; i < X; i++)
    {
        sem_init(&sem_cleaningStaff[i], 0, 0);
    }
    for (int i = 0; i < N; i++)
    {
        sem_init(&sem_room[i], 0, 1);
    }

    // initialize rooms
    for (int i = 0; i < N; i++)
    {
        rooms[i] = Room(i);
        available_0.push_back(i);
    }

    // initialize guest and cleaning staff
    for (int i = 0; i < Y; i++)
    {
        guestNo[i] = i;
        guests[i] = Guest(i, rand() % Y);
    }

    for (int i = 0; i < X; i++)
    {
        cleaningStaffNo[i] = i;
    }

    // guest threads
    guestThreads = (pthread_t *)malloc(Y * sizeof(pthread_t));
    for (int i = 0; i < Y; i++)
    {
        pthread_create(&guestThreads[i], NULL, guestThread, &guestNo[i]);
    }

    // cleaning staff threads
    cleaningStaffThreads = (pthread_t *)malloc(X * sizeof(pthread_t));
    for (int i = 0; i < X; i++)
    {
        pthread_create(&cleaningStaffThreads[i], NULL, cleaningStaffThread, &cleaningStaffNo[i]);
    }

    // join guest threads
    for (int i = 0; i < Y; i++)
    {
        pthread_join(guestThreads[i], NULL);
    }

    // join cleaning staff threads
    for (int i = 0; i < X; i++)
    {
        pthread_join(cleaningStaffThreads[i], NULL);
    }

    // destroy semaphore
    sem_destroy(&sem_room_ptr);
    sem_destroy(&sem_terminal);
    sem_destroy(&sem_cleaning);
    sem_destroy(&sem_room_to_clean);
    for (int i = 0; i < Y; i++)
    {
        sem_destroy(&sem_guests[i]);
    }
    for (int i = 0; i < X; i++)
    {
        sem_destroy(&sem_cleaningStaff[i]);
    }
    for (int i = 0; i < N; i++)
    {
        sem_destroy(&sem_room[i]);
    }

    // free memory
    free(rooms);
    free(guestNo);
    free(cleaningStaffNo);
    free(guests);
    free(sem_guests);
    free(sem_cleaningStaff);
    free(sem_room);

    return 0;
}