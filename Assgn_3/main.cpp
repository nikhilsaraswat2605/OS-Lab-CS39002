#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include "main.hpp"
#include "consumer.hpp"
#include "producer.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc > 2)
    { // check if the number of arguments is correct
        cout << "Usage: ./a.out <flag>" << endl;
        return 0;
    }
    string flag = "-unoptimize";
    if (argc == 2)
    { // check if the flag is correct
        flag = argv[1];
    }
    if (flag != "-unoptimize" && flag != "-optimize")
    { // check if the flag is correct
        cout << "Usage: ./a.out <flag>" << endl;
        return 0;
    }
    else if (flag == "-optimize")
    { // print the version of the program
        cout << "Optimized version" << endl;
    }
    else
    { // print the version of the program
        cout << "Unoptimized version" << endl;
    }

    srand(time(NULL));                    // seed the random number generator
    vector<pair<int, int>> vec;           // vector to store the edges
    ifstream is("facebook_combined.txt"); // input file stream
    int u, v;
    if (!is.is_open())
    {
        cout << "file not open" << endl;
        return 0;
    }
    while (is >> u >> v)
    { // read the edges from the file
        vec.push_back({u, v});
    }
    is.close();

    int n = 0;
    for (auto x : vec)
    { // find the maximum node number
        n = max(n, max(x.first, x.second));
    }
    n++;

    graph g(n); // create a graph with n nodes
    for (auto x : vec)
    { // add the edges to the graph
        g.addEdge(x.first, x.second, 1);
        g.addEdge(x.second, x.first, 1);
    }

    int shmid;
    int key = 1234;                                       // key for the shared memory
    shmid = shmget(key, 2 * sizeof(g), IPC_CREAT | 0666); // create the shared memory
    int *shm;
    shm = (int *)shmat(shmid, NULL, 0); // attach the shared memory
    shmctl(shmid, IPC_RMID, NULL);      // mark the shared memory for deletion
    graph *s;                           // graph
    s = (graph *)shm;                   // cast the shared memory to a graph
    *s = g;
    pid_t pid = fork(); // create a child process
    if (pid == 0)
    {
        // create 10 consumer processes

        for (int i = 0; i < 10; i++)
        {
            pid_t pid = fork(); // create a child process
            if (pid == 0)
            {
                if (flag == "-optimize")
                {                                         // check if the flag is -optimize
                    optimized_consumer_process(i, shmid); // call the optimized consumer process
                }
                else
                {
                    consumer_process(i, shmid); // call the unoptimized consumer process
                }
                exit(0);
            }
        }
        for (int i = 0; i < 10; i++)
        { // wait for all the consumer processes to finish
            wait(NULL);
        }
    }
    else
    {
        pid_t pid = fork(); // create a child process
        if (pid == 0)
        {
            producer_process(shmid); // call the producer process
            exit(0);
        }
        else
        { // wait for the producer process to finish
            waitpid(pid, NULL, 0);
        }
    }
    shmdt(shm); // detach the shared memory
    return 0;
}
