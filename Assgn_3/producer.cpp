#include "producer.hpp"

using namespace std;

void producer_process(int shmid)
{
    // producer process
    while (true)
    {
        sleep(50);                          // sleep for 50 seconds
        int *shm;                           // shared memory
        graph *s;                           // graph
        shm = (int *)shmat(shmid, NULL, 0); // attach the shared memory
        s = (graph *)shm;                   // cast the shared memory to a graph
        // get a random number m in the range [10, 30]
        int m = rand() % 21 + 10;                         // m is the number of nodes to be added
        cout << "Nodes added by producer: " << m << endl; // print the number of nodes added
        // add m new nodes to the graph
        for (int i = 0; i < m; i++)
        {
            s->node_count++;
            // select a number k in the range [1, 20] at random
            int k = rand() % 20 + 1;
            // the new node will connect to k existing nodes
            for (int j = 0; j < k; j++)
            {
                // the probability of a new node connecting to an existing node of degree d is proportional to d
                int total_deg_sum = s->edge_count;
                int random = rand() % total_deg_sum;
                int sum = 0;
                int node_index = 0;
                for (int i = 0; i < s->node_count; i++)
                {
                    sum += s->nodes[i].degree;
                    if (sum > random)
                    {
                        node_index = i;
                        break;
                    }
                }
                // add the new edge

                s->addEdge(s->node_count - 1, node_index, 1); // add the edge from the new node to the existing node
                s->addEdge(node_index, s->node_count - 1, 1); // add the edge from the existing node to the new node
            }
        }
        shmdt(shm);
    }
}