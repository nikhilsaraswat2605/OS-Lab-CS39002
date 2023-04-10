#include "consumer.hpp"

using namespace std;

void dijsktra(int index, graph *s, int iter, vector<int> &src_nodes, int &op_count, clock_t start, double &total_time_taken)
{

    int node_cnt = s->node_count;
    // run multi-source dijkstra's algorithm
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq; // min heap
    vector<int> dist(node_cnt, 1e9), parent(node_cnt, -1);                              // distance and parent of each node
    vector<bool> visited(node_cnt, false);                                              // visited or not
    for (auto src : src_nodes)
    {
        dist[src] = 0;     // distance of source node is 0
        op_count++;        // increment the operation count
        pq.push({0, src}); // push the source node in the priority queue
    }
    while (!pq.empty())
    {
        int u = pq.top().second; // get the node with minimum distance
        pq.pop();                // pop the node
        if (visited[u])
            continue;
        visited[u] = true; // mark the node as visited
        for (int j = s->nodes[u].head; j != -1; j = s->edges[j].nxt)
        {
            op_count++;
            int v = s->edges[j].to;
            int w = s->edges[j].cost;
            if (dist[v] > dist[u] + w)
            { // if the distance of the node is greater than the distance of the current node + weight of the edge
                dist[v] = dist[u] + w;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
    clock_t end = clock();
    
    // string directory_name = "./output/";
    string filename = "consumer-" + to_string(index + 1) + ".txt";
    // filename = directory_name + filename;
    ofstream outfile;
    outfile.open(filename, ios::app);
    outfile << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
    outfile << "Iteration: " << iter << endl;
    outfile << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
    
    for (int i = 0; i < node_cnt; i++)
    {
        outfile << i << " : Distance - " << dist[i] << " : Path - ";
        int u = i;
        vector<int> path;
        while (u != -1)
        {
            path.push_back(u);
            u = parent[u];
        }
        reverse(path.begin(), path.end());
        for (auto u : path)
        {
            if (u == i)
                outfile << u;
            else
                outfile << u << " -> ";
        }
        outfile << endl;
    }
    outfile << endl;
    outfile << "Average number of operations: " << op_count / iter << endl;
    
    double time_taken = (double(end - start) / double(CLOCKS_PER_SEC)) * 1000;
    outfile << "Time taken for this iteration: " << time_taken << " milliseconds" << endl;
    total_time_taken += time_taken;
    outfile << "Average Time taken per iteration: " << total_time_taken / iter << " milliseconds" << endl;

    outfile << endl;
    outfile.close();
}

void update_non_src_nodes(vector<int> non_src, graph *s, vector<int> &dist, vector<int> &parent, int &op_count)
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> q; // min heap
    for (auto u : non_src)
    {
        op_count++;
        q.push({dist[u], u}); // push the non source node in the priority queue
    }
    while (!q.empty())
    {
        pair<int, int> tmp = q.top(); // get the node with minimum distance
        int u = tmp.second;           // get the node with minimum distance

        q.pop();

        for (int j = s->nodes[u].head; j != -1; j = s->edges[j].nxt)
        {
            op_count++;
            int v = s->edges[j].to;
            int w = s->edges[j].cost;
            if (dist[u] > dist[v] + w)
            { // if the distance of the node is greater than the distance of the current node + weight of the edge
                dist[u] = dist[v] + w;
                parent[u] = v;
            }
        }
        for (int j = s->nodes[u].head; j != -1; j = s->edges[j].nxt)
        { // push the non source node in the priority queue
            op_count++;
            int v = s->edges[j].to;
            int w = s->edges[j].cost;
            if (dist[v] > dist[u] + w)
            { // if the distance of the node is greater than the distance of the current node + weight of the edge
                dist[v] = dist[u] + w;
                parent[v] = u;
                q.push({dist[v], v});
            }
        }
    }
}

void multi_src_dijkstra_optimize(vector<int> &src, vector<int> &dist, graph *s, vector<int> &parent, int &op_count)
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> q;
    for (auto u : src)
    { // push the source node in the priority queue
        q.push({0, u});
        op_count++;
        parent[u] = -1;
        dist[u] = 0;
    }
    vector<bool> visited(s->node_count, false); // visited or not
    while (!q.empty())
    {
        pair<int, int> tmp = q.top(); // get the node with minimum distance
        int u = tmp.second;

        q.pop();

        if (visited[u])
        {
            continue;
        }
        visited[u] = true; // mark the node as visited
        for (int j = s->nodes[u].head; j != -1; j = s->edges[j].nxt)
        { // push the non source node in the priority queue
            op_count++;
            int v = s->edges[j].to;
            int w = s->edges[j].cost;
            if (dist[v] > dist[u] + w)
            { // if the distance of the node is greater than the distance of the current node + weight of the edge
                dist[v] = dist[u] + w;
                parent[v] = u;
                q.push({dist[v], v});
            }
        }
    }
}

void optimized_consumer_process(int id, int shmid)
{
    // start timer

    int op_count = 0;
    // string directory_name = "./output/";
    string filename = "consumer-" + to_string(id + 1) + ".txt";
    // filename = directory_name + filename;
    ofstream outfile;
    outfile.open(filename);
    outfile.close();
    int iter = 1;
    vector<int> src_nodes, non_src_nodes, parent; // source nodes and non source nodes
    vector<int> dist;
    int last_node = 0;
    double total_time_taken = 0;
    while (true)
    {

        int *shm;
        graph *s;
        shm = (int *)shmat(shmid, NULL, 0); // attach the shared memory
        s = (graph *)shm;                   // get the graph
        // dijsktra(id, s, iter, src_nodes);
        int extra_nodes = s->node_count - last_node; // extra nodes to be added
        src_nodes.clear();
        non_src_nodes.clear();
        // add extra nodes to dist vector
        for (int i = 0; i < extra_nodes; i++)
        { // add extra nodes to dist vector
            dist.push_back(1e9);
            parent.push_back(-1);
            op_count++;
        }
        for (int i = (id)*extra_nodes / 10; i < min(((id + 1) * extra_nodes / 10), extra_nodes); i++)
        {                                       // store source nodes
            src_nodes.push_back(i + last_node); // store source nodes
            op_count++;
        }
        // store non source nodes
        for (int i = 0; i < (id)*extra_nodes / 10; i++)
        { // store non source nodes
            non_src_nodes.push_back(i + last_node);
            op_count++;
        }
        for (int i = min(((id + 1) * extra_nodes / 10), extra_nodes); i < extra_nodes; i++)
        { // store non source nodes
            non_src_nodes.push_back(i + last_node);
            op_count++;
        }
        last_node = s->node_count; // update the last node

        clock_t start = clock();

        if (iter == 1)
        {                                                                      // if first iteration
            multi_src_dijkstra_optimize(src_nodes, dist, s, parent, op_count); // update the source nodes
        }
        else if (extra_nodes != 0)
        {                                                                      // if not first iteration
            multi_src_dijkstra_optimize(src_nodes, dist, s, parent, op_count); // update the source nodes
            update_non_src_nodes(non_src_nodes, s, dist, parent, op_count);    // update the non source nodes
        }

        clock_t end = clock();
        double time_taken = (double(end - start) / double(CLOCKS_PER_SEC)) * 1000; // time taken in ms

        outfile.open(filename, ios::app);
        outfile << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
        outfile << "Iteration: " << iter << endl;
        outfile << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
        for (int i = 0; i < s->node_count; i++)
        {
            outfile << i << " : Distance - " << dist[i] << " : Path - ";
            int u = i;
            vector<int> path;
            while (u != -1)
            {
                path.push_back(u);
                u = parent[u];
            }
            reverse(path.begin(), path.end());
            for (auto u : path)
            {
                if (u == i)
                    outfile << u;
                else
                    outfile << u << " -> ";
            }
            outfile << endl;
        }
        outfile << endl;
        outfile << "Average number of operations: " << op_count / iter << endl;
        total_time_taken += time_taken;
        outfile << "Time taken for this iteration: " << time_taken << " milliseconds" << endl;
        outfile << "Average time taken per iteration : " << total_time_taken / iter << " milliseconds" << endl;
        outfile << endl;

        outfile.close();
        // cout << "Consumer " << id + 1 << " number of source nodes: " << src_nodes.size() << endl;
        iter++;
        shmdt(shm);
        sleep(30);
    }
}

void consumer_process(int id, int shmid)
{
    int op_count = 0;
    // string directory_name = "./output/";
    string filename = "consumer-" + to_string(id + 1) + ".txt";
    // filename = directory_name + filename;
    ofstream outfile;
    outfile.open(filename);
    outfile.close();
    int iter = 1;
    vector<int> src_nodes;
    int *shm;
    graph *s;
    shm = (int *)shmat(shmid, NULL, 0);
    s = (graph *)shm;
    int extra_nodes = s->node_count;
    for (int i = (id)*extra_nodes / 10; i < min(((id + 1) * extra_nodes / 10), extra_nodes); i++)
    {                           // store source nodes
        src_nodes.push_back(i); // store source nodes
        op_count++;
    }
    int last_node = s->node_count;
    double total_time = 0;
    while (true)
    {

        int *shm;
        graph *s;
        shm = (int *)shmat(shmid, NULL, 0);
        s = (graph *)shm;

        clock_t start = clock();                                       // start time
        dijsktra(id, s, iter, src_nodes, op_count, start, total_time); // dijkstra

        iter++;
        sleep(30); // sleep for 30 seconds
        int extra_nodes = s->node_count - last_node;
        for (int i = (id)*extra_nodes / 10; i < min(((id + 1) * extra_nodes / 10), extra_nodes); i++)
        {                                       // store source nodes
            src_nodes.push_back(i + last_node); // store source nodes
            op_count++;
        }
        last_node = s->node_count;
        shmdt(shm);
        // cout << "Consumer " << id + 1 << " number of source nodes: " << src_nodes.size() << endl;
    }
}
